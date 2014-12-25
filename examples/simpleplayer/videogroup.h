#ifndef QTAV_VIDEOWALL_H
#define QTAV_VIDEOWALL_H

#include <QtCore/QList>
#include <QtAV/AVPlayer.h>
#include <QtAV/WidgetRenderer.h>
#include <QtWidgets/QPushButton>

#define MAX_SCREEN 16

class VideoGroup : public QObject
{
	Q_OBJECT
public:
	explicit VideoGroup(QObject *parent = 0);
	~VideoGroup();

	void play(const QString& file);
	void preload(const QString& file);

public slots:
	void openLocalFile();
	void addRenderer();
	void removeRenderer();

private:
	void updateScreen(int num);
	int timer_id;
	QtAV::AVPlayer *mpPlayer;
	QWidget *view;
	QWidget *mpBar;
	QPushButton *mpAdd, *mpRemove, *mpOpen, *mpPlay, *mpStop, *mpPause;

	QList<QtAV::VideoRenderer*> mRenderers;
	int		m_supportScreen[6];
	int		m_currentScreenIndex;
};

#endif // QTAV_VIDEOWALL_H
