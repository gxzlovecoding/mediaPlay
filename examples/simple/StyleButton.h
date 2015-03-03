#ifndef STYLEBUTTON_H
#define STYLEBUTTON_H

#include <QtGui>
#include <QString>
#include <QPushButton>

class StyleButton : public QPushButton
{
    Q_OBJECT
public:
	StyleButton(QString name = "", QWidget *parent = 0);
	void setStyle(QString enterStyle, QString leaveStyle);
	
signals:
	void onClick(QString name);
protected:
    void mousePressEvent(QMouseEvent *);
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
private:
	QString m_name;
	bool    m_isEnter;
	QString m_enterStyle;
	QString m_leaveStyle;
};

#endif // DRAGICON_H
