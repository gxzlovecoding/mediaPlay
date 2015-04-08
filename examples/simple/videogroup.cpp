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
#include <QToolTip>

#include <QtAV/VideoRendererTypes.h>
#include <QtAV/GLWidgetRenderer2.h>
#include <QtAV/WidgetRenderer.h>

using namespace QtAV;
const qreal kVolumeInterval = 0.05;

static QFile *logFile;
static QMutex mutex;

// 支持中文
#if _MSC_VER >= 1600  
#pragma execution_character_set("utf-8")  
#endif  

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define qInstallMessageHandler qInstallMsgHandler
void Logger(QtMsgType type, const char *msg)
{
#else
void Logger(QtMsgType type, const QMessageLogContext &, const QString& qmsg)
{
	mutex.lock();
	//const char* msg = qUtf8Printable(qmsg);
#endif
	fprintf(stdout, "%s", QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]  ").toStdString().c_str());
	if (logFile->isOpen())
	{
		QTextStream fout(&(*logFile));
		fout << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]  ");
	}

	switch (type) {
	case QtDebugMsg:
		fprintf(stdout, "Debug: %s\n", qmsg);
		if (logFile->isOpen())
		{
			QTextStream fout(&(*logFile));
			fout << "Debug: " << qmsg << endl;
		}
		break;
	case QtWarningMsg:
		fprintf(stdout, "Warning: %s\n", qmsg);
		if (logFile->isOpen())
		{
			QTextStream fout(&(*logFile));
			fout << "Warning: " << qmsg << endl;
		}
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s\n", qmsg);
		if (logFile->isOpen())
		{
			QTextStream fout(&(*logFile));
			fout << "Critical: " << qmsg << endl;
		}
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", qmsg);
		if (logFile->isOpen())
		{
			QTextStream fout(&(*logFile));
			fout << "Fatal: " << qmsg << endl;
		}
		abort();
	}
	fflush(0);
	mutex.unlock();
}

VideoGroup::VideoGroup(QWidget *parent) :
QWidget(parent)
, mpPlayer(0)
, view(0)
, m_isFullscreen(false)
, m_currentScreens(4)
, m_isMute(false)
, m_intervalTimer(new QTimer(this))
, m_isLive(true)
{
	QString runPath = QCoreApplication::applicationDirPath();
	runPath.append("/log.txt");
	logFile = new QFile(runPath);//"log.txt");
	
	if (!logFile->open(QIODevice::WriteOnly))
	{
		qWarning("Failed to open log file");
	}
	else
	{
		qInstallMessageHandler(Logger);
	}

	qDebug("init.......");

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	//setLayout(mainLayout);
	mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);

	mpBar = new QWidget(0, Qt::WindowStaysOnTopHint);
	mpBar->setMaximumHeight(40);
	mpBar->setLayout(new QHBoxLayout);

	mpTimeSlider = new Slider();
	mpTimeSlider->setDisabled(true);
	mpTimeSlider->setMaximumHeight(8);
	mpTimeSlider->setTracking(true);
	mpTimeSlider->setOrientation(Qt::Horizontal);
	mpTimeSlider->setMinimum(0);

	mpPlayPause = new StyleButton();
	mpPlayPause->setMaximumSize(40, 40);
	setPlayPauseButtonStyle();
	//mpStop = new QPushButton();
	//mpStop->setMaximumSize(40, 40);
	//mpStop->setObjectName("mpStop");
	mpForwardBtn = new StyleButton();
	mpForwardBtn->setMaximumSize(40, 40);
	mpForwardBtn->setObjectName("mpForwardBtn");
	mpBackwardBtn = new StyleButton();
	mpBackwardBtn->setMaximumSize(40, 40);
	mpBackwardBtn->setObjectName("mpBackwardBtn");

	mpMute = new StyleButton();
	mpMute->setMaximumSize(40, 40);
	mpMute->setObjectName("mpMute");
	setMuteButtonStyle();

	mpVolumeSlider = new Slider();
	//mpVolumeSlider->hide();
	mpVolumeSlider->setOrientation(Qt::Horizontal);
	mpVolumeSlider->setMinimum(0);
	const int kVolumeSliderMax = 100;
	mpVolumeSlider->setMaximum(kVolumeSliderMax);
	//mpVolumeSlider->setMaximumHeight(8);
	mpVolumeSlider->setMaximumWidth(88);
	mpVolumeSlider->setValue(int(1.0 / kVolumeInterval*qreal(kVolumeSliderMax) / 100.0));

	mpFullscreenBtn = new StyleButton();
	mpFullscreenBtn->setMaximumSize(40, 40);
	mpFullscreenBtn->setObjectName("mpFullscreenBtn");
	setFullScreenButtonStyle();

	connect(mpFullscreenBtn, SIGNAL(clicked()), SLOT(setFullscreen()));
	connect(mpPlayPause, SIGNAL(clicked()), this, SLOT(onPauseResumeClick()));
	connect(mpTimeSlider, SIGNAL(sliderPressed()), SLOT(seek()));
	connect(mpTimeSlider, SIGNAL(sliderReleased()), SLOT(seek()));
	connect(mpMute, SIGNAL(clicked()), this, SLOT(setMute()));
	connect(mpVolumeSlider, SIGNAL(sliderPressed()), SLOT(setVolume()));
	connect(mpVolumeSlider, SIGNAL(valueChanged(int)), SLOT(setVolume()));

	resetPlayer();

	// 添加时间显示
	mpTimeLabel = new QLabel("0:0/0:0");
	mpTimeLabel->setMinimumWidth(80);
	mpTimeLabel->setAlignment(Qt::AlignCenter);
	mpBar->layout()->addWidget(mpTimeLabel);

	for (int i = 0; i < 10; i++)
	{
		mpSplitScreen[i] = NULL;
		if (i == 1 || i == 2 || i == 3 || i == 4 || i == 9)
		{
			mpSplitScreen[i] = new StyleButton(QString("%1").arg(i));
			mpSplitScreen[i]->setMaximumSize(40, 40);
			connect(mpSplitScreen[i], SIGNAL(onClick(QString)), SLOT(setRenderer(QString)));
			mpBar->layout()->addWidget(mpSplitScreen[i]);
		}
	}
	QSpacerItem *left = new QSpacerItem(80, 50, QSizePolicy::Expanding, QSizePolicy::Minimum);
	mpBar->layout()->addItem(left);
	mpBar->layout()->addWidget(mpBackwardBtn);
	mpBar->layout()->addWidget(mpPlayPause);
	//mpBar->layout()->addWidget(mpStop);
	mpBar->layout()->addWidget(mpForwardBtn);
	QSpacerItem *right = new QSpacerItem(60, 50, QSizePolicy::Expanding, QSizePolicy::Minimum);
	mpBar->layout()->addItem(right);
	mpBar->layout()->addWidget(mpMute);
	mpBar->layout()->addWidget(mpVolumeSlider);
	mpBar->layout()->addWidget(mpFullscreenBtn);

	QPalette palette;
	mpBar->setAutoFillBackground(true);
	//palette.setColor(QPalette::Background, QColor(192, 253, 123));
	palette.setBrush(QPalette::Background, QBrush(QPixmap(":/simple/resources/background.png")));
	mpBar->setPalette(palette);

	view = new QWidget;
	QGridLayout *layout = new QGridLayout;
	layout->setSizeConstraint(QLayout::SetMaximumSize);
	layout->setSpacing(1);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);
	view->setLayout(layout);

	updateScreen(m_currentScreens);

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

	// 设置样式
	QFile styleSheet(":/simple/resources/style.qss");
	if (!styleSheet.open(QIODevice::ReadOnly))
	{
		qWarning("Can't open the style sheet file.");
	}
	qApp->setStyleSheet(styleSheet.readAll());

	connect(m_intervalTimer, SIGNAL(timeout()), SLOT(intervalTimerExpired()));
	m_intervalTimer->start(500);
}

VideoGroup::~VideoGroup()
{
	if (!mpPlayer->isLoaded())
	{
		return;
	}

	mpPlayer->stop();
}

void VideoGroup::resetPlayer(void)
{
	if (mpPlayer && mpPlayer->isLoaded())
	{
		mpPlayer->stop();
		delete mpPlayer;
		mpPlayer = NULL;
	}

	mpPlayer = new AVPlayer(this);
	connect(mpPlayer, SIGNAL(preloadSuccess()), this, SLOT(preloadSuccess()));
	connect(mpPlayer, SIGNAL(started()), this, SLOT(onStartPlay()));
	connect(mpPlayer, SIGNAL(stopped()), this, SLOT(onStopPlay()));
	connect(mpPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChange(qint64)));
	connect(mpForwardBtn, SIGNAL(clicked()), mpPlayer, SLOT(seekForward()));
	connect(mpBackwardBtn, SIGNAL(clicked()), mpPlayer, SLOT(seekBackward()));
}

void VideoGroup::setVolume()
{
	if (!mpPlayer->isLoaded())
	{
		return;
	}
	for (int i = 0; i < mpPlayer->audioStreamCount(); i++)
	{
		AudioOutput *ao = mpPlayer ? mpPlayer->audio(i) : 0;
		qreal v = qreal(mpVolumeSlider->value())*kVolumeInterval;
		if (ao)
		{
			ao->setVolume(v);
		}
	}
}

void VideoGroup::setMute()
{
	m_isMute = !m_isMute;
	setMuteButtonStyle();

	if (!mpPlayer->isLoaded())
	{
		return;
	}
	
	for (int i = 0; i < mpPlayer->audioStreamCount(); i++)
	{
		mpPlayer->setMute(m_isMute, i);
	}
}

void VideoGroup::intervalTimerExpired()
{
	if (m_isFullscreen)
	{
		static int notMoveCount = 0;
		static QPoint currentPos(QWidget::mapFromGlobal(QCursor::pos()));

		notMoveCount = (currentPos == QWidget::mapFromGlobal(QCursor::pos())) ? notMoveCount + 1 : 0;
		currentPos = QWidget::mapFromGlobal(QCursor::pos());

		if (notMoveCount > 6)
		{
			if (this->mpTimeSlider->isVisible())
				this->mpTimeSlider->setVisible(false);
			if (this->mpBar->isVisible())
				this->mpBar->setVisible(false);
			QApplication::setOverrideCursor(Qt::BlankCursor);
		}
		else
		{
			if (!this->mpTimeSlider->isVisible())
				this->mpTimeSlider->setVisible(true);
			if (!this->mpBar->isVisible())
				this->mpBar->setVisible(true);
			QApplication::setOverrideCursor(Qt::ArrowCursor);
		}
	}
}

void VideoGroup::setFullscreen()
{
	static QWidget* temp = new QWidget();
	if (!m_isFullscreen)
	{
		// TODO 把全屏放到一个widket做全屏，因为单独全屏有问题。
		temp->setWindowFlags(Qt::Window);
		temp->setLayout(new QVBoxLayout());
		temp->layout()->setSpacing(0);
		temp->layout()->setMargin(0);

		temp->layout()->addWidget(mainWidget);
		temp->showFullScreen();
		//mainWidget->move(0, 0);
		//mainWidget->resize(temp->size());
	}
	else
	{
		temp->layout()->removeWidget(mainWidget);
		temp->setWindowFlags(Qt::Widget);
		temp->hide();

		// 把窗口重新放回splitter
		m_pSplitter->addWidget(mainWidget);

		// 设置列表和主窗口左右比例
		QList<int> size;
		size.append(this->width() * 0.2);
		size.append(this->width() * 0.8);
		m_pSplitter->setSizes(size);
	}
	m_isFullscreen = !m_isFullscreen;
	setFullScreenButtonStyle();
}

void VideoGroup::onPauseResumeClick()
{
	if (mpPlayer && mpPlayer->isLoaded())
	{
		if (mpPlayer->isPlaying())
		{
			// 如果主显示区域没有拖视频窗口进来，提示用户
			int enableProgramCount = 0;
			for (int i = 0; i < m_currentScreens ; i++)
			{
				enableProgramCount += mpPlayer->isEnableProgram(i) ? 1 : 0;
			}

			if (enableProgramCount == 0)
			{
				QToolTip::showText(QCursor::pos(), tr("请拖动视频进显示区播放！"));
				return;
			}

			mpPlayer->pause(!mpPlayer->isPaused());
		}
		else
		{
			mpPlayer->play();
		}
	}

	setPlayPauseButtonStyle();
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

QString VideoGroup::about(void)
{
	return QString("version : 2015-02-09_1");
}

void VideoGroup::preload(const QString& file)
{
	resetPlayer();

	mpPlayer->preLoad(file);

	m_url = file;
}

void VideoGroup::preloadSuccess()
{
	if (!mpPlayer->isLoaded())
		return;

	// 设置播放按钮
	setPlayPauseButtonStyle();

	// 初始化声音
	setVolume();
	m_isMute = !m_isMute;
	setMute();

	mpPlayer->disableAllProgram();

	// 取出视频自带的私有流名字数据
	m_playList->clear();
	for (int i = 0; i < mpPlayer->videoStreamCount(); i++)
	{
		std::string streamName = mpPlayer->getVideoStreamName(i);
		m_playList->addItem(QString::fromLocal8Bit(streamName.c_str()), mpPlayer->getFirstImage(i));
	}

	// 如果是udp直播 就把快进快退disable掉
	if (m_url.startsWith("udp:"))
	{
		mpForwardBtn->setDisabled(true);
		mpBackwardBtn->setDisabled(true);
		m_isLive = true;
		m_duration = 0;
	}
	else
	{
		mpForwardBtn->setEnabled(true);
		mpBackwardBtn->setEnabled(true);
		m_isLive = false;
		m_duration = mpPlayer->duration() / 1000;
	}	

	mpTimeLabel->setText(QString("0:0/%1:%2").arg(m_duration / 60).arg(m_duration % 60));

	// 初始化时显示区域为空
	/*
	for (int i = 0; i < m_currentScreens && i < 1; i++)
	{
		mRenderers[i]->receive(mpPlayer->getFirstFrame(i));
		mRenderers[i]->setProgramIndex(i);
		mpPlayer->setRenderer(mRenderers[i], i);
		mpPlayer->enableProgram(i);
	}
	*/
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

	// 停止播放后，回到初始状态
	for (int i = 0; i < m_currentScreens ; i++)
	{
		if (mpPlayer->isEnableProgram(i))
		{
			mpPlayer->renderer(i)->receive(mpPlayer->getFirstFrame(i));
		}
	}
	// 播放按键也回到初始状态
	setPlayPauseButtonStyle();
}

void VideoGroup::onPositionChange(qint64 pos)
{
	mpTimeSlider->setValue(pos);

	if (!m_isLive)
	{
		int currentTime = pos / 1000;
		mpTimeLabel->setText(QString("%1:%2/%3:%4")\
			.arg(currentTime / 60).arg(currentTime % 60).arg(m_duration / 60).arg(m_duration % 60));
	}
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

void VideoGroup::setRenderer(QString name)
{
	updateScreen(name.toInt());
}

void VideoGroup::setRenderByDrag(QtAV::VideoRenderer* render)
{
	int renderIndex = mRenderers.indexOf(render);
	int programIndex = m_playList->currentRow();

	// 关闭目标render当前节目
	int oldProgramIndex = render->getProgramIndex(); 
	if (oldProgramIndex >= 0)
	{
		// 把源来的program关闭
		mpPlayer->disableProgram(oldProgramIndex);
		mpPlayer->setRenderer(0, oldProgramIndex);
	}

	// 关闭源render当前节目
	// 根据列表节目，找出这个节目的源render
	QtAV::VideoRenderer* oldRender = mpPlayer->renderer(programIndex);

	// 目标render设置
	mRenderers[renderIndex]->receive(mpPlayer->getFirstFrame(programIndex));
	mRenderers[renderIndex]->setProgramIndex(programIndex);
	mpPlayer->setRenderer(mRenderers[renderIndex], programIndex);
	mpPlayer->enableProgram(programIndex);

	//把源render黑掉
	if (oldRender)
	{
		//VideoFrame *temp = new VideoFrame();
		VideoFrame temp;
		oldRender->receive(temp);
		oldRender->setProgramIndex(-1);
	}
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

			//VideoRenderer* renderer = new GLWidgetRenderer2();//VideoRendererFactory::create(v);
			//GLWidgetRenderer2 *renderer = new GLWidgetRenderer2();
			WidgetRenderer *renderer = new WidgetRenderer();
			connect(renderer, SIGNAL(setRenderByDrag(QtAV::VideoRenderer*)), this, SLOT(setRenderByDrag(QtAV::VideoRenderer*)));
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
			// 删除这个窗口前，把这个窗口从播放线程中释放
			// 关闭目标render当前节目
			int oldProgramIndex = r->getProgramIndex();
			if (oldProgramIndex >= 0)
			{
				// 把源来的program关闭
				mpPlayer->disableProgram(oldProgramIndex);
				mpPlayer->setRenderer(0, oldProgramIndex);
			}

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

	setSplitScreenButtonStyle(num);
	preloadSuccess();
}

void VideoGroup::setMuteButtonStyle(void)
{
	if (m_isMute)
	{
		mpMute->setStyle(QString("QPushButton {border-image: url(:/simple/resources/mute_down.png);}"), \
			QString("QPushButton {border-image: url(:/simple/resources/mute.png);}"));
	}
	else
	{
		mpMute->setStyle(QString("QPushButton {border-image: url(:/simple/resources/unmute_down.png);}"), \
			QString("QPushButton {border-image: url(:/simple/resources/unmute.png); }"));
	}
}

void VideoGroup::setFullScreenButtonStyle(void)
{
	if (!m_isFullscreen)
	{
		mpFullscreenBtn->setStyle(QString("QPushButton {border-image: url(:/simple/resources/full_down.png);}"), \
			QString("QPushButton {border-image: url(:/simple/resources/full.png);}"));
	}
	else
	{
		mpFullscreenBtn->setStyle(QString("QPushButton {border-image: url(:/simple/resources/unFull_down.png);}"), \
			QString("QPushButton {border-image: url(:/simple/resources/unFull.png);}"));
	}
}

void VideoGroup::setSplitScreenButtonStyle(int num)
{
	for (int i = 0; i < 10; i++)
	{
		if (mpSplitScreen[i])
		{
			mpSplitScreen[i]->setStyle(QString("QPushButton {border-image: url(:/simple/resources/%1_down.png);}").arg(i), \
				QString("QPushButton {border-image: url(:/simple/resources/%1.png);}").arg(i));
		}
	}

	if (mpSplitScreen[num])
		mpSplitScreen[num]->setStyle(QString("QPushButton {border-image: url(:/simple/resources/%1_down.png);}").arg(num), \
		QString("QPushButton {border-image: url(:/simple/resources/%1_down.png);}").arg(num));
}

void VideoGroup::setPlayPauseButtonStyle(void)
{
	if (mpPlayer && mpPlayer->isLoaded() && (mpPlayer->isPaused() | !mpPlayer->isPlaying()))
	{
		mpPlayPause->setStyle(QString("QPushButton {border-image: url(:/simple/resources/play_down.png);}"), \
			QString("QPushButton {border-image: url(:/simple/resources/play.png);}"));
	}
	else
	{
		mpPlayPause->setStyle(QString("QPushButton {border-image: url(:/simple/resources/pause_down.png);}"), \
			QString("QPushButton {border-image: url(:/simple/resources/pause.png);}"));
	}
}
