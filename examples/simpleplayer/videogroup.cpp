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

VideoGroup::VideoGroup(QObject *parent) :
QObject(parent)
, view(0)
{
	mpPlayer = new AVPlayer(this);

	mpBar = new QWidget(0, Qt::WindowStaysOnTopHint);
	mpBar->setMaximumSize(400, 60);
	mpBar->show();
	mpBar->setLayout(new QHBoxLayout);
	mpOpen = new QPushButton("Open");
	mpPlay = new QPushButton("Play");
	mpStop = new QPushButton("Stop");
	mpPause = new QPushButton("Pause");
	mpPause->setCheckable(true);
	mpAdd = new QPushButton("+");
	mpRemove = new QPushButton("-");

	connect(mpOpen, SIGNAL(clicked()), SLOT(openLocalFile()));
	connect(mpPlay, SIGNAL(clicked()), mpPlayer, SLOT(play()));
	connect(mpStop, SIGNAL(clicked()), mpPlayer, SLOT(stop()));
	connect(mpPause, SIGNAL(toggled(bool)), mpPlayer, SLOT(pause(bool)));
	connect(mpAdd, SIGNAL(clicked()), SLOT(addRenderer()));
	connect(mpRemove, SIGNAL(clicked()), SLOT(removeRenderer()));

	mpBar->layout()->addWidget(mpOpen);
	mpBar->layout()->addWidget(mpPlay);
	mpBar->layout()->addWidget(mpStop);
	mpBar->layout()->addWidget(mpPause);
	mpBar->layout()->addWidget(mpAdd);
	mpBar->layout()->addWidget(mpRemove);
	
	mpBar->resize(200, 25);

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

	view->setMaximumSize(800, 200);
	view->show();

}

VideoGroup::~VideoGroup()
{
	delete view;
	delete mpBar;
}

void VideoGroup::play(const QString &file)
{
	mpPlayer->play(file);
}

void VideoGroup::openLocalFile()
{
	QString file = QFileDialog::getOpenFileName(0, tr("Open a video"));
	if (file.isEmpty())
		return;
	mpPlayer->stop();
	mpPlayer->play(file);
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
			//VideoRendererId v = VideoRendererId_GLWidget2;// 这个不会删除后显示不了 VideoRendererId_Widget;
			VideoRendererId v = VideoRendererId_Widget;// 这个不会删除后显示不了 VideoRendererId_Widget;

			VideoRenderer* renderer = VideoRendererFactory::create(v);
			mRenderers.append(renderer);
			mpPlayer->addVideoRenderer(renderer);
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
			mpPlayer->removeVideoRenderer(r);
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
		mpPlayer->addVideoRenderer(mRenderers.at(0));

		((QGridLayout*)view->layout())->addWidget(mRenderers.at(1)->widget(), 0, 1);
		mpPlayer->addVideoRenderer(mRenderers.at(1));
	}
	else if (mRenderers.size() == 3)
	{
		((QGridLayout*)view->layout())->addWidget(mRenderers.at(0)->widget(),0 , 0, 1, 2);
		mpPlayer->addVideoRenderer(mRenderers.at(0));

		((QGridLayout*)view->layout())->addWidget(mRenderers.at(1)->widget(), 1, 0);
		mpPlayer->addVideoRenderer(mRenderers.at(1));

		((QGridLayout*)view->layout())->addWidget(mRenderers.at(2)->widget(), 1, 1);
		mpPlayer->addVideoRenderer(mRenderers.at(2));
	}
	else
	{
		for (int i = 0; i < mRenderers.size(); ++i)
		{
			((QGridLayout*)view->layout())->addWidget(mRenderers.at(i)->widget(),
				i / (int)sqrt((double)mRenderers.size()), i % (int)(sqrt((double)mRenderers.size())));
			mpPlayer->addVideoRenderer(mRenderers.at(i));
		}
	}
}
