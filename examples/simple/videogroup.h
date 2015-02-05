#ifndef TESTAX_H
#define TESTAX_H

#include <QPushButton>
#include <QSplitter>
#include <QSlider>
#include <QList>
#include <QTimer>
#include <QtWidgets/QWidget>
#include <ActiveQt/QAxBindable>
#include <QtAV/AVPlayer.h>
#include <QtAV/WidgetRenderer.h>
#include "Slider.h"
#include "PlaylistTreeView.h"


class VideoGroup : public QWidget, public QAxBindable
{
	Q_OBJECT
public:
	explicit VideoGroup(QWidget *parent = 0);
	~VideoGroup();

	void play(const QString& file);

	public slots:
	// TODO下面这5个函数很龊，要优化一下
	void set1Renderer();
	void set2Renderer();
	void set3Renderer();
	void set4Renderer();
	void set9Renderer();

	void openUrl();
	void openLocalFile();
	void preloadSuccess();
	void onStartPlay();
	void onStopPlay();
	void onPositionChange(qint64 pos);
	void seek();
	void setRenderByDrag(QtAV::VideoRenderer* render);
	public slots:
	void preload(const QString& file);
	void onPauseResumeClick();
	void setFullscreen();
	QString about(void);

private slots:
	void setVolume();
	void setMute();
	void intervalTimerExpired();
private:
	void resetPlayer(void);
	virtual void resizeEvent(QResizeEvent *event);
	void updateScreen(int num);
	int timer_id;
	QtAV::AVPlayer *mpPlayer;
	QWidget *view;
	QWidget *mpBar;
	QWidget *mainWidget;
	PlaylistTreeView    *m_playList;
	Slider *mpTimeSlider, *mpVolumeSlider;
	QPushButton *mpOne, *mpTwo, *mpThree, *mpFour, *mpNine;
	QPushButton *mpPlayPause, *mpForwardBtn, *mpBackwardBtn, *mpFullscreenBtn, *mpMute;

	QSplitter*   m_pSplitter;

	QList<QtAV::VideoRenderer*> mRenderers;
	bool m_isFullscreen;
	int m_currentScreens;
	bool m_isMute;
	QTimer*      m_intervalTimer;
};

#endif // TESTAX_H
