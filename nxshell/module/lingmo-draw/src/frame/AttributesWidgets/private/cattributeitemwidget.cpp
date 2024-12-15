// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cattributeitemwidget.h"


#include "csidewidthwidget.h"
#include "cspinbox.h"
#include "seperatorline.h"
#include "application.h"
#include "ccolorpickwidget.h"
#include "cattributemanagerwgt.h"
#include "toolbutton.h"
#include "cviewmanagement.h"

#include <DFontSizeManager>

#include <QFontComboBox>
#include <QLineEdit>

#include <QEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

using namespace DrawAttribution;
const int X_OFFSET = 25;
const int Y_OFFSET = -21;
DrawAttribution::CColorSettingButton::CColorSettingButton(const QString &text,
                                                          QWidget *parent, bool autoConnect):
    CAttributeWgt(-1, parent), _text(text)
{
    if (autoConnect) {
        connect(this, &CColorSettingButton::colorChanged, this, [ = ](const QColor & color, int phase) {
            emit attriChanged(color, phase);
        });
    }
}

void CColorSettingButton::setVar(const QVariant &var)
{
    QSignalBlocker bloker(this);
    QColor c = var.value<QColor>();
    setColor(c);
}

CColorPickWidget *CColorSettingButton::colorPick()
{
    static CColorPickWidget *_color = nullptr;
    if (_color == nullptr) {
        _color = new CColorPickWidget(this);
    }
    return _color;
}

void DrawAttribution::CColorSettingButton::setColorFill(DrawAttribution::CColorSettingButton::EColorFill fillPos)
{
    _fillPos = fillPos;
    update();
}

void DrawAttribution::CColorSettingButton::setColor(const QColor &color, int phase)
{
    _color = color;
    emit colorChanged(_color, phase);
    update();
}

void DrawAttribution::CColorSettingButton::setDefaultButtonSize(const QSize &sz)
{
    _defaultButtonSize = sz; setMaximumHeight(_defaultButtonSize.height());
}

void DrawAttribution::CColorSettingButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    initPainter(&painter);
    if (_fillPos == EFillArea) {
        paintFillArea(&painter);
    } else {
        paintFillBorder(&painter);
    }
}

QSize DrawAttribution::CColorSettingButton::sizeHint() const
{
    return minimumSizeHint();
}

QSize DrawAttribution::CColorSettingButton::minimumSizeHint() const
{
    QFontMetrics metrics(this->font());
    QSize textSz(metrics.width(_text), metrics.height());
    const int space = 6;
    return QSize(_defaultButtonSize.width() + space + textSz.width(), qMax(_defaultButtonSize.height(), textSz.width()));
}

void DrawAttribution::CColorSettingButton::mousePressEvent(QMouseEvent *event)
{
    QMetaObject::invokeMethod(this, [ = ]() {
        CColorPickWidget *colorPick = this->colorPick();
        if (!_connectedColorPicker) {
            connect(colorPick, &CColorPickWidget::colorChanged, this, [ = ](const QColor & color, EChangedPhase phase) {
                if (colorPick->caller() == this)
                    setColor(color, phase);
            });
            _connectedColorPicker = true;
        }

        colorPick->setCaller(this);
        colorPick->setColor(_color);
        //调色盘下半截自定义后不自动收起
        // colorPick->setExpandWidgetVisble(false);

        QPoint btnPos = mapToGlobal(QPoint(0, 0));
        QPoint pos(btnPos.x() + X_OFFSET,
                   btnPos.y() + this->height() + Y_OFFSET);

        colorPick->show(pos.x(), pos.y());

    }, Qt::QueuedConnection);
    event->accept();
    //调出调色板
    //return QWidget::mousePressEvent(event);
}

void DrawAttribution::CColorSettingButton::paintFillArea(QPainter *painter)
{
    QSize textSize = QSize(painter->fontMetrics().width(_text), painter->fontMetrics().height());

    QRect buttonRct = QRect(QPoint(0, (height() - _defaultButtonSize.height()) / 2), QSize(_defaultButtonSize.width(), _defaultButtonSize.height()));
    QRect textRct(QPoint(buttonRct.right() + _space, (height() - textSize.height()) / 2), textSize);

    bool isNotVaild = !_color.isValid();
    const QColor borderColor = (isNotVaild || _color.alpha() == 0) ? QColor(77, 82, 93, int(0.8 * 255)) : QColor(255, 255, 255, int(0.1 * 255));
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    const QSizeF btSz = buttonRct.size();

    QPointF topLeft = buttonRct.topLeft();

    QPen pen(painter->pen());

    bool   darkTheme = (DGuiApplicationHelper::instance()->themeType()  == 2);
    QColor penColor  = darkTheme ? borderColor : QColor(0, 0, 0, int(0.1 * 255));

    pen.setColor(penColor);

    pen.setWidthF(1.5);    //多加0.5宽度弥补防走样的误差

    painter->setPen(pen);

    painter->translate(topLeft);

    QPainterPath path;

    QRectF outerct(QPointF(0, 0), btSz);
    QRectF inerrct(QPointF(3, 3), btSz - QSizeF(2 * 3, 2 * 3));

    path.addRoundedRect(outerct, 8, 8);

    //填充色的设置(多选颜色冲突时(图元填充色不一致那么不设置填充色在后续绘制一条斜线))
    painter->setBrush((isNotVaild || _color.alpha() == 0) ?
                      (darkTheme ? QColor(8, 15, 21, int(0.7 * 255)) : QColor(0, 0, 0, int(0.05 * 255))) : _color);

    painter->drawPath(path);

    //如果多选颜色有冲突(isMult为true时)那么就绘制"..."
    if (isNotVaild) {
        painter->save();
        //painter->setPen(darkTheme ? QColor("#C0C6D4") : QColor("#414D68"));
        QColor cp = darkTheme ? QColor("#C5CFE0") : QColor("#000000");
        cp.setAlpha(darkTheme ? int(0.8 * 255) : int(0.2 * 255));
        painter->setPen(cp);
        QFont ft;
        ft.setPixelSize(14);
        painter->setFont(ft);
        painter->drawText(outerct.translated(0, -painter->fontMetrics().height() / 4), "...", QTextOption(Qt::AlignCenter));
        painter->restore();
    } else {
        //颜色没有冲突 如果颜色是透明的要绘制一条斜线表示没有填充色
        if (_color.alpha() == 0) {
            QPen pens(QColor(255, 67, 67, 153));
            pens.setWidthF(2.0);
            painter->setPen(pens);
            painter->drawLine(QLineF(inerrct.bottomLeft(), inerrct.topRight()));
        }
    }

    painter->restore();

    //绘制常量文字("填充")
    painter->save();
    painter->setPen(darkTheme ? QColor("#C0C6D4") : QColor("#414D68"));
    painter->drawText(textRct, _text, QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    painter->restore();
}

void DrawAttribution::CColorSettingButton::paintFillBorder(QPainter *painter)
{
    QSize textSize = QSize(painter->fontMetrics().width(_text), painter->fontMetrics().height());

    QRect buttonRct = QRect(QPoint(0, (height() - _defaultButtonSize.height()) / 2), QSize(_defaultButtonSize.width(), _defaultButtonSize.height()));
    QRect textRct(QPoint(buttonRct.right() + _space, (height() - textSize.height()) / 2), textSize);

    bool isNotVaild = !_color.isValid();
    const QColor borderColor = (isNotVaild || _color.alpha() == 0) ?
                               QColor(77, 82, 93, int(0.8 * 255)) : QColor(255, 255, 255, int(0.1 * 255));
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    const QSizeF btSz = buttonRct.size();

    QPointF topLeft = buttonRct.topLeft();

    QPen pen(painter->pen());

    bool   darkTheme = (DGuiApplicationHelper::instance()->themeType() == 2);
    QColor penColor  = darkTheme ? borderColor : QColor(0, 0, 0, int(0.1 * 255));

    pen.setColor(penColor);

    pen.setWidthF(1.5);    //多加0.5宽度弥补防走样的误差

    painter->setPen(pen);

    painter->translate(topLeft);

    QPainterPath path;

    QRectF outerct(QPointF(0, 0), btSz);
    QRectF inerrct(QPointF(3, 3), btSz - QSizeF(2 * 3, 2 * 3));

    path.addRoundedRect(outerct, 8, 8);
    path.addRoundedRect(inerrct, 5, 5);

    //线条的颜色用path的填充色来表示(如果是选中了多个图元那么有默认的颜色(该默认颜色与主题相关))
    painter->setBrush((isNotVaild || _color.alpha() == 0) ?
                      (darkTheme ? QColor(8, 15, 21, int(0.7 * 255)) : QColor(0, 0, 0, int(0.05 * 255))) : _color);

    painter->drawPath(path);

    //如果颜色是透明的要绘制一条斜线表示没有填充色
    if (!isNotVaild && _color.alpha() == 0) {
        QPen pens(QColor(255, 67, 67, 153));
        pens.setWidthF(2.0);
        painter->setPen(pens);
        painter->drawLine(QLineF(inerrct.bottomLeft(), inerrct.topRight()));
    }

    painter->restore();

    //绘制常量文字("描边")
    painter->save();
    painter->setPen(darkTheme ? QColor("#C0C6D4") : QColor("#414D68"));
    painter->drawText(textRct, _text, QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    painter->restore();
}


CComBoxSettingWgt::CComBoxSettingWgt(const QString &text, QWidget *parent):
    CAttributeWgt(-1, parent)
{
    _comBox = new QComboBox(this);
    _comBox->setMaximumHeight(36);
    _comBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _lab    = new QLabel(this);
    _lab->setText(text);
    QHBoxLayout *pLay = new QHBoxLayout;
    pLay->addWidget(_lab);
    pLay->addSpacing(10);
    pLay->addWidget(_comBox);
    this->setLayout(pLay);
    pLay->setContentsMargins(0, 0, 0, 0);

    if (text.isEmpty())
        _lab->hide();
    //connect(_comBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &CComBoxSettingWgt::onCurrentChanged);

    DFontSizeManager::instance()->bind(_comBox, DFontSizeManager::T6, QFont::Normal);
    DFontSizeManager::instance()->bind(_lab, DFontSizeManager::T6, QFont::Normal);
}

void CComBoxSettingWgt::setText(const QString &text)
{
    _lab->setText(text);
    if (text.isEmpty())
        _lab->hide();
    else {
        _lab->show();
    }
}

QComboBox *CComBoxSettingWgt::comboBox()
{
    return _comBox;
}

void CComBoxSettingWgt::setComboBox(QComboBox *pBox)
{
    if (_comBox != nullptr) {
        //disconnect(_comBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &CComBoxSettingWgt::onCurrentChanged);
        layout()->removeWidget(_comBox);
        _comBox->hide();
        _comBox->deleteLater();
    }
    if (pBox != nullptr) {
        layout()->addWidget(pBox);
        _comBox = pBox;
        pBox->setParent(this);
        DFontSizeManager::instance()->bind(_comBox, DFontSizeManager::T6, QFont::Normal);
        //connect(_comBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &CComBoxSettingWgt::onCurrentChanged);
    }
}

//void CComBoxSettingWgt::onCurrentChanged(int index)
//{
//    emit attriChanged(index);
//}

CAttributeWgt::CAttributeWgt(int attri, QWidget *parent): QWidget(parent)
{
    setAttribution(attri);
    this->hide();
}

void CAttributeWgt::setAttribution(int arrti)
{
    _attri = arrti;
}

int CAttributeWgt::attribution()
{
    return _attri;
}

void CAttributeWgt::setVar(const QVariant &var)
{
    this->setProperty(AttriWidgetVar, var);
}

QSize CAttributeWgt::recommendedSize() const
{
    return sizeHint();
}

bool CAttributeWgt::event(QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
        e->accept();
    }
    return QWidget::event(e);
}

CSpinBoxSettingWgt::CSpinBoxSettingWgt(const QString &text, QWidget *parent):
    CAttributeWgt(-1, parent)
{
    _spinBox = new CSpinBox(this);
    _spinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _lab    = new QLabel(this);
    _lab->setText(text);
    QHBoxLayout *pLay = new QHBoxLayout;
    pLay->addWidget(_lab);
    pLay->addSpacing(10);
    pLay->addWidget(_spinBox);
    this->setLayout(pLay);
    pLay->setContentsMargins(0, 0, 0, 0);
    if (text.isEmpty())
        _lab->hide();

    connect(_spinBox, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        emit attriChanged(value, phase);
        emit widthChanged(value);
    });
}

void CSpinBoxSettingWgt::setVar(const QVariant &var)
{
    if (var.isValid()) {
        QSignalBlocker blocker(_spinBox);
        _spinBox->setValue(var.toInt());
    } else {
        spinBox()->setSpecialText();
    }
}

CSpinBox *CSpinBoxSettingWgt::spinBox()
{
    return _spinBox;
}

QSize CSpinBoxSettingWgt::recommendedSize() const
{
    //return _lab->sizeHint() + _spinBox->sizeHint();
    return CAttributeWgt::sizeHint();
}



SAttrisList::SAttrisList(const QList<int> &list)
{
    for (int i = 0; i < list.count(); ++i) {
        SAttri attri;
        attri.attri = list.at(i);
        this->push_back(attri);
    }
}

SAttrisList SAttrisList::insected(const SAttrisList &other)
{
    SAttrisList result;
    for (auto it : *this) {
        for (auto ot : other) {
            if (it.attri == ot.attri) {
                SAttri temp;
                temp.attri = it.attri;
                if (it.var == ot.var) {
                    temp.var = ot.var;
                }
                result.append(temp);
                break;
            }
        }
    }
    return result;
}

bool SAttrisList::haveAttribution(int attri)
{
    for (int i = 0; i < count(); ++i) {
        if (this->at(i).attri == attri) {
            return true;
        }
    }
    return false;
}

CGroupButtonWgt::CGroupButtonWgt(QWidget *parent): CAttributeWgt(EGroupWgt, parent)
{
    QVariant var; var.setValue<QMargins>(QMargins(0, 0, 0, 0));
    setProperty(WidgetMarginInVerWindow, var);
    setProperty(WidgetAlignInVerWindow, 0);
    //组合按钮
    groupButton = new DIconButton(nullptr);
    groupButton->setIcon(QIcon::fromTheme("menu_group_normal"));
    setWgtAccesibleName(groupButton, "groupButton");
    groupButton->setFixedSize(36, 36);
    groupButton->setIconSize(QSize(20, 20));
    groupButton->setContentsMargins(0, 0, 0, 0);
    //释放组合按钮
    unGroupButton = new DIconButton(nullptr);
    unGroupButton->setIcon(QIcon::fromTheme("menu_ungroup_normal"));
    setWgtAccesibleName(unGroupButton, "unGroupButton");
    unGroupButton->setFixedSize(36, 36);
    unGroupButton->setIconSize(QSize(20, 20));
    unGroupButton->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *play = new QHBoxLayout;
    play->setContentsMargins(0, 0, 0, 0);

    play->addWidget(groupButton);
    play->addWidget(unGroupButton);
    this->setLayout(play);

    groupButton->show();
    unGroupButton->show();

    _labelGroup   = new QLabel(tr("Group"), this);
    _labelGroup->hide();

    _labelUngroup = new QLabel(tr("Ungroup"), this);
    _labelUngroup->hide();


    {
        //组合按钮
        expGroupBtn = new ToolButton(this);
        //expGroupBtn->setFixedSize(200, 34);
        expGroupBtn->setMinimumSize(200, 34);
        expGroupBtn->setText(tr("Group"));
        expGroupBtn->setIcon(QIcon::fromTheme("icon_group_normal"));
        expGroupBtn->hide();

        //释放组合按钮
        expUnGroupBtn = new ToolButton(this);
        //expUnGroupBtn->setFixedSize(200, 34);
        expUnGroupBtn->setMinimumSize(200, 34);
        expUnGroupBtn->setText(tr("Ungroup"));
        expUnGroupBtn->setIcon(QIcon::fromTheme("icon_ungroup_normal"));
        expUnGroupBtn->hide();
    }
    connect(groupButton, &DIconButton::clicked, this, [ = ]() {
        emit this->buttonClicked(true, false);
    });
    connect(unGroupButton, &DIconButton::clicked, this, [ = ]() {
        emit this->buttonClicked(false, true);
    });
    connect(expGroupBtn, &QPushButton::clicked, this, [ = ]() {
        emit this->buttonClicked(true, false);
    });
    connect(expUnGroupBtn, &QPushButton::clicked, this, [ = ]() {
        emit this->buttonClicked(false, true);
    });
}

QSize CGroupButtonWgt::recommendedSize() const
{
    int w = 12 + 36 + 36 + 12;
    int h = 36;
    return QSize(w, h);
}

//CAttributeManagerWgt *CGroupButtonWgt::managerParent()
//{
//    if (parentWidget() != nullptr) {
//        return qobject_cast<CAttributeManagerWgt *>(parentWidget());
//    }
//    return nullptr;
//}

//int CGroupButtonWgt::parentChildCount()
//{
//    if (managerParent() == nullptr) {
//        return 0;
//    }
//    return managerParent()->attributeWgts().count();
//}

void CGroupButtonWgt::setGroupFlag(bool canGroup, bool canUngroup)
{
    groupButton->setEnabled(canGroup);
    unGroupButton->setEnabled(canUngroup);

    expGroupBtn->setEnabled(canGroup);
    expUnGroupBtn->setEnabled(canUngroup);
}

bool CGroupButtonWgt::event(QEvent *e)
{
    if (e->type() == QEvent::ParentChange) {
        if (this->parentWidget() != nullptr) {
            if (parentWidget()->isWindow()) {
                QMetaObject::invokeMethod(this, [ = ]() {
                    if (this->layout()->expandingDirections() != Qt::Vertical) {
                        delete  this->layout();
                        QVBoxLayout *play = new QVBoxLayout;
                        play->setContentsMargins(0, 0, 0, 0);
                        play->addWidget(expGroupBtn);
                        play->addWidget(expUnGroupBtn);
                        this->setLayout(play);
                        groupButton->hide();
                        unGroupButton->hide();

                        expGroupBtn->show();
                        expUnGroupBtn->show();
                    }
                }, Qt::QueuedConnection);
            } else {
                QMetaObject::invokeMethod(this, [ = ]() {
                    if (this->layout()->expandingDirections() != Qt::Horizontal) {
                        delete  this->layout();
                        QHBoxLayout *play = new QHBoxLayout;
                        play->setContentsMargins(0, 0, 0, 0);

                        play->addWidget(groupButton);
                        play->addWidget(unGroupButton);
                        this->setLayout(play);

                        _labelGroup->hide();
                        _labelUngroup->hide();
                        expGroupBtn->hide();
                        expUnGroupBtn->hide();
                        groupButton->show();
                        unGroupButton->show();
                    }
                }, Qt::QueuedConnection);
            }
        }
    }
    return CAttributeWgt::event(e);
}

bool CGroupButtonWgt::eventFilter(QObject *o, QEvent *e)
{
    return CAttributeWgt::eventFilter(o, e);
}

CAttriBaseOverallWgt::CAttriBaseOverallWgt(QWidget *parent): CAttributeWgt(-1, parent)
{
    QHBoxLayout *pLay = new QHBoxLayout;

    _leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    pLay->addSpacerItem(_leftSpacer);

    pLay->addLayout(centerLayout());
    pLay->addSpacing(6);
    pLay->addWidget(getExpButton());

    _rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    pLay->addSpacerItem(_rightSpacer);

    pLay->setSpacing(0);
    this->setLayout(pLay);
    pLay->setContentsMargins(0, 0, 0, 0);
    connect(getExpButton(), &CExpButton::clicked, this, [ = ]() {
        QPoint pos = (getExpButton()->mapToGlobal(getExpButton()->rect().center()));
        getExpsWidget()->move(pos);
        getExpsWidget()->show();
    });
    setFocusPolicy(Qt::StrongFocus);
}

void CAttriBaseOverallWgt::hideExpWindow()
{
    if (getExpsWidget()->isVisible()) {
        getExpsWidget()->close();
    }
}

void CAttriBaseOverallWgt::showEvent(QShowEvent *event)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6, QFont::Normal);
    //move(mapFromGlobal(pos()));
    QMetaObject::invokeMethod(this, [ = ]() {
        autoResizeUpdate();
        //setWidgetAllPosterityFocus(this);
    }, Qt::QueuedConnection);
    QWidget::showEvent(event);

}

void CAttriBaseOverallWgt::resizeEvent(QResizeEvent *event)
{
    autoResizeUpdate();
    QWidget::resizeEvent(event);
}

bool CAttriBaseOverallWgt::event(QEvent *e)
{
    if (e->type() == QEvent::FontChange) {
        autoResizeUpdate();
    }
    return QWidget::event(e);
}

QSize CAttriBaseOverallWgt::recommendedSize() const
{
    return QSize(0, 0);
}
void CAttriBaseOverallWgt::setWidgetAllPosterityFocus(QWidget *pW)
{
    if (pW == nullptr)
        return;

    QList<QObject *> list0 = pW->children();
    for (int i  = 0; i < list0.size(); ++i) {
        QObject *o = list0[i];
        if (o->isWidgetType()) {
            QWidget *pWidget = qobject_cast<QWidget *>(o);
            if (pWidget != nullptr) {
                pWidget->setFocusPolicy(Qt::StrongFocus);
                setWidgetAllPosterityFocus(pWidget);
            }
        }
    }
}

//bool CAttriBaseOverallWgt::widgetShowInBaseOverallWgt(const QWidget *w)
//{
//    auto show = w->property(WidgetShowInHorBaseWidget);
//    if (show.isValid())
//        return show.toBool();
//    return true;
//}

QSize CAttriBaseOverallWgt::attriWidgetRecommendedSize(QWidget *pw)
{
    QVariant var = pw->property(AttriWidgetReWidth);
    if (var.isValid()) {
        return var.toSize();
    }

    auto pAttriW = qobject_cast<CAttributeWgt *>(pw);
    if (pAttriW != nullptr) {
        return pAttriW->recommendedSize();
    }
    return pw->sizeHint();
}

QLayout *CAttriBaseOverallWgt::centerLayout()
{
    if (_pCenterLay == nullptr) {
        _pCenterLay = new QHBoxLayout;
        _pCenterLay->setSpacing(9);
        _pCenterLay->setContentsMargins(0, 0, 0, 0);
    }
    return _pCenterLay;
}

void CAttriBaseOverallWgt::addWidget(QWidget *w)
{
    _allWgts.append(w);
    autoResizeUpdate();
}

CExpWgt *CAttriBaseOverallWgt::getExpsWidget()
{
    if (_pExpWidget == nullptr) {
        _pExpWidget = new CExpWgt(this);
        setWgtAccesibleName(_pExpWidget, "_pExpWidget");
    }
    return _pExpWidget;
}

CExpButton *CAttriBaseOverallWgt::getExpButton()
{
    if (_expBtn == nullptr) {
        _expBtn = new CExpButton(this);
    }
    return _expBtn;
}

void CAttriBaseOverallWgt::autoResizeUpdate()
{
    getExpsWidget()->clear();
    int totalNeedW = totalNeedWidth();

    if (totalNeedW > width()) {
        // 当属性栏所有部件之和大于属性栏宽度
        getExpButton()->show();
        int tempW = getExpButton()->sizeHint().width();
        int takeBeginIndex = _allWgts.count();
        for (int i = 0; i < _allWgts.count(); ++i) {
            auto pw = _allWgts[i];
            int w = attriWidgetRecommendedSize(pw).width();
            tempW += w;
            if (i < _allWgts.count() - 1) {
                tempW += centerLayout()->spacing();
            }
            if (tempW > width()) {
                //qDebug() << "i = " << i << "tempW = " << tempW << "width() = " << width() << "j = " << (tempW - w);
                takeBeginIndex = i;
                break;
            } else {
                // 在属性栏宽度减小时，部件合入下拉框，再随着属性栏宽度增加，部件逐一显示到属性栏上
                centerLayout()->addWidget(pw);
                pw->show();
            }
        }

        for (int i = takeBeginIndex; i < _allWgts.count(); ++i) {
            auto pw = _allWgts.at(i);
            getExpsWidget()->addWidget(pw);
        }

    } else {
        // 当属性栏所有部件之和小于属性栏宽度
        getExpButton()->hide();
        for (int i = 0; i < _allWgts.count(); ++i) {
            auto pw = _allWgts[i];
            centerLayout()->addWidget(pw);
            pw->show();
        }
    }
}

int CAttriBaseOverallWgt::totalNeedWidth()
{
    int totalW = 0;
    for (int i = 0; i < _allWgts.count(); ++i) {
        auto pw = _allWgts[i];
        int w = attriWidgetRecommendedSize(pw).width();
        totalW += w;
        if (i < _allWgts.count() - 1) {
            totalW += centerLayout()->spacing();
        }
    }
    return totalW;
}


CSpline::CSpline(QWidget *parent): QFrame(parent)
{
    //this->setObjectName(QStringLiteral("line"));
    this->setGeometry(QRect(300, 200, 118, 3));
    this->setFrameShape(QFrame::HLine);
    this->setFrameShadow(QFrame::Sunken);
}
