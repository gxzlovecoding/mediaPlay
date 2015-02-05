#include "PlaylistTreeView.h"

// 设置成16:9
#define W_ICONSIZE 178
#define H_ICONSIZE 100

PlaylistTreeView::PlaylistTreeView(QWidget *parent)
: QListWidget(parent)
{
	this->adjustSize();

	//设置QListWidget中的单元项的图片大小
	this->setIconSize(QSize(W_ICONSIZE, H_ICONSIZE));
	this->setResizeMode(QListView::Adjust);
	//设置QListWidget的显示模式
	this->setViewMode(QListView::IconMode);
	//设置QListWidget中的单元项不可被拖动
	this->setMovement(QListView::Snap);
	//设置QListWidget中的单元项的间距
	this->setSpacing(2);

	// 用来设置鼠标图标
	// this->setCursor(Qt::CrossCursor);
}

void PlaylistTreeView::addItem(QString itemName, QImage *image)
{
	QString name = itemName;
	if (!itemName.isEmpty())
	{
		int i = itemName.indexOf(QString("&"));
		//QString index = itemName.right(itemName.size() - i - 1);
		name = itemName.left(i);
	}

	//生成QListWidgetItem对象(注意：其Icon图像进行了伸缩[96*96])---scaled函数
	QListWidgetItem *pItem = new QListWidgetItem(QIcon(QPixmap::fromImage(image->scaled(QSize(W_ICONSIZE, H_ICONSIZE)))), name);
	//设置单元项的宽度和高度
	pItem->setSizeHint(QSize(W_ICONSIZE, H_ICONSIZE + 20));
	this->insertItem(this->count(), pItem);
}

PlaylistTreeView::~PlaylistTreeView()
{
}
