// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMGVIEWLISTVIEW_H
#define IMGVIEWLISTVIEW_H

#include "imgviewdelegate.h"
#include "dtkwidget_global.h"

#include <QPropertyAnimation>
#include <QPixmap>
#include <QIcon>
#include <QFileInfo>
#include <QSize>
#include <QStandardItemModel>
#include <QBuffer>
#include <QMouseEvent>
#include <QPointer>

#include <DListView>
#include <DLabel>
#include <DMenu>
#include <DPushButton>
#include <DIconButton>
#include <DApplicationHelper>

#include "image-viewer_global.h"

DWIDGET_USE_NAMESPACE

class LibImgViewListView : public DListView
{
    Q_OBJECT

public:
    explicit LibImgViewListView(QWidget *parent = nullptr);
    ~LibImgViewListView() override;

    void setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path);//设置需要展示的所有缩略图

    int getSelectIndexByPath(QString path);
    //将选中的项居中
    void setSelectCenter();
    //查看下一张
    void openNext();
    //查看前一张
    void openPre();
    //移除当前选中
    void removeCurrent();

    //设置当前图片
    void setCurrentPath(const QString &path);

    //获取所有路径
    QStringList getAllPath();

    //获得当前item的x位置
    int getCurrentItemX();

    //获得当前item的x位置
    int getRowWidth();

//protected:
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
public slots:
    //有新的图片加载上来
    void slotOneImgReady(QString path, imageViewerSpace::ItemInfo pix);
    //当前图片刷新
    void slotCurrentImgFlush(QPixmap pix, const QSize &originalSize);
    //列表点击事件
    void onClicked(const QModelIndex &index);
private:
    void cutPixmap(imageViewerSpace::ItemInfo &iteminfo);
    //加载后50张图片
    void loadFiftyRight();
    //当点击的是最后一个时，向前移动动画
    void startMoveToLeftAnimation();
    //根据行号获取路径path
    const QString getPathByRow(int row);
signals:
    void openImg(int index, QString path);
public:
    const static int ITEM_NORMAL_WIDTH = 30;//非选中状态宽度
    const static int ITEM_NORMAL_HEIGHT = 80;//非选中状态高度
    const static int ITEM_CURRENT_WH = 60;//当前选中状态宽高
    const static int ITEM_SPACING = 2;//间隔
public:
    LibImgViewDelegate *m_delegate = nullptr;
    QStandardItemModel *m_model = nullptr;
    QStringList m_allFileList;//需要展示的全部缩略图路径

    QPropertyAnimation *m_moveAnimation = nullptr;//移动动画

    int m_currentRow = -1;//当前展示项
    int m_pre = -1;
    QString m_currentPath;
};

#endif // THUMBNAILLISTVIEW_H
