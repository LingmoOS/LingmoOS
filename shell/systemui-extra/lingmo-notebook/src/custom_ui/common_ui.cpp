#include "common_ui.h"
#include <QDebug>
#include <QWidget>

QString dealMessage(const QString msg)
{
    int fontSize = 16;
    if(msg.size() > fontSize)
    {
        QString str;
        int time = msg.size()/fontSize;
        for(int i = 0; i <= time-1; i++)
        {
            str = QString(str + msg.mid(i*fontSize,fontSize)+"\r\n");
        }
        str = QString(str+msg.mid(time*fontSize,fontSize));
        // qDebug() << "Str" <<str << "   time" << time;
        return str;
    }
    else
        return msg;
}

void centerTheWindow(QWidget *self, QWidget *parentWindow)
{
    if (self == nullptr || parentWindow == nullptr) {
        return ;
    }
    QPoint temp = QPoint(parentWindow->mapToGlobal(QPoint(0,0)).x() + parentWindow->width()/2,parentWindow->mapToGlobal(QPoint(0,0)).y() + parentWindow->height()/2);
    self->move(temp - self->rect().center());
}

#ifdef TEST_DEBUG
void showWindowFrame(QWidget *self)
{
    self->setStyleSheet(QString::fromUtf8("border:3px solid blue"));
    return;
}
#endif