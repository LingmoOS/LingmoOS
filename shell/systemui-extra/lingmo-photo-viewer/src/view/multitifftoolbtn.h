#ifndef MULTITIFFTOOLBTN_H
#define MULTITIFFTOOLBTN_H

#include <QObject>
#include <QWidget>
#include <kpushbutton.h>
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
class MultiTiffToolBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit MultiTiffToolBtn(QString btnType, QWidget *parent = nullptr);
    void themeStyle(QString theme);

private:
    void paintEvent(QPaintEvent *event);
    QString m_theme = "";
    QString m_btnType = "";

Q_SIGNALS:
};

#endif // MULTITIFFTOOLBTN_H
