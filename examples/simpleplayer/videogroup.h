#ifndef QTAV_VIDEOWALL_H
#define QTAV_VIDEOWALL_H

#include <QtCore/QList>
#include <QtAV/AVPlayer.h>
#include <QtAV/WidgetRenderer.h>
#include <QtWidgets/QPushButton>

class VideoGroup : public QObject
{
	Q_OBJECT
public:
	explicit VideoGroup(QObject *parent = 0);
	~VideoGroup();
	void setRows(int n);
	void setCols(int n);
	int rows() const;
	int cols() const;

	void play(const QString& file);
	void updateROI();

	public slots:
	void openLocalFile();
	void addRenderer();
	void removeRenderer();

private:
	int r, c;
	int timer_id;
	QtAV::AVPlayer *mpPlayer;
	QWidget *view;
	QWidget *mpBar;
	QPushButton *mpAdd, *mpRemove, *mpOpen, *mpPlay, *mpStop, *mpPause;

	QList<QtAV::VideoRenderer*> mRenderers;
	int		m_supportScreen[6];
	int		m_currentScreenIndex;
};

#endif // QTAV_VIDEOWALL_H
