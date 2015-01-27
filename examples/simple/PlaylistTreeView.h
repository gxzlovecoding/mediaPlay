#ifndef PLAYLISTTREEVIEW_H
#define PLAYLISTTREEVIEW_H

#include <QListWidget>
#include <QImage>
#include <QLabel>
#include <QtGui>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollBar>

class PlaylistTreeView : public QWidget
{
	Q_OBJECT
public:
	PlaylistTreeView(QWidget *parent = 0);
	~PlaylistTreeView();

	void addItem(QString itemName, QImage *image);
	void clear(void);

private:
	QHBoxLayout *m_mainLayout;
	QVBoxLayout *m_leftLayout;
};

#endif


