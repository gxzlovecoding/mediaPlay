#include "PlaylistTreeView.h"

// 设置成16:9
#define W_ICONSIZE 178
#define H_ICONSIZE 100

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
			bar->setPageStep(30);
			bar->setValue(3);
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

	QWidget *itemWidget = new QWidget();
	QGridLayout *ItemVBLayout = new QGridLayout();
	itemWidget->setLayout(ItemVBLayout);

	QLabel *programIcon = new QLabel;
	programIcon->setPixmap(QPixmap::fromImage(*image).scaled(pWidth, pHeight));

	QPushButton *voiceButton = new QPushButton();
	voiceButton->setStyleSheet(QString("QPushButton {color: red;  border-image: url(:/simple/resources/mute.png); max-height: 30px;    max-width: 30px;  }"));
	voiceButton->setMaximumHeight(16);
	voiceButton->setMaximumWidth(16);

	QLabel *programName = new QLabel(itemName);
	programName->setAlignment(Qt::AlignCenter);
	programName->setMaximumHeight(16);

	ItemVBLayout->addWidget(programIcon, 0, 0, 1, 2);
	ItemVBLayout->addWidget(voiceButton, 1, 0);
	ItemVBLayout->addWidget(programName, 1, 1);

	m_leftLayout->addWidget(itemWidget);
}

void PlaylistTreeView::clear(void)
{
}

PlaylistTreeView::~PlaylistTreeView()
{
}
