#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QGridLayout>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItem>
#include <QDebug>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QMovie>
#include <QScrollBar>
#include "controller/interaction.h"
#include "sizedate.h"
#include "sidebardelegate.h"

class SideBar : public QListView
{
    Q_OBJECT
public:
    explicit SideBar(QWidget *parent = nullptr);
    void showItem();
    void setItemNum(double acturalHeight);

public Q_SLOTS:
    void getSelect(int type);
    void openEmptyFile(bool changeHigh); //判断是不是打开了图片，之后设置相册高亮

private:
    QMovie *m_loadingMovie = nullptr; //没有处理完，则显示此动图
    SideBarDelegate *m_delegate = nullptr;
    QModelIndex m_modelIndexOld; //记录上一次的index
    int m_nextMove = 0;          //记录滑动条移动位置-下一次
    int m_backMove = 0;          //记录滑动条移动位置-上一次
    int m_imageNum = 0;


    void initConnect();

protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);

private Q_SLOTS:

    void setCurrIndex(QModelIndex modelIndex);
    void dealScrollbarMove(int moveValue);

Q_SIGNALS:

    void changeImage(bool isChange); //解决从相册切换时动图没有刷新右键菜单内容
    void sizeChange();               //相册尺寸改变，位置改变
};

#endif // SIDEBAR_H
