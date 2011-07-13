#include "Tracker.h"
#include <QGeoPositionInfoSource>
#include <QVariantMap>
#include <QNetworkRequest>
#include <QNetworkConfigurationManager>
#include <QTimer>
#include <QSslError>
#include <QSslCertificate>
#include <QList>
#include <QDebug>

using namespace QtMobility;

Tracker::Tracker(const QString &server, const QString &serverPath, const QString &username, const QString &password, const QString &certPath, unsigned int serverPort, QObject *parent) :
	QObject(parent),
	m_session(QNetworkConfigurationManager().defaultConfiguration()),
	m_isUploading(false)
{
	m_url.setPath(QString("%1/update.py").arg(serverPath));
	m_url.setScheme(QLatin1String("https"));
	m_url.setHost(server);
	m_url.setPort(serverPort);
	m_url.setUserName(username);
	m_url.setPassword(password);

	if (!certPath.isEmpty()) {
		foreach (const QSslCertificate &cert, QSslCertificate::fromPath(certPath, QSsl::Der)) {
			qDebug() << cert << cert.issuerInfo(QSslCertificate::CommonName);
			m_expectedSslErrors.append(QSslError(QSslError::SelfSignedCertificate, cert));
		}
	}

	connect(&m_session, SIGNAL(opened()), this, SLOT(nextUpload()));

	QGeoPositionInfoSource *gps = QGeoPositionInfoSource::createDefaultSource(this);
	if (!gps)
		return;
	connect(gps, SIGNAL(positionUpdated(QGeoPositionInfo)), this, SLOT(positionUpdated(QGeoPositionInfo)));
	gps->setUpdateInterval(1000 * 60 * 15);
	gps->startUpdates();
}
void Tracker::positionUpdated(const QGeoPositionInfo &update)
{
	if (!update.isValid() || !update.coordinate().isValid())
		return;
	QVariantMap map;
	map.insert(QLatin1String("timestamp"), update.timestamp().toMSecsSinceEpoch());
	if (update.coordinate().type() == QGeoCoordinate::Coordinate3D)
		map.insert(QLatin1String("altitude"), update.coordinate().altitude());
	map.insert(QLatin1String("latitude"), update.coordinate().latitude());
	map.insert(QLatin1String("longitude"), update.coordinate().longitude());
	if (update.hasAttribute(QGeoPositionInfo::Direction))
		map.insert(QLatin1String("direction"), update.attribute(QGeoPositionInfo::Direction));
	if (update.hasAttribute(QGeoPositionInfo::GroundSpeed))
		map.insert(QLatin1String("groundSpeed"), update.attribute(QGeoPositionInfo::GroundSpeed));
	if (update.hasAttribute(QGeoPositionInfo::VerticalSpeed))
		map.insert(QLatin1String("verticalSpeed"), update.attribute(QGeoPositionInfo::VerticalSpeed));
	if (update.hasAttribute(QGeoPositionInfo::MagneticVariation))
		map.insert(QLatin1String("magneticVariation"), update.attribute(QGeoPositionInfo::MagneticVariation));
	if (update.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
		map.insert(QLatin1String("horizontalAccuracy"), update.attribute(QGeoPositionInfo::HorizontalAccuracy));
	if (update.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
		map.insert(QLatin1String("verticalAccuracy"), update.attribute(QGeoPositionInfo::VerticalAccuracy));

	m_positions.enqueue(m_json.serialize(map));
	upload();
}
void Tracker::upload()
{
	if (m_isUploading)
		return;
	m_isUploading = true;
	m_session.open();
}
void Tracker::nextUpload()
{
	if (m_positions.empty()) {
		m_session.close();
		m_isUploading = false;
		return;
	}
	if (qobject_cast<QNetworkReply*>(sender())) {
		m_positions.dequeue();
		sender()->deleteLater();
	}

	QNetworkRequest request(m_url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
	QNetworkReply *reply = m_network.post(request, m_positions.head());
	reply->ignoreSslErrors(m_expectedSslErrors);
	connect(reply, SIGNAL(finished()), this, SLOT(nextUpload()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
}
void Tracker::uploadError(QNetworkReply::NetworkError)
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply)
		return;
	qDebug() << reply->errorString();
	reply->deleteLater();
	QTimer::singleShot(1000 * 60 * 5, this, SLOT(nextUpload()));
}
