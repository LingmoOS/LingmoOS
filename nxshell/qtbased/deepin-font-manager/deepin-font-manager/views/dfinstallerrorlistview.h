// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFINSTALLERRORLISTVIEW_H
#define DFINSTALLERRORLISTVIEW_H

#include "dfinstallerroritemmodel.h"
#include "dfontinfodialog.h"

#include <DListView>

#include <QPainterPath>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class DFInstallErrorListView;

/*************************************************************************
 <Class>         DFInstallErrorListDelegate
 <Description>   DFInstallErrorListViewd的绘图代理类，
 <Author>
 <Note>          null
*************************************************************************/
class DFInstallErrorListDelegate : public DStyledItemDelegate
{
public:
    explicit DFInstallErrorListDelegate(QAbstractItemView *parent = nullptr);
    //代理绘制字体列表入口
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    //获得当前QModelIndex对象size
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    //代理绘制勾选按钮函数
    void drawCheckBox(QPainter *painter, DFInstallErrorItemModel itemModel, QRect bgRect)const;
    //代理绘制勾选按钮图标函数
    void drawCheckBoxIcon(QPainter *painter, QRect bgRect)const;
    //代理绘制字体名
    void drawFontName(QPainter *painter, const QStyleOptionViewItem &option,
                      DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable = true)const;
    //代理绘制字体样式
    void drawFontStyle(QPainter *painter, const QStyleOptionViewItem &option,
                       DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable = true)const;
    //代理绘制勾选状态
    void drawSelectStatus(QPainter *painter, const QStyleOptionViewItem &option, QRect bgRect)const;
    //代理绘制tab聚焦状态背景颜色
    void paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect)const;
    //listview中字体名长度自适应处理
    QString lengthAutoFeed(QPainter *painter, QString sourceStr, int m_StatusWidth)const;
    //获取需要绘制区域的路径
    void setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius)const;

private:
    DFInstallErrorListView *m_parentView;
};

class QStandardItemModel;
/*************************************************************************
 <Class>         DFInstallErrorListView
 <Description>   构造函数-构造字体列表视图实例对象
 <Author>
 <Note>          null
*************************************************************************/
class DFInstallErrorListView : public DListView
{
    Q_OBJECT

public:
    explicit DFInstallErrorListView(const QList<DFInstallErrorItemModel> &installErrorFontModelList,
                                    QWidget *parent = nullptr);
    //析构函数-析构字体列表视图实例对象
    ~DFInstallErrorListView() override;
    //重写鼠标press事件
    void mousePressEvent(QMouseEvent *event) override;
    //重写设置选中函数
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    //重写事件过滤器
    bool eventFilter(QObject *obj, QEvent *event)override;
    //初始化异常字体列表信息
    void initErrorListData();
    //初始化绘图代理-实例化代理对象
    void initDelegate();
    //新增异常字体信息至列表
    void addErrorListData(const QList<DFInstallErrorItemModel> &installErrorFontModelList);
    //检查是否需要滚动至指定QModelIndex
    void checkScrollToIndex(QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles, QStringList &errorFileList);
    //滚动至指定QModelIndex
    void scrollToIndex(QString &filePath);
    //设置选中状态
    void setSelectStatus(QStringList &HalfInstalledFiles, QModelIndexList &beforeSelectFiles);
    //更新DFInstallErrorItemModel信息
    void updateErrorFontModelList(int index, DFInstallErrorItemModel m_currentItemModel);
    //初始化ItemModel对象
    void initModel(bool newOne = true);
    //获取当前QStandardItemModel实例对象
    QStandardItemModel *getErrorListSourceModel();
    //获取当前是否为tab聚焦状态
    bool getIsTabFocus() const;
    //更新tab聚焦状态
    void setIsInstallFocus(bool isInstallFocus);
    //查看是否为列表新增获取的tab聚焦
    bool getIsInstallFocus() const;
    //不可见则滚动到modelindex
    void ifNeedScrollTo(QModelIndex idx);
    //响应home和end快捷按键-fix bug 43109-便于父对象调用，整理成函数
    void responseToHomeAndEnd(bool isHomeKeyPressed);
    //响应PageUp和PageDown快捷按键-便于父对象调用，整理成函数
    void responseToPageUpAndPageDown(bool isPageUpPressed);
private:

    DFontInfoManager *m_fontInfoManager;
    DFInstallErrorListDelegate *m_errorListItemDelegate;
    QStandardItemModel *m_errorListSourceModel;
    QList<DFInstallErrorItemModel> m_installErrorFontModelList;
    QList<QString> m_errorFontlist;
    int beforeSelectRow = 0;
    bool m_IsTabFocus = false;
    bool m_ifFirstFocus = true;
    //初始选中第一个可选项
    void initSelectedItem();
    //获取图标范围
    inline QRect getIconRect(const QRect &rect);
signals:
    //发送item点击信号
    void clickedErrorListItem(const QModelIndex &index);

private slots:
protected:
    //重写键盘press事件-安装验证页面，listview上下键自动跳过异常字体
    void keyPressEvent(QKeyEvent *event)Q_DECL_OVERRIDE;//SP3--安装验证页面，listview上下键自动跳过异常字体
    //判断下个选中
    bool selectNextIndex(int nextIndex);//SP3--安装验证页面，listview上下键自动跳过异常字体
};

#endif // DFINSTALLERRORLISTVIEW_H
