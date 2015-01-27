#include "PlaylistTreeView.h"

ProgramItem::ProgramItem(QWidget *parent)
: QWidget(parent)
{

}

void ProgramItem::init(int iconW, int iconH, QString itemName, QImage *image)
{
	QGridLayout *ItemVBLayout = new QGridLayout();
	this->setLayout(ItemVBLayout);

	QLabel *programIcon = new QLabel;
	programIcon->setPixmap(QPixmap::fromImage(*image).scaled(iconW, iconH));

	QPushButton *voiceButton = new QPushButton();
	voiceButton->setStyleSheet(QString("QPushButton {color: red;  border-image: url(:/simple/resources/mute.png); max-height: 30px;    max-width: 30px;  }"));
	voiceButton->setMaximumHeight(16);
	voiceButton->setMaximumWidth(16);

	m_programName = itemName;
	QLabel *programName = new QLabel(itemName);
	programName->setAlignment(Qt::AlignCenter);
	programName->setMaximumHeight(16);

	ItemVBLayout->addWidget(programIcon, 0, 0, 1, 2);
	ItemVBLayout->addWidget(voiceButton, 1, 0);
	ItemVBLayout->addWidget(programName, 1, 1);
}

ProgramItem::~ProgramItem()
{

}

PlaylistTreeView::PlaylistTreeView(QWidget *parent)
: QWidget(parent)
{
	m_mainLayout = new QHBoxLayout();
	m_mainLayout->setSpacing(0);
	m_mainLayout->setMargin(0);
	this->setLayout(m_mainLayout);

	m_leftLayout = new QVBoxLayout();
	m_leftLayout->setSpacing(0);
	m_leftLayout->setMargin(0);

	m_mainLayout->addLayout(m_leftLayout);
}

void PlaylistTreeView::addItem(QString itemName, QImage *image)
{
	static int remainHeight = this->size().height();

	int pWidth = this->size().width() - 40;
	int pHeight = pWidth * 0.5625;

	// 如果剩下的高度不够放下一个节目，就不显示了
	if (remainHeight < pHeight + 40)
	{
		// 节目列表超出了显示范围，引出滚动条
		static bool flag = false;
		if (!flag)
		{
			QScrollBar *bar = new QScrollBar();
			bar->setPageStep(1);
			bar->setMaximum(5);
			bar->setValue(0);
			int a = bar->value();
			connect(bar, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
			m_mainLayout->addWidget(bar);
			flag = true;

			QSpacerItem *left = new QSpacerItem(pWidth, remainHeight, QSizePolicy::Expanding, QSizePolicy::Minimum);
			m_leftLayout->addItem(left);
		}

		return;
	}
	else
	{
		remainHeight -= (pHeight + 40);
	}

	ProgramItem* itemWidget = new ProgramItem();
	itemWidget->init(pWidth, pHeight, itemName, image);
	m_leftLayout->addWidget(itemWidget);
}

void PlaylistTreeView::valueChanged(int value)
{
	int v = value;
}

void PlaylistTreeView::clear(void)
{
}

PlaylistTreeView::~PlaylistTreeView()
{
}
