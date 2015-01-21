#ifndef TESTAX_H
#define TESTAX_H

#include <QPushButton>
#include <QSplitter>
#include <QSlider>
#include <QList>
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
	// TODO������5������������Ҫ�Ż�һ��
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
	QPushButton *mpOne, *mpTwo, *mpThree, *mpFour, *mpNine;
	QPushButton *mpPlayPause, *mpStop, *mpForwardBtn, *mpBackwardBtn, *mpFullscreenBtn;

	QSplitter*   m_pSplitter;

	QList<QtAV::VideoRenderer*> mRenderers;
	bool m_isFullscreen;
	int m_currentScreens;
};

#endif // TESTAX_H