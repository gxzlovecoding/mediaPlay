#include "StyleButton.h"

StyleButton::StyleButton(QString name, QWidget *parent)
: QPushButton(parent)
, m_name(name)
, m_isEnter(false)
{
	this->setMouseTracking(true);
}

void StyleButton::setStyle(QString enterStyle, QString leaveStyle)
{
	m_enterStyle = enterStyle;
	m_leaveStyle = leaveStyle;
	if (m_isEnter && !m_enterStyle.isEmpty())
	{
		this->setStyleSheet(m_enterStyle);
	}
	else if (!m_leaveStyle.isEmpty())
	{
		this->setStyleSheet(m_leaveStyle);
	}
}

void StyleButton::mousePressEvent(QMouseEvent * e)
{
	emit onClick(m_name);
	QPushButton::mousePressEvent(e);
}


void StyleButton::enterEvent(QEvent *e)
{
	m_isEnter = true;
	if (!m_enterStyle.isEmpty())
	{
		this->setStyleSheet(m_enterStyle);
	}

	QPushButton::enterEvent(e);
}

void StyleButton::leaveEvent(QEvent *e)
{
	m_isEnter = false;
	if (!m_leaveStyle.isEmpty())
	{
		this->setStyleSheet(m_leaveStyle);
	}

	QPushButton::leaveEvent(e);
}
