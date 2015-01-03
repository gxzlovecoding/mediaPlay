#include "PlaylistTreeView.h"


PlaylistTreeView::PlaylistTreeView(QWidget *parent)
: QTreeView(parent)
{
	this->adjustSize();

	m_model = new QStandardItemModel();
	this->setModel(m_model);

	QStandardItem *item = new QStandardItem("player");
	m_model->setHorizontalHeaderItem(0, item);
}

void PlaylistTreeView::addItem(QString itemName, QString value)
{
	QStandardItem *item = new QStandardItem(itemName);
	item->setData(value);
	m_model->appendRow(item);
}

PlaylistTreeView::~PlaylistTreeView()
{
	delete m_model;
}
