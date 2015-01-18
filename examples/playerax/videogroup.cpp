#include "videogroup.h"

#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <ActiveQt/QAxFactory>
#include <QDesktopWidget>
#include <QApplication>

#include <QtAV/VideoRendererTypes.h>

using namespace QtAV;

VideoGroup::VideoGroup(QWidget *parent)
	: QWidget(parent)
	, m_isFullscreen(false)
	, m_currentScreens(1)
{
	this->resize(1000, 600);
	//this->setLayout(new QVBoxLayout);

	// 主视频窗口初始化 --------->
	view = new QWidget;
	view->setLayout(new QGridLayout);
	view->layout()->setSizeConstraint(QLayout::SetMaximumSize);
	view->layout()->setSpacing(1);
	view->layout()->setMargin(0); 
	view->layout()->setContentsMargins(0, 0, 0, 0);
	mpPlayer = new AVPlayer();
	updateScreen(m_currentScreens);
	// <-----------

	// 进度条 --------->
	mpTimeSlider = new Slider();
	mpTimeSlider->setDisabled(true);
	mpTimeSlider->setMaximumHeight(8);
	mpTimeSlider->setTracking(true);
	mpTimeSlider->setOrientation(Qt::Horizontal);
	mpTimeSlider->setMinimum(0);
	mpTimeSlider->setMaximumHeight(10);
	// <--------

	// 控制面板初始化 ---------->
	mpBar = new QWidget();
	mpBar->setMaximumHeight(50);
	mpBar->setLayout(new QHBoxLayout);

	mpOpenUrl = new QPushButton("O_URL");
	mpOpen = new QPushButton("O_File");
	mpPlay = new QPushButton("Play");
	mpStop = new QPushButton("Stop");
	mpPause = new QPushButton("Pause");
	mpPause->setCheckable(true);
	mpOne = new QPushButton("1");
	mpTwo = new QPushButton("2");
	mpThree = new QPushButton("3");
	mpFour = new QPushButton("4");
	mpNine = new QPushButton("9");
	mpFullscreenBtn = new QPushButton("Full");
	mpForwardBtn = new QPushButton("FW");
	mpBackwardBtn = new QPushButton("BW");

	mpBar->layout()->addWidget(mpOpenUrl);
	mpBar->layout()->addWidget(mpOpen);
	mpBar->layout()->addWidget(mpPlay);
	mpBar->layout()->addWidget(mpStop);
	mpBar->layout()->addWidget(mpPause);
	mpBar->layout()->addWidget(mpBackwardBtn);
	mpBar->layout()->addWidget(mpForwardBtn);
	mpBar->layout()->addWidget(mpOne);
	mpBar->layout()->addWidget(mpTwo);
	mpBar->layout()->addWidget(mpThree);
	mpBar->layout()->addWidget(mpFour);
	mpBar->layout()->addWidget(mpNine);
	mpBar->layout()->addWidget(mpFullscreenBtn);

	connect(mpOpenUrl, SIGNAL(clicked()), SLOT(openUrl()));
	connect(mpOpen, SIGNAL(clicked()), SLOT(openLocalFile()));
	connect(mpPlay, SIGNAL(clicked()), mpPlayer, SLOT(play()));
	connect(mpStop, SIGNAL(clicked()), mpPlayer, SLOT(stop()));
	connect(mpPause, SIGNAL(toggled(bool)), mpPlayer, SLOT(pause(bool)));
	connect(mpOne, SIGNAL(clicked()), SLOT(set1Renderer()));
	connect(mpTwo, SIGNAL(clicked()), SLOT(set2Renderer()));
	connect(mpThree, SIGNAL(clicked()), SLOT(set3Renderer()));
	connect(mpFour, SIGNAL(clicked()), SLOT(set4Renderer()));
	connect(mpNine, SIGNAL(clicked()), SLOT(set9Renderer()));
	connect(mpFullscreenBtn, SIGNAL(clicked()), SLOT(setFullscreen()));
	connect(mpPlayer, SIGNAL(preloadSuccess()), this, SLOT(preloadSuccess()));
	connect(mpPlayer, SIGNAL(started()), this, SLOT(onStartPlay()));
	connect(mpPlayer, SIGNAL(stopped()), this, SLOT(onStopPlay()));
	connect(mpPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChange(qint64)));
	connect(mpTimeSlider, SIGNAL(sliderPressed()), SLOT(seek()));
	connect(mpTimeSlider, SIGNAL(sliderReleased()), SLOT(seek()));
	connect(mpForwardBtn, SIGNAL(clicked()), mpPlayer, SLOT(seekForward()));
	connect(mpBackwardBtn, SIGNAL(clicked()), mpPlayer, SLOT(seekBackward()));	
	// <---------

	// 左则视频列表 --------->
	m_playList = new PlaylistTreeView();
	// <---------

	// 右则主窗口（包括视频窗口、进度条、控制面板）------->
	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);

	mainLayout->addWidget(view);
	mainLayout->addWidget(mpTimeSlider);
	mainLayout->addWidget(mpBar);
	// <---------

	// 整个窗口左右分部设置--------->
	m_pSplitter = new QSplitter(Qt::Horizontal, this);
	m_pSplitter->resize(this->size());
	m_pSplitter->addWidget(m_playList);
	m_pSplitter->addWidget(mainWidget);
	
	m_pSplitter->handle(0)->installEventFilter(this);
	m_pSplitter->setHandleWidth(1);

	// 设置列表和主窗口左右比例
	QList<int> size;
	size.append(this->width() * 0.2);
	size.append(this->width() * 0.8);
	m_pSplitter->setSizes(size);
	// <------------	
}


VideoGroup::~VideoGroup()
{
}

void VideoGroup::resizeEvent(QResizeEvent *event)
{
	m_pSplitter->setFixedSize(this->size());
}

void VideoGroup::play(const QString &file)
{
	qint64 pos = 0;
	mpPlayer->seek(pos);
	//mpPlayer->play(file);
	//TODO 第一个是接口是预显示第一帧。

	//TODO 真正设置好每个画面对应某个节目后才开始播放。
}

void VideoGroup::preload(const QString& file)
{
	mpPlayer->preLoad(file);
}

void VideoGroup::preloadSuccess()
{
	if (!mpPlayer->isLoaded())
		return;

	mpPlayer->disableAllProgram();

	// 取出视频自带的私有流名字数据
	m_playList->clear();
	for (int i = 0; i < mpPlayer->videoStreamCount(); i++)
	{
		std::string streamName = mpPlayer->getVideoStreamName(i);
		m_playList->addItem(QString::fromLocal8Bit(streamName.c_str()), "11");
	}

	for (int i = 0; i < m_currentScreens && i < mpPlayer->videoStreamCount(); i++)
	{
		mRenderers[i]->receive(mpPlayer->getFirstFrame(i));
		mpPlayer->setRenderer(mRenderers[i], i);
		mpPlayer->enableProgram(i);
	}
}

void VideoGroup::onStartPlay()
{
	mpTimeSlider->setMinimum(mpPlayer->mediaStartPosition());
	mpTimeSlider->setMaximum(mpPlayer->mediaStopPosition());
	mpTimeSlider->setValue(0);
	mpTimeSlider->setEnabled(true);
}

void VideoGroup::onStopPlay()
{
	mpTimeSlider->setValue(0);
	qDebug(">>>>>>>>>>>>>>disable slider");
	mpTimeSlider->setDisabled(true);
}

void VideoGroup::onPositionChange(qint64 pos)
{
	mpTimeSlider->setValue(pos);
}

void VideoGroup::seek()
{
	mpPlayer->seek((qint64)mpTimeSlider->value());
}

void VideoGroup::openUrl()
{
	QString url = QInputDialog::getText(0, tr("Open an url"), tr("Url"));
	if (url.isEmpty())
		return;
	preload(url);
}
void VideoGroup::openLocalFile()
{
	QString file = QFileDialog::getOpenFileName(0, tr("Open a media file"));
	if (file.isEmpty())
		return;
	preload(file);
}

void VideoGroup::set1Renderer()
{
	updateScreen(1);
}
void VideoGroup::set2Renderer()
{
	updateScreen(2);
}
void VideoGroup::set3Renderer()
{
	updateScreen(3);
}
void VideoGroup::set4Renderer()
{
	updateScreen(4);
}
void VideoGroup::set9Renderer()
{
	updateScreen(9);
}

void VideoGroup::updateScreen(int num)
{
	m_currentScreens = num;

	if (mRenderers.size() == num) 
	{
		// 窗口不变
		return;
	}
	else if (mRenderers.size() < num)
	{
		// 窗口增加
		while (mRenderers.size() < num)
		{
			VideoRendererId v = VideoRendererId_GLWidget2;// 这个不会删除后显示不了 VideoRendererId_Widget;
			//VideoRendererId v = VideoRendererId_Widget;// 这个不会删除后显示不了 VideoRendererId_Widget;

			VideoRenderer* renderer = VideoRendererFactory::create(v);
			mRenderers.append(renderer);
			renderer->widget()->setAttribute(Qt::WA_DeleteOnClose);
			renderer->widget()->setWindowFlags(renderer->widget()->windowFlags() | Qt::FramelessWindowHint);
		}
	}
	else
	{
		// 窗口减少
		while (mRenderers.size() > num)
		{
			VideoRenderer *r = mRenderers.takeLast();
			// TODO 删除这个窗口前，把这个窗口从播放线程中释放。
			//mpPlayer->removeVideoRenderer(r);
			if (view)
			{
				view->layout()->removeWidget(r->widget());
			}
			delete r;
		}
	}

	if (mRenderers.size() == 2)
	{
		((QGridLayout*)view->layout())->addWidget(mRenderers.at(0)->widget(), 0, 0);

		((QGridLayout*)view->layout())->addWidget(mRenderers.at(1)->widget(), 0, 1);
	}
	else if (mRenderers.size() == 3)
	{
		((QGridLayout*)view->layout())->addWidget(mRenderers.at(0)->widget(), 0, 0, 1, 2);

		((QGridLayout*)view->layout())->addWidget(mRenderers.at(1)->widget(), 1, 0);

		((QGridLayout*)view->layout())->addWidget(mRenderers.at(2)->widget(), 1, 1);
	}
	else
	{
		for (int i = 0; i < mRenderers.size(); ++i)
		{
			((QGridLayout*)view->layout())->addWidget(mRenderers.at(i)->widget(),
				i / (int)sqrt((double)mRenderers.size()), i % (int)(sqrt((double)mRenderers.size())));
		}
	}

	preloadSuccess();
}

void VideoGroup::setFullscreen()
{
	static QWidget* temp = new QWidget();
	if (!m_isFullscreen)
	{
		// TODO 把全屏放到一个widket做全屏，因为单独全屏有问题。
		temp->setWindowFlags(Qt::Window);
		temp->setLayout(new QVBoxLayout());

		temp->layout()->addWidget(mainWidget);
		temp->showFullScreen();
		mainWidget->move(0, 0);
		mainWidget->resize(temp->size());
	}
	else
	{
		temp->layout()->removeWidget(mainWidget);
		temp->setWindowFlags(Qt::Widget);
		temp->hide();

		// 把窗口重新放回splitter
		m_pSplitter->addWidget(mainWidget);
	}
	m_isFullscreen = !m_isFullscreen;
}

QAXFACTORY_DEFAULT(VideoGroup,
	   "{42E32F5E-1B92-4368-8D96-022647B1BCFB}",
	   "{C5982A06-A7F7-4783-B535-8348FA949E74}",
	   "{C791939A-8F7B-47AE-9B03-A1DEF0A70002}",
	   "{C570ED43-966B-4BED-BA80-04DB85E399BF}",
	   "{EA7EA284-C139-4E97-8C23-460877AB770B}")
