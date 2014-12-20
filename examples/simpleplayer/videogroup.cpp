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
, r(3), c(3), view(0)
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

	m_currentScreenIndex = 4;
	int tempSupportScreen[6] = { 1, 2, 3, 4, 9, 16 };
	memcpy(m_supportScreen, tempSupportScreen, sizeof(tempSupportScreen));

	for (int i = 0; i < r; ++i) {
		for (int j = 0; j < c; ++j) {
			//VideoRendererId v = VideoRendererId_GLWidget2;// 这个不会删除后显示不了 VideoRendererId_Widget;
			VideoRendererId v = VideoRendererId_Widget;// 这个不会删除后显示不了 VideoRendererId_Widget;

			VideoRenderer* renderer = VideoRendererFactory::create(v);
			mRenderers.append(renderer);
			renderer->widget()->setAttribute(Qt::WA_DeleteOnClose);
			renderer->widget()->setWindowFlags(renderer->widget()->windowFlags() | Qt::FramelessWindowHint);
		}
	}

	view = new QWidget;
	view->resize(qApp->desktop()->size());
	QGridLayout *layout = new QGridLayout;
	layout->setSizeConstraint(QLayout::SetMaximumSize);
	layout->setSpacing(1);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);
	view->setLayout(layout);

	for (int i = 0; i < mRenderers.size(); ++i)
	{
		int x = i / cols();
		int y = i%cols();
		((QGridLayout*)view->layout())->addWidget(mRenderers.at(i)->widget(), x, y);
		mpPlayer->addVideoRenderer(mRenderers.at(i));
	}
	view->setMaximumSize(800, 200);
	view->show();

}

VideoGroup::~VideoGroup()
{
	delete view;
	delete mpBar;
}

void VideoGroup::setRows(int n)
{
	r = n;
}

void VideoGroup::setCols(int n)
{
	c = n;
}

int VideoGroup::rows() const
{
	return r;
}

int VideoGroup::cols() const
{
	return c;
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
	/*
	r = 2;
	c = 2;
	for (int i = 0; i < r; ++i) {
	for (int j = 0; j < c; ++j) {
	//VideoRendererId v = VideoRendererId_GLWidget2;// 这个不会删除后显示不了 VideoRendererId_Widget;
	VideoRendererId v = VideoRendererId_Widget;// 这个不会删除后显示不了 VideoRendererId_Widget;

	VideoRenderer* renderer = VideoRendererFactory::create(v);
	mRenderers.append(renderer);
	renderer->widget()->setAttribute(Qt::WA_DeleteOnClose);
	renderer->widget()->setWindowFlags(renderer->widget()->windowFlags() | Qt::FramelessWindowHint);
	}
	}


	for (int i = 0; i < mRenderers.size(); ++i)
	{
	int x = i / cols();
	int y = i%cols();
	((QGridLayout*)view->layout())->addWidget(mRenderers.at(i)->widget(), x, y);
	mpPlayer->addVideoRenderer(mRenderers.at(i));
	}
	*/

	//VideoRendererId v = VideoRendererId_GLWidget2;// 这个不会删除后显示不了 VideoRendererId_Widget;
	VideoRendererId v = VideoRendererId_Widget;// 这个不会删除后显示不了 VideoRendererId_Widget;

	VideoRenderer* renderer = VideoRendererFactory::create(v);
	mRenderers = mpPlayer->videoOutputs();
	mRenderers.append(renderer);
	renderer->widget()->setAttribute(Qt::WA_DeleteOnClose);
	Qt::WindowFlags wf = renderer->widget()->windowFlags();
	wf |= Qt::FramelessWindowHint;

	renderer->widget()->setWindowFlags(wf);
	int w = view ? view->frameGeometry().width() / c : qApp->desktop()->width() / c;
	int h = view ? view->frameGeometry().height() / r : qApp->desktop()->height() / r;
	renderer->widget()->resize(w, h);
	mpPlayer->addVideoRenderer(renderer);
	int i = (mRenderers.size() - 1) / cols();
	int j = (mRenderers.size() - 1) % cols();

	if (view) 
	{
		((QGridLayout*)view->layout())->addWidget(renderer->widget(), i, j);
		view->show();
	}
	else 
	{
		renderer->widget()->move(j*w, i*h);
		renderer->widget()->show();
	}
	//updateROI();
}

void VideoGroup::removeRenderer()
{
	{
		m_currentScreenIndex = m_currentScreenIndex > 0 ? m_currentScreenIndex - 1 : m_currentScreenIndex;
		int currentScreen = m_supportScreen[m_currentScreenIndex];
		while (mRenderers.size() > currentScreen)
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
		return;
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

	return;
}

void VideoGroup::updateROI()
{
	return;
	if (mRenderers.isEmpty())
		return;
	
	foreach(VideoRenderer *renderer, mRenderers) 
	{
		renderer->setRegionOfInterest(0, 0, 0, 0);
	}
	
	return;
}
