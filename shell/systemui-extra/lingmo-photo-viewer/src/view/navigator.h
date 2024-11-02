#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QDebug>
#include "controller/interaction.h"

class Navigator : public QWidget
{
    Q_OBJECT
public:
    explicit Navigator(QWidget *parent = nullptr);
    void showNavigation(QPixmap pix);

private:
    QLabel *m_bottomImage;
    bool m_isPress = false;
    QPoint m_startPos = QPoint(-1, -1);
    QPoint m_endPos = QPoint(-1, -1);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void getHighLightRegion(QPoint startPos, QPoint endPos); //拿起始点和终点



Q_SIGNALS:
    void naviChange();
    void posChange(QPointF curMousePos, bool needChangeDisplayImage);
    void navigatorState(bool navigatorState);
    void notifyNotUpdated(bool);
    void needUpdateImagePosFromMainWid(bool);
    void needUpdateQmlImagePos(bool needRecord); //鼠标释放，需要更新qml界面图片的显示位置
};

#endif // NAVIGATOR_H
