#include "PlaylistTreeView.h"

ProgramItem::ProgramItem(int id, QWidget *parent)
: QWidget(parent)
, m_isMute(false)
, m_id(id)
{

}

ProgramItem* ProgramItem::clone()
{
	ProgramItem* item = new ProgramItem(m_id);
	item->init(m_programName, m_programPixmap);
	item->setMute(m_isMute);
	return item;
}

void ProgramItem::setMute(bool flag)
{
	m_isMute = flag;
	if (m_isMute)
		m_voiceButton->setStyleSheet(QString("QPushButton {color: red;  border-image: url(:/simple/resources/mute.png); max-height: 30px;    max-width: 30px;  }"));
	else
		m_voiceButton->setStyleSheet(QString("QPushButton {color: red;  border-image: url(:/simple/resources/unmute.png); max-height: 30px;    max-width: 30px;  }"));
}

void ProgramItem::init(QString itemName, QPixmap *image)
{
	m_programName = itemName;
	m_programPixmap = new QPixmap(*image);

	QGridLayout *ItemVBLayout = new QGridLayout();
	this->setLayout(ItemVBLayout);
	
	m_programIcon = new DragIcon(*m_programPixmap);
	connect(m_programIcon, SIGNAL(onClick()), this, SLOT(onClick()));

	m_voiceButton = new QPushButton();
	m_voiceButton->setMaximumHeight(16);
	m_voiceButton->setMaximumWidth(16);
	connect(m_voiceButton, SIGNAL(clicked()), this, SLOT(onMuteClick()));
	setMute(false);

	m_programNameLabel = new QLabel(itemName);
	m_programNameLabel->setAlignment(Qt::AlignCenter);
	m_programNameLabel->setMaximumHeight(16);

	ItemVBLayout->addWidget(m_programIcon, 0, 0, 1, 2);
	ItemVBLayout->addWidget(m_voiceButton, 1, 0);
	ItemVBLayout->addWidget(m_programNameLabel, 1, 1);
}

void ProgramItem::onMuteClick()
{
	setMute(!m_isMute);
}

void ProgramItem::onClick()
{
	emit activeItem(m_id);
}

ProgramItem::~ProgramItem()
{
	if (m_programPixmap)
		delete m_programPixmap;
	if (m_programIcon)
		delete m_programIcon;
	if (m_voiceButton)
		delete m_voiceButton;
	if (m_programNameLabel)
		delete m_programNameLabel;
}

PlaylistTreeView::PlaylistTreeView(QWidget *parent)
: QWidget(parent)
, m_displayNumber(0)
, m_totalProbramNumber(0)
{
	m_mainLayout = new QHBoxLayout();
	m_mainLayout->setSpacing(0);
	m_mainLayout->setMargin(0);
	this->setLayout(m_mainLayout);

	m_leftLayout = new QVBoxLayout();
	m_leftLayout->setSpacing(0);
	m_leftLayout->setMargin(0);

	m_scrollBar = new QScrollBar();
	m_scrollBar->setPageStep(1);
	m_scrollBar->setMaximum(0);
	m_scrollBar->setValue(0);
	connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));

	m_mainLayout->addLayout(m_leftLayout);
}

void PlaylistTreeView::addItem(QString itemName, QImage *image)
{
	static int remainHeight = this->size().height();

	int pWidth = this->size().width() - 40;
	int pHeight = pWidth * 0.5625;

	// 生成节目Item
	ProgramItem* itemWidget = new ProgramItem(m_totalProbramNumber++);
	itemWidget->init(itemName, &QPixmap::fromImage(*image).scaled(pWidth, pHeight));

	// 放入List
	m_programList.append(itemWidget);

	if (remainHeight > pHeight + 40)
	{
		remainHeight -= (pHeight + 40);

		// 添加进去左边列表
		ProgramItem *item = itemWidget->clone();
		connect(item, SIGNAL(activeItem(int)), this, SLOT(activeItem(int)));
		m_leftLayout->addWidget(item);
		m_currentDisplayList.append(item);
		m_displayNumber++;
	}
	else if (remainHeight > 0)
	{
		// 如果剩下的高度不够放下一个节目，就不显示了
		// 节目列表超出了显示范围，引出滚动条
		m_mainLayout->addWidget(m_scrollBar);

		//QSpacerItem *left = new QSpacerItem(pWidth, remainHeight, QSizePolicy::Expanding, QSizePolicy::Minimum);
		//m_leftLayout->addItem(left);
		remainHeight = 0;
	}

	if (m_totalProbramNumber > m_displayNumber)
		m_scrollBar->setMaximum(m_totalProbramNumber - m_displayNumber);
}

void PlaylistTreeView::valueChanged(int value)
{
	//int v = value;
	while (!m_currentDisplayList.empty())
	{
		ProgramItem *item = m_currentDisplayList.takeLast();
		m_programList[item->getID()]->setMute(item->getMute());
		m_leftLayout->removeWidget(item);
		delete item;
	}

	for (int i = 0; i < m_displayNumber; i++)
	{
		ProgramItem *item = m_programList[i + value]->clone();
		connect(item, SIGNAL(activeItem(int)), this, SLOT(activeItem(int)));
		m_leftLayout->addWidget(item);
		m_currentDisplayList.append(item);
	}
}

void PlaylistTreeView::activeItem(int id)
{
	m_activeItem = id;
}

void PlaylistTreeView::clear(void)
{
}

PlaylistTreeView::~PlaylistTreeView()
{
}
