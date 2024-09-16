// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QComboBox>
#include <QFontComboBox>
#include <DLineEdit>
#include <QAbstractItemView>

#include "textattributionregister.h"
#include "application.h"
#include "pageview.h"
#include "pageitem.h"
#include "sliderspinboxwidget.h"
#include "textitem.h"
#include "pagescene.h"
#include "colorstylewidget.h"

void TextAttributionRegister::registe()
{
    supWeightStyleList << "Regular" << "Black" << "DemiBold" << "Bold" << "Medium" << "Light" << "ExtraLight";
    _defaultFontSizeSet << 8 << 10 << 12 << 14 << 16 << 18 << 24 << 36 << 48 << 60 << 72 << 100;

    //1.安装文字颜色设置控件
    m_fontColorAttri = new ColorStyleWidget(drawBoard());
    m_fontColorAttri->setAttribution(EFontColor);
    m_fontColorAttri->setTitleText(tr("Color"));
    m_fontColorAttri->setColorFill(0);
    drawBoard()->attributionManager()->installComAttributeWgt(m_fontColorAttri->attribution(), m_fontColorAttri, QVariant());

    connect(m_fontColorAttri, &ColorStyleWidget::colorChanged, this, [ = ](const QColor & color, int phase) {
        drawBoard()->setDrawAttribution(m_fontColorAttri->attribution(), color, phase);
    });

    connect(m_fontColorAttri->lineEdit(), &DLineEdit::returnPressed, this, [ = ] {
        drawBoard()->currentPage()->view()->setFocus();
    });

    //2.安装字体字重设置控件
    initFontWeightWidget();

    //3.安装字体族设置控件
    initFontFamilyWidget(m_fontHeavy);

    //4.安装字体大小设置控件
    initFontFontSizeWidget();
}

void TextAttributionRegister::initFontFamilyWidget(QComboBox *fontHeavy)
{
    auto attriMangerWgt = drawBoard()->attributionManager();

    //文字字体名设置控件
    m_fontAttri = new ComboBoxSettingWgt(tr("Font"), drawBoard());
    m_fontAttri->setAttribution(EFontFamily);
    auto fontComboBox = new QFontComboBox(m_fontAttri);
    fontComboBox->setFocusPolicy(Qt::StrongFocus);
    setWgtAccesibleName(fontComboBox, "Text font family comboBox");
    fontComboBox->setFocusPolicy(Qt::NoFocus);
    fontComboBox->setEditable(true);
    fontComboBox->lineEdit()->setReadOnly(true);
    m_fontAttri->setComboBox(fontComboBox);

    // 设置默认的字体类型为思源宋黑体，没有该字体则选择系统第一个默认字体
    QFontDatabase fontbase;
    QString sourceHumFont = QObject::tr("Source Han Sans CN");
    if (!fontbase.families().contains(sourceHumFont)) {
        // 修改为系统配置的字体
        sourceHumFont = qApp->font().family();
    }
    drawBoard()->attributionManager()->installComAttributeWgt(EFontFamily, m_fontAttri, sourceHumFont);

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

    connect(attriMangerWgt->helper(), &AttributionManagerHelper::updateWgt, m_fontAttri, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == m_fontAttri) {
            QSignalBlocker blocker(fontComboBox);
            if (var.isValid()) {
                QString string = var.toString();
                if (!string.isEmpty()) {
                    QSignalBlocker FWeightblocker(fontHeavy);
                    reInitFontWeightComboxItems(string, fontHeavy);

                    // 由于需要显示 "— —" ，设置 fontComboBox setEditable(true)
                    // setCurrentText() 会有单独判断，导致实际字体不同步，所以手动查找设置字体
                    int index = fontComboBox->findText(string);
                    if (-1 != index) {
                        fontComboBox->setCurrentIndex(index);
                    }
                    return;
                }
            }

            // 其它情况，直接设置文本
            fontComboBox->setCurrentText(QString("— —"));
        }
    });

    m_fontComBox = fontComboBox;
    m_fontComBox->view()->installEventFilter(this);

    QSignalBlocker blocker(fontHeavy);
    reInitFontWeightComboxItems(sourceHumFont, fontHeavy);
}

void TextAttributionRegister::initFontWeightWidget()
{
    auto attriMangerWgt = drawBoard()->attributionManager();
    //文字字重设置控件
    m_fontWeightAttri = new ComboBoxSettingWgt("", drawBoard());
    m_fontWeightAttri->setAttribution(EFontWeightStyle);
    auto ftStyleComboBox = new QComboBox(drawBoard());
    setWgtAccesibleName(ftStyleComboBox, "Text font style comboBox");
    ftStyleComboBox->setFocusPolicy(Qt::NoFocus);
    ftStyleComboBox->setMinimumWidth(130);
    ftStyleComboBox->setEditable(true);
    ftStyleComboBox->lineEdit()->setReadOnly(true);
    ftStyleComboBox->addItems(supWeightStyleList);
    m_fontWeightAttri->setComboBox(ftStyleComboBox);

    connect(ftStyleComboBox, &QComboBox::currentTextChanged, m_fontWeightAttri, [ = ](const QString & style) {
        drawBoard()->setDrawAttribution(EFontWeightStyle, style, EChanged, false);
    });
    connect(attriMangerWgt->helper(), &AttributionManagerHelper::updateWgt, m_fontWeightAttri, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == m_fontWeightAttri) {
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
    drawBoard()->attributionManager()->installComAttributeWgt(EFontWeightStyle, m_fontWeightAttri, supWeightStyleList.first());

    m_fontHeavy = ftStyleComboBox;

    m_fontWeightAttri->installEventFilter(this);
    m_fontHeavy->view()->installEventFilter(this);
}

void TextAttributionRegister::reInitFontWeightComboxItems(const QString &family, QComboBox *fontHeavy)
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

void TextAttributionRegister::initFontFontSizeWidget()
{
    auto attriMangerWgt = drawBoard()->attributionManager();

    //文字字体大小设置控件
    m_fontSizeAttri = new SliderSpinBoxWidget(EFontSize, SliderSpinBoxWidget::EIntegerStyle, drawBoard());
    m_fontSizeAttri->setTitle(tr("Font size"));
    m_fontSizeAttri->setRange(8, 500);
    connect(m_fontSizeAttri, &SliderSpinBoxWidget::sigValueChanged, this, [ = ](int value, EChangedPhase phase) {
        drawBoard()->setDrawAttribution(EFontSize, value, phase);
        //调节字体大小，需要激活editor框
        if (1 == drawBoard()->currentPage()->scene()->selectedPageItems().size()) {
            TextItem *Textitem = static_cast<TextItem *>(drawBoard()->currentPage()->scene()->selectedPageItems().first());
            Textitem->setEditBoxActive();
        }
        if (phase == EChangedFinished || phase == EChanged) {
            QMetaObject::invokeMethod(this, [ = ] {
                m_fontSizeAttri->clearFocus();
                drawBoard()->currentPage()->view()->setFocus();
            }, Qt::QueuedConnection);
        }
    });

    connect(m_fontSizeAttri->lineEdit(), &QLineEdit::editingFinished, this, [ = ] {
        m_fontSizeAttri->setFocus(Qt::NoFocusReason);
        drawBoard()->currentPage()->view()->setFocus();
    });

    connect(attriMangerWgt->helper(), &AttributionManagerHelper::updateWgt, m_fontSizeAttri, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == m_fontSizeAttri) {
            //QSignalBlocker blocker(ftSizeComboBox);
            QString text = (!var.isValid() || var.toInt() == 0) ?  QStringLiteral("— —") : QString("%1px").arg(var.toInt());
            //ftSizeComboBox->setCurrentText(text);
            //修复两个标签页中文本字号一起改变
            _currenFontSize = var.toInt();
            m_fontSizeAttri->setVar(var);
        }
    });
    drawBoard()->attributionManager()->installComAttributeWgt(EFontSize, m_fontSizeAttri, _currenFontSize);

}

bool TextAttributionRegister::isTextEnableUndoThisTime(bool considerRecorderEmpty)
{
    if (drawBoard()->currentPage() == nullptr)
        return false;

    //如果当前是激活的状态(意味着文字是处于编辑状态),那么文字外部撤销还原栈不用进行数据收集
    auto curView = drawBoard()->currentPage()->view();
    auto curActiveBzItem = curView->activeProxDrawItem();
    if (curActiveBzItem != nullptr) {
        if (curActiveBzItem->type() == TextType) {
            return false;
        }
    }
//    if (considerRecorderEmpty)
//        return CUndoRedoCommand::isRecordEmpty();

    return true;
}

QList<QWidget *> TextAttributionRegister::getAttriWidgets()
{
    return QList<QWidget *>() << m_fontColorAttri << m_fontAttri << m_fontWeightAttri << m_fontSizeAttri;
}

bool TextAttributionRegister::eventFilter(QObject *o, QEvent *event)
{
    //当属性没有显示收到此事件， combobox->view()没有初始化，会访问到野外指针
    if (event->type() == QEvent::Destroy) {
        return  true;
    }

    if (o == m_fontComBox->view()) {
        if (event->type() == QEvent::Show) {
            if (o == m_fontComBox->view()) {
                _activePackup = false;
                _fontViewShowOut = true;
                cachedItemsFontFamily();
                _cachedFontFamily = drawBoard()->pageAttriVariant(drawBoard()->currentPage(), EFontFamily).toString();
            }

        } else if (event->type() == QEvent::Hide) {
            if (o == m_fontComBox->view()) {
                _fontViewShowOut = false;
                QMetaObject::invokeMethod(this, [ = ]() {
                    // _activePackup值为false控件预览，值为true重新赋值
                    if (!_activePackup) {
                        drawBoard()->setDrawAttribution(EFontFamily, _cachedFontFamily, EChangedAbandon, false);
                        // 还原
                        this->restoreItemsFontFamily();
                        // 手动恢复各图元的字体
                        for (auto it = _cachedItemsFontFamily.begin(); it != _cachedItemsFontFamily.end(); ++it) {
                            it.key()->setAttributionVar(EFontFamily, it.value().fontFamily, EChangedAbandon);
                        }
                    } else {
                        drawBoard()->setDrawAttribution(EFontFamily, m_fontComBox->currentText(), EChangedFinished, false);
                        reInitFontWeightComboxItems(m_fontComBox->currentText(), m_fontHeavy);
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
                return true;
            }

            if (Qt::Key_Down == keyEvent->key() || Qt::Key_PageDown == keyEvent->key()) {
                int size = m_fontSize->currentText().replace("px", "").toInt();
                --size;
                size = qMin(qMax(size, 8), 500);
                m_fontSize->setCurrentText(QString("%1px").arg(size));
                return true;
            }
        }
    } else if (m_fontHeavy->parentWidget() == o) {
        if (QEvent::ParentChange == event->type()) {
//            ComBoxSettingWgt *comb = qobject_cast<ComBoxSettingWgt *>(m_fontHeavy->parentWidget());

//            if (comb->parentWidget() != nullptr) {
//                if (comb->parentWidget()->isWindow()) {
//                    comb->setText(tr("Weight"));
//                } else {
//                    comb->setText("");
//                }
//            }
        }
    }

    return QObject::eventFilter(o, event);
}

void TextAttributionRegister::resetItemsFontFamily()
{
    _cachedItemsFontFamily.clear();
}

void TextAttributionRegister::cachedItemsFontFamily()
{
    resetItemsFontFamily();
    if (drawBoard()->currentPage() != nullptr) {
        auto bzItems =  drawBoard()->currentPage()->scene()->selectedPageItems();
        foreach (auto p, bzItems) {
            if (p->type() == TextType) {
                auto textItem = static_cast<TextItem *>(p);
                Font ft;
                ft.FontWeight = textItem->fontStyle();
                ft.fontFamily = textItem->fontFamily();
                _cachedItemsFontFamily.insert(textItem, ft);
                //textItem->beginPreview();
            }
        }
    }
}

void TextAttributionRegister::restoreItemsFontFamily()
{
    for (auto it = _cachedItemsFontFamily.begin(); it != _cachedItemsFontFamily.end(); ++it) {
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

extern QHash<QByteArray, QFont> *qt_app_fonts_hash();
QStyleOptionMenuItem QComboxMenuDelegate::getStyleOption(const QStyleOptionViewItem &option, const QModelIndex &index) const
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
