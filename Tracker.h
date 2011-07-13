#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include <QQueue>
#include <QGeoPositionInfo>
#include <QNetworkSession>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <qjson/serializer.h>

using namespace QtMobility;

class Tracker : public QObject
{
	Q_OBJECT
public:
	Tracker(const QString &server, const QString &serverPath, const QString &username, const QString &password, const QString &certPath = QString(), unsigned int port = 443, QObject *parent = 0);

private:
	QUrl m_url;
	QList<QSslError> m_expectedSslErrors;
	QQueue<QByteArray> m_positions;
	QJson::Serializer m_json;
	QNetworkSession m_session;
	QNetworkAccessManager m_network;
	bool m_isUploading;

private slots:
	void positionUpdated(const QGeoPositionInfo &update);
	void upload();
	void uploadError(QNetworkReply::NetworkError);
	void nextUpload();

};

#endif // TRACKER_H
