#include "videogroup.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QtCore/QUrl>
#include <QtAV/AudioOutput.h>
#include <QtAV/VideoRendererTypes.h>

using namespace QtAV;

VideoGroup::VideoGroup(QWidget *parent) :
QWidget(parent)
, view(0)
{
	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	//setLayout(mainLayout);
	mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);

	mpPlayer = new AVPlayer(this);

	mpBar = new QWidget(0, Qt::WindowStaysOnTopHint);
	mpBar->setMaximumSize(400, 40);
	mpBar->setLayout(new QHBoxLayout);

	mpTimeSlider = new Slider();
	mpTimeSlider->setDisabled(true);
	//mpTimeSlider->setFixedHeight(8);
	mpTimeSlider->setMaximumHeight(8);
	mpTimeSlider->setTracking(true);
	mpTimeSlider->setOrientation(Qt::Horizontal);
	mpTimeSlider->setMinimum(0);

	mpOpen = new QPushButton("Open");
	mpPlay = new QPushButton("Play");
	mpStop = new QPushButton("Stop");
	mpPause = new QPushButton("Pause");
	mpPause->setCheckable(true);
	mpAdd = new QPushButton("+");
	mpRemove = new QPushButton("-");
	mpForwardBtn = new QPushButton("FW");
	mpBackwardBtn = new QPushButton("BW");

	connect(mpOpen, SIGNAL(clicked()), SLOT(openLocalFile()));
	connect(mpPlay, SIGNAL(clicked()), mpPlayer, SLOT(play()));
	connect(mpStop, SIGNAL(clicked()), mpPlayer, SLOT(stop()));
	connect(mpPause, SIGNAL(toggled(bool)), mpPlayer, SLOT(pause(bool)));
	connect(mpAdd, SIGNAL(clicked()), SLOT(addRenderer()));
	connect(mpRemove, SIGNAL(clicked()), SLOT(removeRenderer()));
	connect(mpPlayer, SIGNAL(preloadSuccess()), this, SLOT(preloadSuccess()));
	connect(mpPlayer, SIGNAL(started()), this, SLOT(onStartPlay()));
	connect(mpPlayer, SIGNAL(stopped()), this, SLOT(onStopPlay()));
	connect(mpPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChange(qint64)));
	connect(mpTimeSlider, SIGNAL(sliderPressed()), SLOT(seek()));
	connect(mpTimeSlider, SIGNAL(sliderReleased()), SLOT(seek()));
	connect(mpForwardBtn, SIGNAL(clicked()), mpPlayer, SLOT(seekForward()));
	connect(mpBackwardBtn, SIGNAL(clicked()), mpPlayer, SLOT(seekBackward()));

	mpBar->layout()->addWidget(mpOpen);
	mpBar->layout()->addWidget(mpPlay);
	mpBar->layout()->addWidget(mpStop);
	mpBar->layout()->addWidget(mpPause);
	mpBar->layout()->addWidget(mpBackwardBtn);
	mpBar->layout()->addWidget(mpForwardBtn);
	mpBar->layout()->addWidget(mpAdd);
	mpBar->layout()->addWidget(mpRemove);
	
	//mpBar->setMaximumHeight(25);

	view = new QWidget;
	view->resize(qApp->desktop()->size());
	QGridLayout *layout = new QGridLayout;
	layout->setSizeConstraint(QLayout::SetMaximumSize);
	layout->setSpacing(1);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);
	view->setLayout(layout);

	m_currentScreenIndex = 4;
	int tempSupportScreen[6] = { 1, 2, 3, 4, 9, 16 };
	memcpy(m_supportScreen, tempSupportScreen, sizeof(tempSupportScreen));
	updateScreen(m_supportScreen[m_currentScreenIndex]);

	//mainLayout->addLayout(layout);
	mainLayout->addWidget(view);
	mainLayout->addWidget(mpTimeSlider);
	mainLayout->addWidget(mpBar);

	m_pSplitter = new QSplitter(Qt::Horizontal, this);
	m_pSplitter->resize(this->size());

	m_playList = new PlaylistTreeView();

	m_pSplitter->addWidget(m_playList);
	m_pSplitter->addWidget(mainWidget);
	m_pSplitter->handle(0)->installEventFilter(this);
	m_pSplitter->setHandleWidth(1);

	// 设置列表和主窗口左右比例
	QList<int> size;
	size.append(this->width() * 0.2);
	size.append(this->width() * 0.8);
	m_pSplitter->setSizes(size);
}

VideoGroup::~VideoGroup()
{
	delete view;
	delete mpBar;
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
	if (! mpPlayer->isLoaded())
		return;

	m_playList->addItem("aa", "11");
	m_playList->addItem("bb", "22");

	mpPlayer->disableAllProgram();

	for (int i = 0; i < m_supportScreen[m_currentScreenIndex] && i < mpPlayer->videoStreamCount(); i++)
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

void VideoGroup::openLocalFile()
{
}

void VideoGroup::addRenderer()
{
	m_currentScreenIndex = m_currentScreenIndex < 4 ? m_currentScreenIndex + 1 : m_currentScreenIndex;
	int currentScreen = m_supportScreen[m_currentScreenIndex];
	updateScreen(currentScreen);
}

void VideoGroup::removeRenderer()
{
	m_currentScreenIndex = m_currentScreenIndex > 0 ? m_currentScreenIndex - 1 : m_currentScreenIndex;
	int currentScreen = m_supportScreen[m_currentScreenIndex];
	updateScreen(currentScreen);
}

void VideoGroup::updateScreen(int num)
{
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
		((QGridLayout*)view->layout())->addWidget(mRenderers.at(0)->widget(),0 , 0, 1, 2);

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
