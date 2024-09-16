// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "attributemanager.h"
#include "csidewidthwidget.h"
#include "seperatorline.h"
#include "globaldefine.h"
#ifdef USE_DTK
#include <DFontSizeManager>
#endif

#include <QHBoxLayout>
#include <QFontComboBox>
#include <QLineEdit>
#include <QGraphicsDropShadowEffect>
#include <QTimer>


class AttributionManager::AttributionManager_private
{
public:
    explicit AttributionManager_private(AttributionManager *qq): q(qq) {}

    void showAttributionHelper(const SAttrisList &attribution, int showFlags, const QPoint &pos)
    {
        if (_sAttributions != attribution) {
            if ((acceptedShowFlag & showFlags) || (showFlags & ForceShow)) {
                _sAttributions = attribution;
                _dirty = 1;
                delayShow(showFlags, pos);
            }
        } else {

            if ((acceptedShowFlag & showFlags) || (showFlags & ForceShow))
                showHelper(showFlags, pos);
        }
    }
    void showHelper(int flag, const QPoint &pos)
    {
        cachedShowFlag = flag;
        cachedPos = pos;
        QMetaObject::invokeMethod(q->helper(), [ = ]() {
            if (!exposeWidgets.isEmpty()) {
                q->showAt(cachedShowFlag, cachedPos);
            }
        }, Qt::QueuedConnection);
    }
    void delayShow(int flag, const QPoint &pos)
    {
        if (delayTimer == nullptr) {
            delayTimer = new QTimer(q->helper());
            delayTimer->setSingleShot(true);
            QObject::connect(delayTimer, &QTimer::timeout, _helper, [ = ]() {
                QList<QWidget *> newWantedWgts;
                foreach (auto key, _sAttributions) {
                    auto itF = s_allInstalledAttriWgts.find(key.attri);
                    if (itF != s_allInstalledAttriWgts.end()) {
                        auto w = itF.value();
                        newWantedWgts.append(w);
                        q->setWidgetAttribution(w, key.var);
                    }
                }
                //属性刷新就重新刷新显示
                auto oldWidgets = exposeWidgets;
                if (oldWidgets != newWantedWgts) {
                    exposeWidgets = newWantedWgts;
                    q->showWidgets(cachedShowFlag, oldWidgets, exposeWidgets);
                    showHelper(cachedShowFlag, cachedPos);
                } else {
                    showHelper(cachedShowFlag, cachedPos);
                }
            });
        }
        cachedShowFlag = flag;
        cachedPos = pos;
        delayTimer->start(70);
    }



    AttributionManager *q;
    SAttrisList          _sAttributions;
    bool                 _dirty = false;

    QMap<int, QWidget *> s_allInstalledAttriWgts;
    QMap<int, QVariant>  s_allInstalledDefaultVar;

    QList<QWidget *> exposeWidgets;
    AttributionManagerHelper *_helper = nullptr;

    QTimer *delayTimer = nullptr;
    int    cachedShowFlag = 0;
    QPoint cachedPos;

    int  acceptedShowFlag = ToolActivedAndClicked | ItemStyleEditMenu;
    QWidget *widget = nullptr;
};
AttributionManager::AttributionManager(): AttributionManager_d(new AttributionManager_private(this))
{
    d_AttributionManager()->_helper = new AttributionManagerHelper(this);
}

AttributionManagerHelper *AttributionManager::helper() const
{
    return d_AttributionManager()->_helper;
}

SAttrisList AttributionManager::attributions() const
{
    return d_AttributionManager()->_sAttributions;
}

//QList<QWidget *> AttributionManager::allInstallAttriWidgets() const
//{
//    QList<QWidget *> results;
//    foreach (auto w, d_AttributionManager()->s_allInstalledAttriWgts) {
//        results.append(w);
//    }
//    return results;
//}

void AttributionManager::showAttributions(const SAttrisList &attribution, int showFlags, const QPoint &pos)
{
    d_AttributionManager()->showAttributionHelper(attribution, showFlags, pos);
}

int AttributionManager::showFlags() const
{
    return d_AttributionManager()->acceptedShowFlag;
}

void AttributionManager::setShowFlags(int flag)
{
    d_AttributionManager()->acceptedShowFlag = flag;
}

void AttributionManager::changeAttribution(const SAttri &attri, bool update)
{
    for (auto &p : d_AttributionManager()->_sAttributions) {
        if (p.attri == attri.attri) {
            p.var = attri.var;
            if (update) {
                d_AttributionManager()->_dirty = 1;
                d_AttributionManager()->delayShow(NoDisplay, QPoint());
            }
            break;
        }
    }
}

void AttributionManager::showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets)
{
    auto displayW = displayWidget();
    if (displayW != nullptr) {
        foreach (auto w, oldWidgets) {
            w->setParent(nullptr);
            w->hide();
        }
        foreach (auto w, exposeWidgets) {
            if (w->parent() != displayW) {
                w->setParent(displayW);
                if (displayW != nullptr)
                    displayW->layout()->addWidget(w);
                w->show();
            }
        }
        displayW->updateGeometry();
        displayW->adjustSize();
    }
}

bool isAAncestorOfB(const QWidget *a, const QWidget *b)
{
    while (b) {
        if (b == a)
            return true;
//        if (b->isWindow())
//            return false;
        b = b->parentWidget();
    }
    return false;
}
bool AttributionManager::isFriendWidget(QWidget *w) const
{
    bool result = false;
    foreach (auto wIt, d_AttributionManager()->s_allInstalledAttriWgts) {
        bool isFriend = (isAAncestorOfB(wIt, w) || wIt == w);
        if (isFriend) {
            result = true;
            break;
        }
    }
    return result;
}


void AttributionManager::installComAttributeWgt(int attri,
                                                QWidget *pWgt,
                                                const QVariant &defaultVar)
{
    if (pWgt != nullptr) {
        auto w = qobject_cast<AttributeWgt *>(pWgt);
        if (w != nullptr) {
            w->setAttribution(attri);
            QObject::connect(w, &AttributeWgt::attriChanged, helper(), &AttributionManagerHelper::onAttriWidgetValueChanged);
        }
        d_AttributionManager()->s_allInstalledAttriWgts.insert(attri, pWgt);
        pWgt->hide();
    }

    setDefaultAttributionVar(attri, defaultVar);
}

void AttributionManager::removeComAttributeWgt(int attri)
{
    auto itf = d_AttributionManager()->s_allInstalledAttriWgts.find(attri);
    if (itf != d_AttributionManager()->s_allInstalledAttriWgts.end()) {
        auto w = qobject_cast<AttributeWgt *>(itf.value());
        if (w != nullptr) {
            QObject::disconnect(w, &AttributeWgt::attriChanged, helper(), &AttributionManagerHelper::onAttriWidgetValueChanged);
        }
        d_AttributionManager()->s_allInstalledAttriWgts.erase(itf);
    }

    auto itfV = d_AttributionManager()->s_allInstalledDefaultVar.find(attri);
    if (itfV != d_AttributionManager()->s_allInstalledDefaultVar.end()) {
        d_AttributionManager()->s_allInstalledAttriWgts.erase(itf);
    }
}

void AttributionManager::setDefaultAttributionVar(int attri, const QVariant &defaultVar)
{
    d_AttributionManager()->s_allInstalledDefaultVar.insert(attri, defaultVar);
}

SAttri AttributionManager::attrOfWidget(QWidget *pWgt)
{
    for (auto it = d_AttributionManager()->s_allInstalledAttriWgts.begin(); it != d_AttributionManager()->s_allInstalledAttriWgts.end(); ++it) {
        if (pWgt == it.value()) {
            QVariant var = pWgt->property(AttriWidgetVar);
            return SAttri(it.key(), var);
        }
    }
    return SAttri();
}

//QWidget *AttributionManager::widgetOfAttr(int attri)
//{
//    auto it = d_AttributionManager()->s_allInstalledAttriWgts.find(attri);
//    if (it == d_AttributionManager()->s_allInstalledAttriWgts.end())
//        return nullptr;
//    return it.value();
//}

QVariant AttributionManager::defaultAttriVar(int attri)
{
    return d_AttributionManager()->s_allInstalledDefaultVar[attri];
}

void AttributionManager::setWidgetAttribution(QWidget *pWgt, const QVariant &var)
{
    auto pw = qobject_cast<AttributeWgt *>(pWgt);
    if (pw != nullptr) {
        pw->setVar(var);
    } else {
        pWgt->setProperty(AttriWidgetVar, var);
    }

    pWgt->blockSignals(true);
    emit helper()->updateWgt(pWgt, var);
    pWgt->blockSignals(false);
}

void AttributionManager::setDisplayWidget(QWidget *widget)
{
    d_AttributionManager()->widget = widget;
}

QWidget *AttributionManager::displayWidget() const
{
    return d_AttributionManager()->widget;
}

void AttributionManager::showAt(int active, const QPoint &pos)
{
    if (displayWidget() != nullptr) {
        displayWidget()->move(pos);
        displayWidget()->show();
    }
}

AttributionManagerHelper::AttributionManagerHelper(AttributionManager *pManager): _manager(pManager)
{
    //更新当前的属性值
    connect(this, &AttributionManagerHelper::attributionChanged, this, [ = ](int attris, const QVariant & var) {
        _manager->changeAttribution(SAttri(attris, var), false);
    });
}

void AttributionManagerHelper::onAttriWidgetValueChanged(const QVariant var, int phase)
{
    auto attriWidget = qobject_cast<AttributeWgt *>(sender());
    if (attriWidget != nullptr && attriWidget->attribution() >= 0) {
        emit attributionChanged(attriWidget->attribution(), var, phase);
    }
}

AttributionManagerWgt::AttributionManagerWgt(QWidget *parent): QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    setLayout(lay);
}

void AttributionManagerWgt::showAt(int active, const QPoint &pos)
{
    this->move(pos);
    show();
}

void AttributionManagerWgt::showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets)
{
    setVisible(!exposeWidgets.isEmpty());

    foreach (auto w, oldWidgets) {
        w->setParent(nullptr);
        w->hide();
    }
    foreach (auto w, exposeWidgets) {
        w->setParent(this);
        layout()->addWidget(w);
        w->show();
    }
}
