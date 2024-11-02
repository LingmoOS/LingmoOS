#include "klabel.h"

#include "../util.h"

KLabel::KLabel(QWidget *parent)
    : QLabel(parent)
{
    m_strText = "";
}

KLabel::KLabel(QString strText, QWidget *parent)
    : QLabel(strText, parent)
{
    m_strText = strText;
}

void KLabel::setText(const QString &strText)
{
    m_strText = strText;
    QLabel::setText(strText);
}

void KLabel::paintEvent(QPaintEvent *event)
{
    QString strEText = getElidedText(font(), m_strText, width());
    if (strEText != m_strText) {
        QLabel::setText(strEText);
        setToolTip(m_strText);
    } else {
        QLabel::setText(m_strText);
        setToolTip("");
    }
    QLabel::paintEvent(event);
}
