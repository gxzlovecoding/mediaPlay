#ifndef QTAV_VIDEOWALL_H
#define QTAV_VIDEOWALL_H

#include <QtCore/QList>
#include <QtAV/AVPlayer.h>
#include <QtAV/WidgetRenderer.h>
#include <QtWidgets/QPushButton>
#include "Slider.h"

#define MAX_SCREEN 16

class VideoGroup : public QWidget
{
	Q_OBJECT
public:
	explicit VideoGroup(QWidget *parent = 0);
	~VideoGroup();

	void play(const QString& file);
	void preload(const QString& file);

public slots:
	void openLocalFile();
	void addRenderer();
	void removeRenderer();
	void preloadSuccess();
	void onStartPlay();
	void onStopPlay();
	void onPositionChange(qint64 pos);
	void seek();

private:
	void updateScreen(int num);
	int timer_id;
	QtAV::AVPlayer *mpPlayer;
	QWidget *view;
	QWidget *mpBar;
	Slider *mpTimeSlider;
	QPushButton *mpAdd, *mpRemove, *mpOpen, *mpPlay, *mpStop, *mpPause, *mpForwardBtn, *mpBackwardBtn;

	QList<QtAV::VideoRenderer*> mRenderers;
	int		m_supportScreen[6];
	int		m_currentScreenIndex;
};

#endif // QTAV_VIDEOWALL_H
