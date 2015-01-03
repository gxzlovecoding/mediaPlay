#ifndef PLAYLISTTREEVIEW_H
#define PLAYLISTTREEVIEW_H

#include <QTreeView>
#include <QtGui>

class PlaylistTreeView : public QTreeView
{
	Q_OBJECT
public:
	PlaylistTreeView(QWidget *parent = 0);
	~PlaylistTreeView();

	void addItem(QString itemName, QString value = "");
	
private:
	QStandardItemModel *m_model;
};

#endif


