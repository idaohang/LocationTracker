#include <QCoreApplication>
#include "Tracker.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationName("Location Tracker");
	a.setOrganizationName("ZX2C4 Software");
	a.setOrganizationDomain("zx2c4.com");
	a.setApplicationVersion("0.1");
	Tracker tracker("zx2c4.com", "/projects/locationtracker", "n950", "tester1234", "/home/user/MyDocs/zx2c4.der");
	return a.exec();
}
