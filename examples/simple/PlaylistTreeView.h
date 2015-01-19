#ifndef PLAYLISTTREEVIEW_H
#define PLAYLISTTREEVIEW_H

#include <QListWidget>
#include <QImage>
#include <QtGui>

class PlaylistTreeView : public QListWidget
{
	Q_OBJECT
public:
	PlaylistTreeView(QWidget *parent = 0);
	~PlaylistTreeView();

	void addItem(QString itemName, QImage *image);
	void clear(void);

private:
};

#endif


