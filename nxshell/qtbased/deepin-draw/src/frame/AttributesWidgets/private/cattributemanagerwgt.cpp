// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cattributemanagerwgt.h"
#include "csidewidthwidget.h"
#include "seperatorline.h"
#include "globaldefine.h"
#include "ccolorpickwidget.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QFontComboBox>
#include <QLineEdit>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

CExpButton::CExpButton(QWidget *parent): QWidget(parent)
{
    _expBtn  = new DIconButton(this);
    _expBtn->setIcon(QIcon::fromTheme("icon_open_normal"));
    _expBtn->setFixedSize(36, 36);
    _expBtn->setIconSize(QSize(30, 30));
    //_sepLine = new SeperatorLine;

    QHBoxLayout *lay = new QHBoxLayout;
    //lay->addWidget(_sepLine);
    lay->addWidget(_expBtn);
    this->setLayout(lay);
    lay->setContentsMargins(0, 0, 0, 0);

    connect(_expBtn, &DIconButton::clicked, this, &CExpButton::clicked);
}

CExpWgt::CExpWgt(QWidget *parent): DBlurEffectWidget(parent)
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setWindowFlags(this->windowFlags() | Qt::Popup | Qt::FramelessWindowHint);
    //设置圆角
    this->setBlurRectXRadius(18);
    this->setBlurRectYRadius(18);
    this->setBlurEnabled(true);
    this->setMode(DBlurEffectWidget::GaussianBlur);
    this->setMaskAlpha(255);
    //绘制背景阴影
//    this->setAttribute(Qt::WA_TranslucentBackground);
//    const int nMargin = 10;     // 设置阴影宽度
//    auto shadow_effect = new QGraphicsDropShadowEffect(this);
//    shadow_effect->setOffset(0, 0);
//    shadow_effect->setBlurRadius(nMargin);
//    this->setGraphicsEffect(shadow_effect); //最外层的Frame

    QVBoxLayout *pVLay = new QVBoxLayout;
    this->setLayout(pVLay);
    pVLay->setContentsMargins(0, 12, 0, 12);

    _pCenterLay = new QVBoxLayout;
    pVLay->addStretch();
    pVLay->addLayout(_pCenterLay);
    pVLay->addStretch();
    _pCenterLay->setContentsMargins(0, 0, 0, 0);
}

class DQSpacerItem: public QSpacerItem
{
public:
    using QSpacerItem::QSpacerItem;
    QSize sizeHint() const override {return QSize(0, 0);}
    //QSize minimumSize() const override {return QSize(0, 0);}
    //QSize maximumSize() const override {return QSize(0, 0);}
};
void CExpWgt::addWidget(QWidget *pWidget)
{
    if (!widgetShowInWindow(pWidget)) {
        pWidget->hide();
        return;
    }

    if (!_widgets.isEmpty()) {
        auto line = new CSpline(this);
        _pCenterLay->addWidget(line);
        _splines.append(line);
    }

    auto lay = packageWidget(pWidget, widgetMarginInWindow(pWidget), widgetAlignInWindow(pWidget));
    _pCenterLay->addItem(lay);
    _widgets.append(pWidget);
}

//void CExpWgt::setWidgets(const QList<QWidget *> &widgets)
//{
//    if (_widgets == widgets)
//        return;

//    clear();

//    foreach (auto w, widgets) {
//        addWidget(w);
//    }
//}

void CExpWgt::clear()
{
    clearChildLayout();
    _widgets.clear();
}

void CExpWgt::showEvent(QShowEvent *event)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6, QFont::Normal);
    DBlurEffectWidget::showEvent(event);
}

void CExpWgt::paintEvent(QPaintEvent *event)
{
    //qWarning() << "_sp line = " << _splines;
    DBlurEffectWidget::paintEvent(event);
}

bool CExpWgt::widgetShowInWindow(const QWidget *w)
{
    auto show = w->property(WidgetShowInVerWindow);
    if (show.isValid())
        return show.toBool();
    return true;
}

QMargins CExpWgt::widgetMarginInWindow(const QWidget *w)
{
    auto margins = w->property(WidgetMarginInVerWindow);
    if (margins.isValid())
        return margins.value<QMargins>();
    return QMargins(20, 0, 20, 0);
}

Qt::Alignment CExpWgt::widgetAlignInWindow(const QWidget *w)
{
    auto margins = w->property(WidgetAlignInVerWindow);
    if (margins.isValid())
        return Qt::Alignment(margins.value<int>());
    return Qt::AlignLeft;
}

void CExpWgt::clearChildLayout()
{
    while (_pCenterLay->count() > 0) {
        QLayoutItem *pItem = _pCenterLay->takeAt(0);
        if (nullptr != pItem) {
            delete pItem;
        }
    }

    foreach (auto p, _splines) {
        p->deleteLater();
    }
    _splines.clear();
}

QHBoxLayout *CExpWgt::packageWidget(QWidget *pWidget, const QMargins &margins, Qt::Alignment align)
{
    pWidget->setParent(this);
    QHBoxLayout *layTemp = new QHBoxLayout;

    layTemp->setContentsMargins(margins.left(), margins.top(), margins.right(), margins.bottom());
    layTemp->addWidget(pWidget, 0, align);

    return layTemp;
}

CAttributeManagerWgt::CAttributeManagerWgt(QWidget *parent): CAttriBaseOverallWgt(parent)
{
    //更新当前的属性值
    connect(this, &CAttributeManagerWgt::attributionChanged, this, [ = ](int attris, const QVariant & var) {
        this->changeAttribution(SAttri(attris, var), false);
    });
    setFocusPolicy(Qt::StrongFocus);
}

SAttrisList CAttributeManagerWgt::attributions() const
{
    return _sAttributions;
}

void CAttributeManagerWgt::setAttributions(const SAttrisList &attribution)
{
    if (_sAttributions != attribution) {
        //qWarning() << "_sAttributions = " << _sAttributions << "attribution = " << attribution;

        _sAttributions = attribution;
        _dirty = 1;
        //ensureAttributions();
        if (this->isHidden())
            this->show();
        else
            update();
    }
}

void CAttributeManagerWgt::changeAttribution(const SAttri &attri, bool update)
{
    //qFind(_sAttributions.begin())
    for (auto &p : _sAttributions) {
        if (p.attri == attri.attri) {
            p.var = attri.var;
            if (update) {
                _dirty = 1;
                this->update();
            }
            break;
        }
    }
}

QList<QWidget *> CAttributeManagerWgt::attributeWgts()
{
    ensureAttributions();
    return _allWgts;
}

QList<QVariant> CAttributeManagerWgt::attributeVars()
{
    ensureAttributions();
    return QList<QVariant>();
}

bool CAttributeManagerWgt::isLogicAncestorOf(QWidget *w)
{
    auto tempW = w;
    while (tempW != nullptr) {
        if (tempW == this) {
            return true;
        }
        auto picker = qobject_cast<CColorPickWidget *>(tempW);
        if (picker != nullptr &&
                picker->caller()->parentWidget() == this) {
            return true;
        }
        tempW = tempW->parentWidget();
    }
    return false;
}

void CAttributeManagerWgt::installComAttributeWgt(int attri,
                                                  QWidget *pWgt,
                                                  const QVariant &defaultVar)
{
    if (pWgt != nullptr) {
        auto w = qobject_cast<CAttributeWgt *>(pWgt);
        if (w != nullptr) {
            w->setAttribution(attri);
            connect(w, &CAttributeWgt::attriChanged, this, &CAttributeManagerWgt::onAttriWidgetValueChanged);
        }
        s_allInstalledAttriWgts.insert(attri, pWgt);
    }

    setDefaultAttributionVar(attri, defaultVar);
}

void CAttributeManagerWgt::removeComAttributeWgt(int attri)
{
    auto itf = s_allInstalledAttriWgts.find(attri);
    if (itf != s_allInstalledAttriWgts.end()) {
        auto w = qobject_cast<CAttributeWgt *>(itf.value());
        if (w != nullptr) {
            disconnect(w, &CAttributeWgt::attriChanged, this, &CAttributeManagerWgt::onAttriWidgetValueChanged);
        }
        s_allInstalledAttriWgts.erase(itf);
    }

    auto itfV = s_allInstalledDefaultVar.find(attri);
    if (itfV != s_allInstalledDefaultVar.end()) {
        s_allInstalledAttriWgts.erase(itf);
    }
}

void CAttributeManagerWgt::setDefaultAttributionVar(int attri, const QVariant &defaultVar)
{
    s_allInstalledDefaultVar.insert(attri, defaultVar);
}

int CAttributeManagerWgt::attrOfWidget(QWidget *pWgt)
{
    for (auto it = s_allInstalledAttriWgts.begin(); it != s_allInstalledAttriWgts.end(); ++it) {
        if (pWgt == it.value()) {
            return it.key();
        }
    }
    return -1;
}

QWidget *CAttributeManagerWgt::widgetOfAttr(int attri)
{
    auto it = s_allInstalledAttriWgts.find(attri);
    if (it == s_allInstalledAttriWgts.end())
        return nullptr;
    return it.value();
}

QVariant CAttributeManagerWgt::defaultAttriVar(int attri)
{
    return s_allInstalledDefaultVar[attri];
}

void CAttributeManagerWgt::setWidgetRecommedSize(QWidget *pWgt, const QSize &sz)
{
    if (pWgt == nullptr)
        return;

    pWgt->setProperty(AttriWidgetReWidth, sz);
}

void CAttributeManagerWgt::ensureAttributions()
{
    if (_dirty) {
        QList<QWidget *> newWantedWgts;
        foreach (auto key, _sAttributions) {
            auto itF = s_allInstalledAttriWgts.find(key.attri);
            if (itF != s_allInstalledAttriWgts.end()) {
                auto w = itF.value();
                newWantedWgts.append(w);
                setWidgetAttribution(w, key.var);

                if (!_allWgts.contains(w)) {
                    centerLayout()->addWidget(w);
                    w->show();
                } else {
                    _allWgts.removeOne(w);
                }
            }
        }

        foreach (auto pW, _allWgts) {
            pW->setParent(nullptr);
            pW->hide();
        }

        _allWgts = newWantedWgts;

        if (_allWgts.count() == 1) {
            if (qobject_cast<CAttriBaseOverallWgt *>(_allWgts.first()) != nullptr) {
                _leftSpacer->changeSize(0, 0);
                _rightSpacer->changeSize(0, 0);
            } else {
                _leftSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
                _rightSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
            }
        } else {
            _leftSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
            _rightSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
        }

        autoResizeUpdate();
        _dirty = 0;
    }
}

QSize CAttributeManagerWgt::attriWidgetRecommendedSize(QWidget *pw)
{
    return CAttriBaseOverallWgt::attriWidgetRecommendedSize(pw);
}

void CAttributeManagerWgt::setWidgetAttribution(QWidget *pWgt, const QVariant &var)
{
    auto pw = qobject_cast<CAttributeWgt *>(pWgt);
    if (pw != nullptr) {
        pw->setVar(var);
    } else {
        pWgt->setProperty(AttriWidgetVar, var);
    }

    pWgt->blockSignals(true);
    emit updateWgt(pWgt, var);
    pWgt->blockSignals(false);
}

void CAttributeManagerWgt::onAttriWidgetValueChanged(const QVariant var, int phase)
{
    auto attriWidget = qobject_cast<CAttributeWgt *>(sender());
    if (attriWidget != nullptr && attriWidget->attribution() >= 0) {
        emit attributionChanged(attriWidget->attribution(), var, phase);
    }
}

void CAttributeManagerWgt::paintEvent(QPaintEvent *event)
{
    //qWarning() << "paintEvent ====================================== " << this->children();
    ensureAttributions();
    QWidget::paintEvent(event);

#ifdef QT_DEBUG
//    QPainter painter(this);
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor(255, 255, 0));
//    painter.drawRect(rect());
#endif
}



//void CAttributeManagerWgt::showEvent(QShowEvent *event)
//{
//    QMetaObject::invokeMethod(this, [ = ]() {
//        autoResizeUpdate();
//        setWidgetAllPosterityFocus(this);
//    }, Qt::QueuedConnection);
//    QWidget::showEvent(event);
//}

//void CAttributeManagerWgt::resizeEvent(QResizeEvent *event)
//{
//    ensureAttributions();
//    autoResizeUpdate();
//    QWidget::resizeEvent(event);
//}
