// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ctexttool.h"
#include "cdrawscene.h"
#include "cgraphicstextitem.h"
#include "cdrawparamsigleton.h"
#include "widgets/ctextedit.h"
#include "frame/cgraphicsview.h"
#include "cundoredocommand.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
#include "application.h"
#include "cviewmanagement.h"
#include "cundoredocommand.h"

#include <DFontSizeManager>

#include <QLineEdit>
#include <QFontComboBox>
#include <QAbstractItemView>

#include <QGraphicsView>
#include <DToolButton>

CTextTool::CTextTool()
    : IDrawTool(text)
{
    supWeightStyleList << "Regular" << "Black" << "DemiBold" << "Bold" << "Medium" << "Light" << "ExtraLight";
    _defaultFontSizeSet << 8 << 10 << 12 << 14 << 16 << 18 << 24 << 36 << 48 << 60 << 72 << 100;
}

CTextTool::~CTextTool()
{

}

DrawAttribution::SAttrisList CTextTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EFontColor)
           << defaultAttriVar(DrawAttribution::EFontFamily)
           << defaultAttriVar(DrawAttribution::EFontWeightStyle)
           << defaultAttriVar(DrawAttribution::EFontSize);
    return result;
}

void CTextTool::registerAttributionWidgets()
{
    //1.安装文字颜色设置控件
    auto fontColor = new CColorSettingButton(tr("Color"), nullptr, false);
    fontColor->setAttribution(EFontColor);
    drawBoard()->attributionWidget()->installComAttributeWgt(EFontColor, fontColor, QColor(0, 0, 0));
    setWgtAccesibleName(fontColor, "Text color button");

    connect(fontColor, &CColorSettingButton::colorChanged, this, [ = ](const QColor & color, int phase) {
        CCmdBlock block(isTextEnableUndoThisTime() ? drawBoard()->currentPage()->scene()->selectGroup() : nullptr);
        drawBoard()->setDrawAttribution(EFontColor, color, phase, false);
    });


    //2.安装字体字重设置控件
    initFontWeightWidget();

    //3.安装字体族设置控件
    initFontFamilyWidget(m_fontHeavy);

    //4.安装字体大小设置控件
    initFontFontSizeWidget();
}

QCursor CTextTool::cursor() const
{
    return QCursor(Qt::IBeamCursor);
}

QAbstractButton *CTextTool::initToolButton()
{
    DToolButton *m_textBtn = new DToolButton;
    m_textBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_T)));
    setWgtAccesibleName(m_textBtn, "Text tool button");
    m_textBtn->setToolTip(tr("Text (T)"));
    m_textBtn->setIconSize(QSize(48, 48));
    m_textBtn->setFixedSize(QSize(37, 37));
    m_textBtn->setCheckable(true);

    connect(m_textBtn, &DToolButton::toggled, m_textBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_text tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_text tool_active");
        m_textBtn->setIcon(b ? activeIcon : icon);
    });
    m_textBtn->setIcon(QIcon::fromTheme("ddc_text tool_normal"));
    return m_textBtn;
}

void CTextTool::transferFocusBack()
{
    if (this->drawBoard()->currentPage() == nullptr)
        return;

    //另外需要将焦点转移到text
    auto pView = this->drawBoard()->currentPage()->view();
    pView->captureFocus();
}

void CTextTool::onSizeChanged(int fontSz, bool backFocus)
{
    fontSz = qMin(qMax(fontSz, 8), 500);
    QSignalBlocker blokerLineEditor(m_fontSize->lineEdit());
    m_fontSize->lineEdit()->setText(QString("%1px").arg(fontSz));

    QSignalBlocker bloker(m_fontSize);
    if (_currenFontSize != fontSz) {
        _currenFontSize = fontSz;
        CCmdBlock block(isTextEnableUndoThisTime() ? drawBoard()->currentPage()->scene()->selectGroup() : nullptr);
        drawBoard()->setDrawAttribution(EFontSize, fontSz, EChanged, false);
    }

    if (backFocus)
        transferFocusBack();

    auto index = m_fontSize->findText(m_fontSize->lineEdit()->text());
    qobject_cast<QComboxMenuDelegate *>(m_fontSize->itemDelegate())->dontShowCheckState(index == -1);
}

void CTextTool::resetItemsFontFamily()
{
    _cachedFontFamily.clear();
}

void CTextTool::cachedItemsFontFamily()
{
    resetItemsFontFamily();
    if (drawBoard()->currentPage() != nullptr) {
        auto group = drawBoard()->currentPage()->scene()->selectGroup();
        auto bzItems = group->getBzItems(true);
        foreach (auto p, bzItems) {
            if (p->type() == TextType) {
                auto textItem = static_cast<CGraphicsTextItem *>(p);
                Font ft;
                ft.FontWeight = textItem->fontStyle();
                ft.fontFamily = textItem->fontFamily();
                _cachedFontFamily.insert(textItem, ft);
                textItem->beginPreview();
            }
        }
    }
}

void CTextTool::restoreItemsFontFamily()
{
    for (auto it = _cachedFontFamily.begin(); it != _cachedFontFamily.end(); ++it) {
        if (!it->fontFamily.isEmpty()) {
            QSignalBlocker blocker(m_fontHeavy);
            it.key()->setFontFamily(it->fontFamily);
            reInitFontWeightComboxItems(it->fontFamily, m_fontHeavy);
            it.key()->setFontStyle(it->FontWeight);

            m_fontHeavy->setCurrentText(it->FontWeight);
        } else {
            QString family  = drawApp->drawBoard()->currentPage()->defaultAttriVar(EFontFamily).toString();
            QSignalBlocker blocker(m_fontComBox);
            m_fontComBox->setCurrentText(family);
        }
    }
}
bool CTextTool::isTextEnableUndoThisTime(bool considerRecorderEmpty)
{
    if (currentPage() == nullptr)
        return false;

    //如果当前是激活的状态(意味着文字是处于编辑状态),那么文字外部撤销还原栈不用进行数据收集
    auto curView = this->currentPage()->view();
    auto curActiveBzItem = curView->activeProxDrawItem();
    if (curActiveBzItem != nullptr) {
        if (curActiveBzItem->type() == TextType) {
            return false;
        }
    }
    if (considerRecorderEmpty)
        return CUndoRedoCommand::isRecordEmpty();

    return true;
}

void CTextTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsTextItem *pItem = dynamic_cast<CGraphicsTextItem *>(pInfo->businessItem);
        if (nullptr != pItem) {
            if (pItem->scene() == nullptr) {
                pItem->drawScene()->addCItem(pItem);
            }

            if (pItem->scene() != nullptr && !rasterItemToLayer(event, pInfo))
                CCmdBlock block(event->scene(), CSceneUndoRedoCommand::EItemAdded, pItem);

            pItem->setTextState(CGraphicsTextItem::EInEdit, true);
            pItem->textEditor()->applyDefaultToFirstFormat();
            pItem->textEditor()->document()->clearUndoRedoStacks();
            event->scene()->selectItem(pItem);
        }
    }

    IDrawTool::toolCreatItemFinish(event, pInfo);
}

CGraphicsItem *CTextTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((event->eventType() == CDrawToolEvent::EMouseEvent && event->mouseButtons() == Qt::LeftButton)
            || event->eventType() == CDrawToolEvent::ETouchEvent) {
        CGraphicsTextItem *pItem =  new CGraphicsTextItem(tr("Input text here"));
        //初始化不要响应信号
        QSignalBlocker blocker(pItem->textEditor());

        pItem->setPos(event->pos().x(), event->pos().y());
        pItem->textEditor()->setAlignment(Qt::AlignLeft);
        pItem->textEditor()->selectAll();
        pItem->textEditor()->document()->clearUndoRedoStacks();
        event->scene()->addCItem(pItem);

        return pItem;
    }
    return nullptr;
}

int CTextTool::minMoveUpdateDistance()
{
    return 0;
}

bool CTextTool::isPressEventHandledByQt(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)

    return IDrawTool::isPressEventHandledByQt(event, pInfo);
}

bool CTextTool::rasterItemToLayer(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    return false;
}

bool CTextTool::eventFilter(QObject *o, QEvent *event)
{
    if (o == m_fontSize->view() || o == m_fontComBox->view() || o == m_fontHeavy->view()) {
        if (event->type() == QEvent::Show) {
            if (o == m_fontComBox->view()) {
                _activePackup = false;
                _fontViewShowOut = true;
                cachedItemsFontFamily();
                CCmdBlock block(isTextEnableUndoThisTime() ? drawBoard()->currentPage()->scene()->selectGroup() : nullptr, EChangedBegin);
            }

        } else if (event->type() == QEvent::Hide) {
            if (o == m_fontComBox->view()) {
                _fontViewShowOut = false;
                QMetaObject::invokeMethod(this, [ = ]() {
                    // _activePackup值为false控件预览，值为true重新赋值
                    if (!_activePackup) {
                        //还原
                        this->restoreItemsFontFamily();
                        CUndoRedoCommand::clearCommand();
                    } else {
                        drawBoard()->setDrawAttribution(EFontFamily, m_fontComBox->currentText(), EChangedFinished, false);
                        reInitFontWeightComboxItems(m_fontComBox->currentText(), m_fontHeavy);
                        CCmdBlock block(isTextEnableUndoThisTime(false) ? drawBoard()->currentPage()->scene()->selectGroup() : nullptr, EChangedFinished);
                    }
                    for (auto it = _cachedFontFamily.begin(); it != _cachedFontFamily.end(); ++it) {
                        auto textItem = static_cast<CGraphicsTextItem *>(it.key());
                        textItem->endPreview(false);
                    }
                }, Qt::QueuedConnection);
            }



            //transferFocusBack();

            return true;
        }
    } else if (o == m_fontSize) {
        //实现按键响应的字体大小设置
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (Qt::Key_Up == keyEvent->key() || Qt::Key_PageUp == keyEvent->key()) {
                int size = m_fontSize->currentText().replace("px", "").toInt();
                ++size;
                size = qMin(qMax(size, 8), 500);
                m_fontSize->setCurrentText(QString("%1px").arg(size));
                onSizeChanged(size, false);
                return true;
            }

            if (Qt::Key_Down == keyEvent->key() || Qt::Key_PageDown == keyEvent->key()) {
                int size = m_fontSize->currentText().replace("px", "").toInt();
                --size;
                size = qMin(qMax(size, 8), 500);
                m_fontSize->setCurrentText(QString("%1px").arg(size));
                onSizeChanged(size, false);
                return true;
            }
        }
    } else if (m_fontHeavy->parentWidget() == o) {
        if (QEvent::ParentChange == event->type()) {
            CComBoxSettingWgt *comb = qobject_cast<CComBoxSettingWgt *>(m_fontHeavy->parentWidget());

            if (comb->parentWidget() != nullptr) {
                if (comb->parentWidget()->isWindow()) {
                    comb->setText(tr("Weight"));
                } else {
                    comb->setText("");
                }
            }
        }
    }

    return IDrawTool::eventFilter(o, event);
}

void CTextTool::initFontFamilyWidget(QComboBox *fontHeavy)
{
    auto attriMangerWgt = drawBoard()->attributionWidget();

    //文字字体名设置控件
    auto fontFamily = new CComBoxSettingWgt(tr("Font"));
    fontFamily->setAttribution(EFontFamily);
    auto fontComboBox = new QFontComboBox;
    setWgtAccesibleName(fontComboBox, "Text font family comboBox");
    fontComboBox->setFocusPolicy(Qt::NoFocus);
    fontComboBox->setEditable(true);
    fontComboBox->lineEdit()->setReadOnly(true);
    fontFamily->setComboBox(fontComboBox);

    // 设置默认的字体类型为思源宋黑体，没有该字体则选择系统第一个默认字体
    QFontDatabase fontbase;
    QString sourceHumFont = QObject::tr("Source Han Sans CN");
    if (!fontbase.families().contains(sourceHumFont)) {
        sourceHumFont = fontbase.families().first();
    }
    drawBoard()->attributionWidget()->installComAttributeWgt(EFontFamily, fontFamily, sourceHumFont);

    connect(fontComboBox, QOverload<const QString &>::of(&QComboBox::activated), this, [ = ](const QString & family) {
        Q_UNUSED(family)
        _activePackup = true;
    });

    connect(fontComboBox, QOverload<const QString &>::of(&QComboBox::highlighted), this, [ = ](const QString & family) {
        //预览的不用支持撤销还原
        if (_fontViewShowOut) {
            drawBoard()->setDrawAttribution(EFontFamily, family, EChanged, false);
            reInitFontWeightComboxItems(family, fontHeavy);
        }
    });

    connect(attriMangerWgt, &CAttributeManagerWgt::updateWgt, fontFamily, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == fontFamily) {
            QSignalBlocker blocker(fontComboBox);
            QString string = var.isValid() ? var.toString() : QString("— —");
            if (string.isEmpty()) {
                string = QString("— —");
            } else {
                QSignalBlocker FWeightblocker(fontHeavy);
                reInitFontWeightComboxItems(string, fontHeavy);
            }
            fontComboBox->setCurrentText(string);
        }
    });

    m_fontComBox = fontComboBox;
    m_fontComBox->view()->installEventFilter(this);

    QSignalBlocker blocker(fontHeavy);
    reInitFontWeightComboxItems(sourceHumFont, fontHeavy);
}

void CTextTool::initFontWeightWidget()
{
    auto attriMangerWgt = drawBoard()->attributionWidget();
    //文字字重设置控件
    auto fontWeightStyle = new CComBoxSettingWgt();
    fontWeightStyle->setAttribution(EFontWeightStyle);
    auto ftStyleComboBox = new QComboBox;
    setWgtAccesibleName(ftStyleComboBox, "Text font style comboBox");
    ftStyleComboBox->setFocusPolicy(Qt::NoFocus);
    ftStyleComboBox->setMinimumWidth(130);
    ftStyleComboBox->setEditable(true);
    ftStyleComboBox->lineEdit()->setReadOnly(true);
    ftStyleComboBox->addItems(supWeightStyleList);
    fontWeightStyle->setComboBox(ftStyleComboBox);

    connect(ftStyleComboBox, &QComboBox::currentTextChanged, fontWeightStyle, [ = ](const QString & style) {
        CCmdBlock block(isTextEnableUndoThisTime() ? drawBoard()->currentPage()->scene()->selectGroup() : nullptr);
        drawBoard()->setDrawAttribution(EFontWeightStyle, style, EChanged, false);
    });
    connect(attriMangerWgt, &CAttributeManagerWgt::updateWgt, fontWeightStyle, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == fontWeightStyle) {
            QSignalBlocker blocker(ftStyleComboBox);
            QString string = var.isValid() ? var.toString() : QString("— —");
            if (string.isEmpty()) {
                string = QString("— —");
            }
            ftStyleComboBox->setCurrentText(string);
            for (int i = 0; i < ftStyleComboBox->count(); ++i) {
                if (ftStyleComboBox->itemText(i) == string) {
                    ftStyleComboBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EFontWeightStyle, fontWeightStyle, supWeightStyleList.first());

    m_fontHeavy = ftStyleComboBox;

    fontWeightStyle->installEventFilter(this);
    m_fontHeavy->view()->installEventFilter(this);
}

void CTextTool::reInitFontWeightComboxItems(const QString &family, QComboBox *fontHeavy)
{
    auto currentWeightStyle = fontHeavy->currentText();
    int canKeepIndex = -1;
    {
        QSignalBlocker blocker(fontHeavy);
        fontHeavy->clear();
        QFontDatabase base;
        QStringList listStylyName = base.styles(family);
        for (int i = 0; i < supWeightStyleList.count(); ++i) {
            auto style = supWeightStyleList.at(i);
            if (listStylyName.contains(style)) {
                fontHeavy->addItem(style);
            }
        }
        canKeepIndex = fontHeavy->findText(currentWeightStyle);
        fontHeavy->setCurrentIndex(-1);
    }
    //qDebug() << "currentWeightStyle = " << currentWeightStyle << "canKeepIndex = " << canKeepIndex;
    if (canKeepIndex < 0) {
        fontHeavy->setCurrentIndex(0);
    } else {
        fontHeavy->setCurrentIndex(canKeepIndex);
    }
}

void CTextTool::initFontFontSizeWidget()
{
    auto attriMangerWgt = drawBoard()->attributionWidget();

    //文字字体大小设置控件
    auto fontSize = new CComBoxSettingWgt(tr("Size"));
    fontSize->setAttribution(EFontSize);
    auto ftSizeComboBox = new QComboBox;
    ftSizeComboBox->setEditable(true);
    ftSizeComboBox->setMinimumWidth(105);
    fontSize->setComboBox(ftSizeComboBox);
    setWgtAccesibleName(fontSize->comboBox(), "Text font size comboBox");
    for (int i = 0; i < _defaultFontSizeSet.count(); ++i) {
        auto text = QString("%1px").arg(_defaultFontSizeSet[i]);
        ftSizeComboBox->addItem(text);
    }
    QRegExp regx("[0-9]*p?x?");
    QValidator *validator = new QRegExpValidator(regx, ftSizeComboBox);
    ftSizeComboBox->setValidator(validator);
    connect(ftSizeComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), fontSize, [ = ](const QString & fontSize) {
        int size = QString(fontSize).remove("px").toInt();
        onSizeChanged(size, false);
    });
    connect(ftSizeComboBox->lineEdit(), &QLineEdit::editingFinished, fontSize, [ = ]() {
        int size = QString(ftSizeComboBox->currentText()).remove("px").toInt();
        onSizeChanged(size);
    });

    connect(attriMangerWgt, &CAttributeManagerWgt::updateWgt, fontSize, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == fontSize) {
            QSignalBlocker blocker(ftSizeComboBox);
            QString text = (!var.isValid() || var.toInt() == 0) ?  QStringLiteral("— —") : QString("%1px").arg(var.toInt());
            ftSizeComboBox->setCurrentText(text);
            //修复两个标签页中文本字号一起改变
            _currenFontSize = var.toInt();
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EFontSize, fontSize, _currenFontSize);

    ftSizeComboBox->lineEdit()->setReadOnly(Application::isTabletSystemEnvir());

    m_fontSize = ftSizeComboBox;

    //combox的lineedit控件编辑文字后不用添加新的item
    m_fontSize->setInsertPolicy(QComboBox::NoInsert);

    m_fontSize->view()->installEventFilter(this);
    m_fontSize->installEventFilter(this);

    auto delegate = new QComboxMenuDelegate(m_fontSize->view(), m_fontSize);
    m_fontSize->setItemDelegate(delegate);
}

//typedef QHash<QByteArray, QFont> FontHash;
extern QHash<QByteArray, QFont> *qt_app_fonts_hash();
QStyleOptionMenuItem QComboxMenuDelegate::getStyleOption(const QStyleOptionViewItem &option,
                                                         const QModelIndex &index) const
{
    QStyleOptionMenuItem menuOption;

    QPalette resolvedpalette = option.palette.resolve(QApplication::palette("QMenu"));
    QVariant value = index.data(Qt::ForegroundRole);
    if (value.canConvert<QBrush>()) {
        resolvedpalette.setBrush(QPalette::WindowText, qvariant_cast<QBrush>(value));
        resolvedpalette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(value));
        resolvedpalette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));
    }
    menuOption.palette = resolvedpalette;
    menuOption.state = QStyle::State_None;
    if (mCombo->window()->isActiveWindow())
        menuOption.state = QStyle::State_Active;
    if ((option.state & QStyle::State_Enabled) && (index.model()->flags(index) & Qt::ItemIsEnabled))
        menuOption.state |= QStyle::State_Enabled;
    else
        menuOption.palette.setCurrentColorGroup(QPalette::Disabled);
    if (option.state & QStyle::State_Selected)
        menuOption.state |= QStyle::State_Selected;
    menuOption.checkType = QStyleOptionMenuItem::NonExclusive;
    // a valid checkstate means that the model has checkable items
    const QVariant checkState = index.data(Qt::CheckStateRole);
    if (!checkState.isValid()) {
        menuOption.checked = mCombo->currentIndex() == index.row();
    } else {
        menuOption.checked = qvariant_cast<int>(checkState) == Qt::Checked;
        menuOption.state |= qvariant_cast<int>(checkState) == Qt::Checked
                            ? QStyle::State_On : QStyle::State_Off;
    }
    if (/*QComboBoxDelegate::isSeparator(index)*/index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator"))
        menuOption.menuItemType = QStyleOptionMenuItem::Separator;
    else
        menuOption.menuItemType = QStyleOptionMenuItem::Normal;

    QVariant variant = index.model()->data(index, Qt::DecorationRole);
    switch (variant.userType()) {
    case QMetaType::QIcon:
        menuOption.icon = qvariant_cast<QIcon>(variant);
        break;
    case QMetaType::QColor: {
        static QPixmap pixmap(option.decorationSize);
        pixmap.fill(qvariant_cast<QColor>(variant));
        menuOption.icon = pixmap;
        break;
    }
    default:
        menuOption.icon = qvariant_cast<QPixmap>(variant);
        break;
    }
    if (index.data(Qt::BackgroundRole).canConvert<QBrush>()) {
        menuOption.palette.setBrush(QPalette::All, QPalette::Window,
                                    qvariant_cast<QBrush>(index.data(Qt::BackgroundRole)));
    }
    menuOption.text = index.model()->data(index, Qt::DisplayRole).toString()
                      .replace(QLatin1Char('&'), QLatin1String("&&"));
    //menuOption.reservedShortcutWidth = 0;
    menuOption.maxIconWidth =  option.decorationSize.width() + 4;
    menuOption.menuRect = option.rect;
    menuOption.rect = option.rect;

    // Make sure fonts set on the model or on the combo box, in
    // that order, also override the font for the popup menu.
    QVariant fontRoleData = index.data(Qt::FontRole);
    if (fontRoleData.isValid()) {
        menuOption.font = qvariant_cast<QFont>(fontRoleData);
    } else if (mCombo->testAttribute(Qt::WA_SetFont)
               || mCombo->testAttribute(Qt::WA_MacSmallSize)
               || mCombo->testAttribute(Qt::WA_MacMiniSize)
               || mCombo->font() != qt_app_fonts_hash()->value("QComboBox", QFont())) {
        menuOption.font = mCombo->font();
    } else {
        menuOption.font = qt_app_fonts_hash()->value("QComboMenuItem", mCombo->font());
    }

    menuOption.fontMetrics = QFontMetrics(menuOption.font);

    return menuOption;
}

//bool QComboxMenuDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
//                                      const QStyleOptionViewItem &option, const QModelIndex &index)
//{
//    Q_ASSERT(event);
//    Q_ASSERT(model);

//    // make sure that the item is checkable
//    Qt::ItemFlags flags = model->flags(index);
//    if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
//            || !(flags & Qt::ItemIsEnabled))
//        return false;

//    // make sure that we have a check state
//    const QVariant checkState = index.data(Qt::CheckStateRole);
//    if (!checkState.isValid())
//        return false;

//    // make sure that we have the right event type
//    if ((event->type() == QEvent::MouseButtonRelease)
//            || (event->type() == QEvent::MouseButtonDblClick)
//            || (event->type() == QEvent::MouseButtonPress)) {
//        QMouseEvent *me = static_cast<QMouseEvent *>(event);
//        if (me->button() != Qt::LeftButton)
//            return false;

//        if ((event->type() == QEvent::MouseButtonPress)
//                || (event->type() == QEvent::MouseButtonDblClick)) {
//            pressedIndex = index.row();
//            return false;
//        }

//        if (index.row() != pressedIndex)
//            return false;
//        pressedIndex = -1;

//    } else if (event->type() == QEvent::KeyPress) {
//        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space
//                && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
//            return false;
//    } else {
//        return false;
//    }

//    // we don't support user-tristate items in QComboBox (not implemented in any style)
//    Qt::CheckState newState = (static_cast<Qt::CheckState>(checkState.toInt()) == Qt::Checked)
//                              ? Qt::Unchecked : Qt::Checked;
//    return model->setData(index, newState, Qt::CheckStateRole);
//}
