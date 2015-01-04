#ifndef QTAV_VIDEOWALL_H
#define QTAV_VIDEOWALL_H

#include <QtCore/QList>
#include <QSplitter>
#include <QtAV/AVPlayer.h>
#include <QtAV/WidgetRenderer.h>
#include <QtWidgets/QPushButton>
#include "Slider.h"
#include "PlaylistTreeView.h"

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
    void openUrl();
	void openLocalFile();
	void addRenderer();
	void removeRenderer();
	void preloadSuccess();
	void onStartPlay();
	void onStopPlay();
	void onPositionChange(qint64 pos);
	void seek();

private:
	virtual void resizeEvent(QResizeEvent *event);
	void updateScreen(int num);
	int timer_id;
	QtAV::AVPlayer *mpPlayer;
	QWidget *view;
	QWidget *mpBar;
	QWidget *mainWidget;
	PlaylistTreeView    *m_playList;
	Slider *mpTimeSlider;
	QPushButton *mpAdd, *mpRemove, *mpOpenUrl, *mpOpen, *mpPlay, *mpStop, *mpPause, *mpForwardBtn, *mpBackwardBtn;

	QSplitter*   m_pSplitter;

	QList<QtAV::VideoRenderer*> mRenderers;
	int		m_supportScreen[6];
	int		m_currentScreenIndex;
};

#endif // QTAV_VIDEOWALL_H
