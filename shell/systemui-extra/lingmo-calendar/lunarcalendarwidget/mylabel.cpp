#include "mylabel.h"
#include <QtWidgets>

MyLabel::MyLabel(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
}

MyLabel::MyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f) : QLabel(text, parent, f), m_fullText(text)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
}

void MyLabel::setText(const QString &text)
{
    setFullText(text);
}

void MyLabel::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void MyLabel::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void MyLabel::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

//全部的字符串
QString MyLabel::fullText() const
{
    return m_fullText;
}

void MyLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    elideText();
}

void MyLabel::elideText()
{
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) - this->width();
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width());
        QLabel::setText(showText);
        if (showText != m_fullText) {
            QString str = dealMessage(m_fullText);
            this->setToolTip(str);
        } else {
            this->setToolTip("");
        }
    } else {
        QLabel::setText(m_fullText);
        this->setToolTip("");
    }
}

QString MyLabel::dealMessage(QString msg)
{
    if (msg.size() > m_fontSize) {
        QString str;
        int time = msg.size() / m_fontSize;
        for (int i = 0; i <= time - 1; i++) {
            str = QString(str + msg.mid(i * m_fontSize, m_fontSize) + "\r\n");
        }
        str = QString(str + msg.mid(time * m_fontSize, m_fontSize));
        //        qDebug() << "Str" <<str << "   time" << time;
        return str;
    } else
        return msg;
}
