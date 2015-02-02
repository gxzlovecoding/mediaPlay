#include "PlaylistTreeView.h"

// ���ó�16:9
#define W_ICONSIZE 178
#define H_ICONSIZE 100

PlaylistTreeView::PlaylistTreeView(QWidget *parent)
: QListWidget(parent)
{
	this->adjustSize();

	//����QListWidget�еĵ�Ԫ���ͼƬ��С
	this->setIconSize(QSize(W_ICONSIZE, H_ICONSIZE));
	this->setResizeMode(QListView::Adjust);
	//����QListWidget����ʾģʽ
	this->setViewMode(QListView::IconMode);
	//����QListWidget�еĵ�Ԫ��ɱ��϶�
	this->setMovement(QListView::Snap);
	//����QListWidget�еĵ�Ԫ��ļ��
	this->setSpacing(2);

	// �����������ͼ��
	// this->setCursor(Qt::CrossCursor);
}

void PlaylistTreeView::addItem(QString itemName, QImage *image)
{
	//����QListWidgetItem����(ע�⣺��Iconͼ�����������[96*96])---scaled����
	QListWidgetItem *pItem = new QListWidgetItem(QIcon(QPixmap::fromImage(image->scaled(QSize(W_ICONSIZE, H_ICONSIZE)))), itemName);
	//���õ�Ԫ��Ŀ�Ⱥ͸߶�
	pItem->setSizeHint(QSize(W_ICONSIZE, H_ICONSIZE + 20));
	this->insertItem(this->count(), pItem);
}

void PlaylistTreeView::clear(void)
{
}

PlaylistTreeView::~PlaylistTreeView()
{
}
