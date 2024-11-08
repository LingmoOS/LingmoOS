#ifndef PUSHBUTTONICON_H
#define PUSHBUTTONICON_H

#include <QObject>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "mylabelicon.h"
namespace KInstaller {
//class PushButtonIcon : public QWidget
//{
//    Q_OBJECT
//public:
//    PushButtonIcon(QString icon, QString txt, QWidget* parent = nullptr);

//    void showIconAndText();
//    void showIcon();
//signals:
//    void clicked();
//protected:
//    bool eventFilter(QObject *, QEvent *);//事件过滤器处理函数
//public:
//    QLabel* m_txtLabel;
//    QLabel* m_IcomLabel;
//    QString m_icon;
//    QString m_txt;

//};
class PushButtonIcon : public QWidget
{
    Q_OBJECT
public:
    PushButtonIcon(QPixmap icon, QString txt, QWidget* parent = nullptr);

    void showIconAndText();
    void showIcon();


    QIcon m_icon;
    QString m_txt;
    QHBoxLayout* hlay;
    mylabelicon* licon;
    QLabel* ltxt;

signals:
    void clicked();

private:
//    void mousePressEvent(QMouseEvent* event);
    bool eventFilter(QObject *, QEvent *);//事件过滤器处理函数
   void paintEvent(QPaintEvent *ev);
   QColor Shadow=QColor(0, 0, 0, 0);
   QPainter pain;

};
}
#endif // PUSHBUTTONICON_H
