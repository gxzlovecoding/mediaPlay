#ifndef PLAYLISTTREEVIEW_H
#define PLAYLISTTREEVIEW_H

#include <QListWidget>
#include <QImage>
#include <QLabel>
#include <QtGui>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QImage>
#include <QString>
#include "dragicon.h"

class ProgramItem : public QWidget
{
	Q_OBJECT
public:
	ProgramItem(QWidget *parent = 0);
	~ProgramItem();
	void init(int iconW, int iconH, QString itemName, QImage *image);
private:
	QString m_programName;
};

class PlaylistTreeView : public QWidget
{
	Q_OBJECT
public:
	PlaylistTreeView(QWidget *parent = 0);
	~PlaylistTreeView();

	void addItem(QString itemName, QImage *image);
	void clear(void);

public slots:
	void valueChanged(int value);
private:
	QHBoxLayout *m_mainLayout;
	QVBoxLayout *m_leftLayout;
};

#endif


