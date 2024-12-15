// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATTRIBUTEMANAGERWGT_H
#define CATTRIBUTEMANAGERWGT_H
#include "cattributeitemwidget.h"


#include <DIconButton>
#include <DBlurEffectWidget>


#include <QWidget>
#include <QVariant>

#include <QWidget>
#include <QMap>
#include <QLabel>
class QSpacerItem;
DWIDGET_USE_NAMESPACE

using namespace DrawAttribution;

namespace DrawAttribution {
class CAttributeManagerWgt: public CAttriBaseOverallWgt
{
    Q_OBJECT
public:
    explicit CAttributeManagerWgt(QWidget *parent = nullptr);

    SAttrisList      attributions() const;
    void             setAttributions(const SAttrisList &attribution);
    void             changeAttribution(const SAttri &attri, bool update = true);

    QList<QWidget *> attributeWgts();

    QList<QVariant>  attributeVars();

    bool            isLogicAncestorOf(QWidget *w);

    void installComAttributeWgt(int attri, QWidget *pWgt, const QVariant &defaultVar = QVariant());
    void removeComAttributeWgt(int attri);

    void setDefaultAttributionVar(int attri, const QVariant &defaultVar);
    int  attrOfWidget(QWidget *pWgt);
    QWidget *widgetOfAttr(int attri);


    QVariant defaultAttriVar(int attri);


    void setWidgetRecommedSize(QWidget *pWgt, const QSize &sz);

signals:
    void attributionChanged(int attris, const QVariant &var, int phase = EChanged, bool autoCmdStack = true);
    void updateWgt(QWidget *pWgt, const QVariant &var);

private:
    void     ensureAttributions();
    QSize    attriWidgetRecommendedSize(QWidget *pWgt) override;
    void     setWidgetAttribution(QWidget *pWgt, const QVariant &var);
    void     onAttriWidgetValueChanged(const QVariant var, int phase);

protected:
    void paintEvent(QPaintEvent *event) override;

private:

    SAttrisList            _sAttributions;

    QMap<int, QWidget *> s_allInstalledAttriWgts;
    QMap<int, QVariant>  s_allInstalledDefaultVar;

    bool _dirty = false;
};
}

#endif // CATTRIBUTEMANAGERWGT_H
