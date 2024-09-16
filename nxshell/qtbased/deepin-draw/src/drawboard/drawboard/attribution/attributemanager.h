// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATTRIBUTEMANAGERWGT_H
#define CATTRIBUTEMANAGERWGT_H
#include "attributewidget.h"


#ifdef USE_DTK
#include <DIconButton>
#include <DBlurEffectWidget>
#endif


#include <QWidget>
#include <QVariant>

#include <QWidget>
#include <QMap>
#include <QLabel>
class QSpacerItem;
#ifdef USE_DTK
DWIDGET_USE_NAMESPACE
#endif


class AttributionManager;
class DRAWLIB_EXPORT AttributionManagerHelper: public QObject
{
    Q_OBJECT
public:
    AttributionManagerHelper(AttributionManager *pManager = nullptr);

    Q_SLOT void  onAttriWidgetValueChanged(const QVariant var, int phase);

signals:
    // 属性栏属性变更时(界面控制)触发，更新内部属性
    void attributionChanged(int attris, const QVariant &var, int phase = EChanged, bool autoCmdStack = true);
    void updateWgt(QWidget *pWgt, const QVariant &var);

    // 通过内部更新的属性信息(界面拖拽等)，广播到外部(属性栏等)
    void internalAttibutionUpdate(int attris, const QVariant &var, int phase = EChanged);

private:
    AttributionManager *_manager = nullptr;
};

class DRAWLIB_EXPORT AttributionManager
{
public:
    enum ShowFlags {NoDisplay = 0x00,

                    ToolActived           = 0x01,
                    ToolActivedAndClicked = ToolActived << 1,
                    CurrentPageChanged    = ToolActived << 2,
                    ItemSelected          = ToolActived << 3,
                    ItemAttriChanged      = ToolActived << 4,
                    ItemStyleEditMenu     = ToolActived << 5,

                    ShowAtLeft   = ToolActived << 6,
                    ShowAtTop    = ToolActived << 7,
                    ShowAtRight  = ToolActived << 8,
                    ShowAtBottom = ToolActived << 9,

                    ForceShow = ToolActived << 13
                   };

    AttributionManager();

    AttributionManagerHelper *helper() const;

    SAttrisList attributions() const;
    void        showAttributions(const SAttrisList &attribution,
                                 int showFlags = ForceShow,
                                 const QPoint &pos = QPoint());
    int         showFlags()const;
    void        setShowFlags(int flag);

    void        changeAttribution(const SAttri &attri, bool update = true);

    //QList<QWidget *> allInstallAttriWidgets() const;

    void installComAttributeWgt(int attri, QWidget *pWgt, const QVariant &defaultVar = QVariant());
    void removeComAttributeWgt(int attri);

    void    setDefaultAttributionVar(int attri, const QVariant &defaultVar);
    SAttri  attrOfWidget(QWidget *pWgt);
    //QWidget *widgetOfAttr(int attri);

    QVariant defaultAttriVar(int attri);
    void     setWidgetAttribution(QWidget *pWgt, const QVariant &var);

    void setDisplayWidget(QWidget *widget);
    QWidget *displayWidget() const;


    virtual void showAt(int active, const QPoint &pos);
    virtual void showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets);
    virtual bool isFriendWidget(QWidget *w)const;


    PRIVATECLASS(AttributionManager)
};

class DRAWLIB_EXPORT AttributionManagerWgt: public QWidget, public AttributionManager
{
    Q_OBJECT
public:
    AttributionManagerWgt(QWidget *parent = nullptr);

    void showAt(int active, const QPoint &pos) override;

    void showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets) override;
};


#endif // CATTRIBUTEMANAGERWGT_H
