#include <QApplication>
#include <QMessageBox>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QTextCodec>

#include <QtAV/AVPlayer.h>
#include <QtAV/GLWidgetRenderer2.h>
#include "videogroup.h"

static FILE *sLogfile = 0; //'log' is a function in msvc math.h

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define qInstallMessageHandler qInstallMsgHandler
void Logger(QtMsgType type, const char *msg)
{
#else
void Logger(QtMsgType type, const QMessageLogContext &, const QString& qmsg)
{
	const char* msg = qPrintable(qmsg);
#endif
	fprintf(stdout, "%s", QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]  ").toStdString().c_str());
	if (sLogfile)
		fprintf(sLogfile, "%s", QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]  ").toStdString().c_str());

	switch (type) {
	case QtDebugMsg:
		fprintf(stdout, "Debug: %s\n", msg);
		if (sLogfile)
			fprintf(sLogfile, "Debug: %s\n", msg);
		break;
	case QtWarningMsg:
		fprintf(stdout, "Warning: %s\n", msg);
		if (sLogfile)
			fprintf(sLogfile, "Warning: %s\n", msg);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s\n", msg);
		if (sLogfile)
			fprintf(sLogfile, "Critical: %s\n", msg);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", msg);
		if (sLogfile)
			fprintf(sLogfile, "Fatal: %s\n", msg);
		abort();
	}
	fflush(0);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// 设置编码
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

	// 设置log日志
	sLogfile = fopen(QString("log.txt").toUtf8().constData(), "w+");
	if (!sLogfile) 
	{
		qWarning("Failed to open log file");
		sLogfile = stdout;
	}
	qInstallMessageHandler(Logger);

	// 设置风格
	qDebug("set nomal style.");
	QFile styleSheet(":/APlayer/Resources/qss/style.qss");
	if (!styleSheet.open(QIODevice::ReadOnly))
	{
		qWarning("Can't open the style sheet file.");
		return -1;
	}
	qApp->setStyleSheet(styleSheet.readAll());

	VideoGroup wall;
	wall.resize(1000, 600);
	wall.show();

	//wall.play("F:\\movie\\ALAN_HACKEN.mp4");
	//wall.play("C:\\workspace\\3rd-party\\movie\\mul.ts");
	//wall.play("C:\\workspace\\3rd-party\\movie\\trailer-mpeg2.mpg");
	//wall.preload("C:\\workspace\\3rd-party\\movie\\trailer-mpeg2.mpg");
	wall.preload("C:\\workspace\\3rd-party\\movie\\mul.ts");

    return a.exec();
}
