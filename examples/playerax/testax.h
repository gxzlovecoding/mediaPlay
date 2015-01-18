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

#include "ui_testax.h"

class testax : public QWidget, public QAxBindable
{
	Q_OBJECT

public:
	testax(QWidget *parent = 0);
	~testax();
	void play(const QString& file);
	void preload(const QString& file);

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
	void setFullscreen();
private:
	virtual void resizeEvent(QResizeEvent *event);
	void updateScreen(int num);

	QtAV::AVPlayer *mpPlayer;
	Slider *mpTimeSlider;
	QSplitter*   m_pSplitter;
	PlaylistTreeView    *m_playList;
	QWidget *view, *mpBar, *mainWidget;
	QPushButton *mpOne, *mpTwo, *mpThree, *mpFour, *mpNine;
	QPushButton	*mpOpenUrl, *mpOpen, *mpPlay, *mpStop, *mpPause, *mpForwardBtn, *mpBackwardBtn, *mpFullscreenBtn;
	QList<QtAV::VideoRenderer*> mRenderers;
	bool m_isFullscreen;
	int m_currentScreens;
};

#endif // TESTAX_H
