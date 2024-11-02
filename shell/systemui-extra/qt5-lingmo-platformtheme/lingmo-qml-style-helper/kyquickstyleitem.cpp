/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#include "kyquickstyleitem.h"

#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QApplication>
#include <QSGNinePatchNode>
#include <QPainter>
#include <QPixmapCache>
#include <QStringBuilder>
#include <QQuickWindow>

#include <KSharedConfig>
#include <KConfigGroup>
#include <QGSettings/QGSettings>
#include <QApplication>


QStyle *KyQuickStyleItem::s_style = nullptr;

QStyle *KyQuickStyleItem::style()
{
    auto style = qApp->style();
    return style ? style : s_style;
}

KyQuickStyleItem::KyQuickStyleItem(QQuickItem *parent)
    : QQuickItem(parent),
    m_styleoption(nullptr),
    m_itemType(Undefined),
    m_sunken(false),
    m_raised(false),
    m_active(true),
    m_selected(false),
    m_focus(false),
    m_hover(false),
    m_on(false),
    m_horizontal(true),
    m_transient(false),
    m_sharedWidget(false),
    m_minimum(0),
    m_maximum(100),
    m_value(0),
    m_step(0),
    m_paintMargins(0),
    m_contentWidth(0),
    m_contentHeight(0),
    m_textureWidth(0),
    m_textureHeight(0),
    m_lastFocusReason(Qt::NoFocusReason)
{
    // There is no styleChanged signal and QApplication sends QEvent::StyleChange only to all QWidgets
    if (qApp->style()) {
        connect(qApp->style(), &QObject::destroyed, this, &KyQuickStyleItem::styleChanged);
    } else {
        KSharedConfig::Ptr kdeglobals = KSharedConfig::openConfig();
        KConfigGroup cg(kdeglobals, "KDE");
        auto style = s_style;
        s_style = QStyleFactory::create(cg.readEntry("widgetStyle", QStringLiteral("Fusion")));
        if (style) {
            delete style;
        }
    }

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings* styleSettings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(styleSettings, &QGSettings::changed, this, [&](const QString &key){
            if (key == "systemFontSize" || key == "systemFont") {
                emit fontChanged();
                updatePolish();
            }
            if (key == "iconThemeName"){
                polish();
            }
        });
    }

    m_font = qApp->font();
    setFlag(QQuickItem::ItemHasContents, true);
    setSmooth(false);

    connect(this, &KyQuickStyleItem::visibleChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::widthChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::heightChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::enabledChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::infoChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::onChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::selectedChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::activeChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::textChanged, this, &KyQuickStyleItem::updateSizeHint);
    connect(this, &KyQuickStyleItem::textChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::activeChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::raisedChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::sunkenChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::hoverChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::maximumChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::minimumChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::valueChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::horizontalChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::transientChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::activeControlChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::hasFocusChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::activeControlChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::hintChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::propertiesChanged, this, &KyQuickStyleItem::updateSizeHint);
    connect(this, &KyQuickStyleItem::propertiesChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::elementTypeChanged, this, &KyQuickStyleItem::updateItem);
    connect(this, &KyQuickStyleItem::contentWidthChanged, this, &KyQuickStyleItem::updateSizeHint);
    connect(this, &KyQuickStyleItem::contentHeightChanged, this, &KyQuickStyleItem::updateSizeHint);
    connect(this, &KyQuickStyleItem::widthChanged, this, &KyQuickStyleItem::updateRect);
    connect(this, &KyQuickStyleItem::heightChanged, this, &KyQuickStyleItem::updateRect);

    connect(this, &KyQuickStyleItem::heightChanged, this, &KyQuickStyleItem::updateBaselineOffset);
    connect(this, &KyQuickStyleItem::contentHeightChanged, this, &KyQuickStyleItem::updateBaselineOffset);

    connect(qApp, &QApplication::fontChanged, this, &KyQuickStyleItem::updateSizeHint, Qt::QueuedConnection);
}

KyQuickStyleItem::~KyQuickStyleItem()
{
    if (const QStyleOptionButton *aux = qstyleoption_cast<const QStyleOptionButton*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionViewItem *aux = qstyleoption_cast<const QStyleOptionViewItem*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionHeader *aux = qstyleoption_cast<const QStyleOptionHeader*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionToolButton *aux = qstyleoption_cast<const QStyleOptionToolButton*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionToolBar *aux = qstyleoption_cast<const QStyleOptionToolBar*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionTab *aux = qstyleoption_cast<const QStyleOptionTab*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionFrame *aux = qstyleoption_cast<const QStyleOptionFrame*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionFocusRect *aux = qstyleoption_cast<const QStyleOptionFocusRect*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionTabWidgetFrame *aux = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionMenuItem *aux = qstyleoption_cast<const QStyleOptionMenuItem*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionComboBox *aux = qstyleoption_cast<const QStyleOptionComboBox*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionSpinBox *aux = qstyleoption_cast<const QStyleOptionSpinBox*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionSlider *aux = qstyleoption_cast<const QStyleOptionSlider*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionProgressBar *aux = qstyleoption_cast<const QStyleOptionProgressBar*>(m_styleoption))
        delete aux;
    else if (const QStyleOptionGroupBox *aux = qstyleoption_cast<const QStyleOptionGroupBox*>(m_styleoption))
        delete aux;
    else
        delete m_styleoption;

    m_styleoption = nullptr;
}

void KyQuickStyleItem::initStyleOption()
{
    if (m_styleoption)
        m_styleoption->state = {};

    QString sizeHint = m_hints.value(QStringLiteral("size")).toString();

    bool needsResolvePalette = true;
    update();

    switch (m_itemType) {
    case Button: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionButton();

        QStyleOptionButton *opt = qstyleoption_cast<QStyleOptionButton*>(m_styleoption);
        opt->text = text();

        const QVariant icon = m_properties[QStringLiteral("icon")];
        if (icon.canConvert<QIcon>()) {
            opt->icon = icon.value<QIcon>();
        } else if (icon.canConvert<QUrl>() && icon.value<QUrl>().isLocalFile()) {
            opt->icon = QIcon(icon.value<QUrl>().toLocalFile());
        } else if (icon.canConvert<QString>()) {
            opt->icon = QIcon::fromTheme(icon.value<QString>());
        }
        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(),
                              m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ButtonIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;
        opt->features = activeControl() == QLatin1String("default") ?
                    QStyleOptionButton::DefaultButton :
                    QStyleOptionButton::None;
        if (m_properties[QStringLiteral("flat")].toBool()) {
            opt->features |= QStyleOptionButton::Flat;
        }
        const QFont font = qApp->font("QPushButton");
        opt->fontMetrics = QFontMetrics(font);
        QObject * menu = m_properties[QStringLiteral("menu")].value<QObject *>();
        if (menu) {
            opt->features |= QStyleOptionButton::HasMenu;
        }
    }
        break;
    case ItemRow: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionViewItem();

        QStyleOptionViewItem *opt = qstyleoption_cast<QStyleOptionViewItem*>(m_styleoption);
        opt->features = {};
        if (activeControl() == QLatin1String("alternate"))
            opt->features |= QStyleOptionViewItem::Alternate;
    }
        break;

    case Splitter: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOption;
        }
    }
        break;

    case Item: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionViewItem();
        }
        QStyleOptionViewItem *opt = qstyleoption_cast<QStyleOptionViewItem*>(m_styleoption);
        opt->features = QStyleOptionViewItem::HasDisplay;
        opt->text = text();
        opt->textElideMode = Qt::ElideRight;
        opt->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        opt->decorationAlignment = Qt::AlignCenter;
        resolvePalette();
        needsResolvePalette = false;
        QPalette pal = m_styleoption->palette;
        pal.setBrush(QPalette::Base, Qt::NoBrush);
        m_styleoption->palette = pal;
        const QFont font = qApp->font("QAbstractItemView");
        opt->font = font;
        opt->fontMetrics = QFontMetrics(font);
        break;
    }
    case ItemBranchIndicator: {
        if (!m_styleoption)
            m_styleoption = new QStyleOption;

        m_styleoption->state = QStyle::State_Item; // We don't want to fully support Win 95
        if (m_properties.value(QStringLiteral("hasChildren")).toBool())
            m_styleoption->state |= QStyle::State_Children;
        if (m_properties.value(QStringLiteral("hasSibling")).toBool()) // Even this one could go away
            m_styleoption->state |= QStyle::State_Sibling;
        if (m_on)
            m_styleoption->state |= QStyle::State_Open;
    }
        break;
    case Header: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionHeader();

        QStyleOptionHeader *opt = qstyleoption_cast<QStyleOptionHeader*>(m_styleoption);
        opt->text = text();
        opt->textAlignment = static_cast<Qt::AlignmentFlag>(m_properties.value(QStringLiteral("textalignment")).toInt());
        opt->sortIndicator = activeControl() == QLatin1String("down") ?
                    QStyleOptionHeader::SortDown
                  : activeControl() == QLatin1String("up") ?
                        QStyleOptionHeader::SortUp : QStyleOptionHeader::None;
        QString headerpos = m_properties.value(QStringLiteral("headerpos")).toString();
        if (headerpos == QLatin1String("beginning"))
            opt->position = QStyleOptionHeader::Beginning;
        else if (headerpos == QLatin1String("end"))
            opt->position = QStyleOptionHeader::End;
        else if (headerpos == QLatin1String("only"))
            opt->position = QStyleOptionHeader::OnlyOneSection;
        else
            opt->position = QStyleOptionHeader::Middle;

        const QFont font = qApp->font("QHeaderView");
        opt->fontMetrics = QFontMetrics(font);
    }
        break;
    case ToolButton: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionToolButton();

        QStyleOptionToolButton *opt =
                qstyleoption_cast<QStyleOptionToolButton*>(m_styleoption);
        opt->subControls = QStyle::SC_ToolButton;
        opt->state |= QStyle::State_AutoRaise;
        opt->activeSubControls = QStyle::SC_ToolButton;
        opt->text = text();

        const QVariant icon = m_properties[QStringLiteral("icon")];
        if (icon.canConvert<QIcon>()) {
            opt->icon = icon.value<QIcon>();
        } else if (icon.canConvert<QUrl>() && icon.value<QUrl>().isLocalFile()) {
            opt->icon = QIcon(icon.value<QUrl>().toLocalFile());
        } else if (icon.canConvert<QString>()) {
            opt->icon = QIcon::fromTheme(icon.value<QString>());
        }
        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(),
                              m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ToolBarIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;

        if (m_properties.value(QStringLiteral("menu")).toBool()) {
            opt->subControls |= QStyle::SC_ToolButtonMenu;
            opt->features = QStyleOptionToolButton::HasMenu;
        }

        const int toolButtonStyle = m_properties.value(QStringLiteral("toolButtonStyle")).toInt();

        switch (toolButtonStyle) {
        case Qt::ToolButtonIconOnly:
        case Qt::ToolButtonTextOnly:
        case Qt::ToolButtonTextBesideIcon:
        case Qt::ToolButtonTextUnderIcon:
        case Qt::ToolButtonFollowStyle:
            opt->toolButtonStyle = (Qt::ToolButtonStyle)toolButtonStyle;
            break;
        default:
            opt->toolButtonStyle = Qt::ToolButtonFollowStyle;
        }

        const QFont font = qApp->font("QToolButton");
        opt->font = font;
        opt->fontMetrics = QFontMetrics(font);
    }
        break;
    case ToolBar: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionToolBar();
    }
        break;
    case Tab: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionTab();

        QStyleOptionTab *opt = qstyleoption_cast<QStyleOptionTab*>(m_styleoption);
        opt->text = text();

        const QVariant icon = m_properties[QStringLiteral("icon")];
        if (icon.canConvert<QIcon>()) {
            opt->icon = icon.value<QIcon>();
        } else if (icon.canConvert<QUrl>() && icon.value<QUrl>().isLocalFile()) {
            opt->icon = QIcon(icon.value<QUrl>().toLocalFile());
        } else if (icon.canConvert<QString>()) {
            opt->icon = QIcon::fromTheme(icon.value<QString>());
        }
        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(),
                              m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ButtonIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;

        if (m_properties.value(QStringLiteral("hasFrame")).toBool())
            opt->features |= QStyleOptionTab::HasFrame;

        QString orientation = m_properties.value(QStringLiteral("orientation")).toString();
        QString position = m_properties.value(QStringLiteral("tabpos")).toString();
        QString selectedPosition = m_properties.value(QStringLiteral("selectedpos")).toString();

        opt->shape = orientation == QLatin1String("Bottom") ? QTabBar::RoundedSouth : QTabBar::RoundedNorth;
        if (position == QLatin1String("beginning"))
            opt->position = QStyleOptionTab::Beginning;
        else if (position == QLatin1String("end"))
            opt->position = QStyleOptionTab::End;
        else if (position == QLatin1String("only"))
            opt->position = QStyleOptionTab::OnlyOneTab;
        else
            opt->position = QStyleOptionTab::Middle;

        if (selectedPosition == QLatin1String("next"))
            opt->selectedPosition = QStyleOptionTab::NextIsSelected;
        else if (selectedPosition == QLatin1String("previous"))
            opt->selectedPosition = QStyleOptionTab::PreviousIsSelected;
        else
            opt->selectedPosition = QStyleOptionTab::NotAdjacent;


    } break;

    case Frame: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionFrame();

        QStyleOptionFrame *opt = qstyleoption_cast<QStyleOptionFrame*>(m_styleoption);
        opt->frameShape = QFrame::StyledPanel;
        opt->lineWidth = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
        opt->midLineWidth = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
    }
        break;
    case FocusRect: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionFocusRect();
        // Needed on windows
        m_styleoption->state |= QStyle::State_KeyboardFocusChange;
    }
        break;
    case TabFrame: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionTabWidgetFrame();
        QStyleOptionTabWidgetFrame *opt = qstyleoption_cast<QStyleOptionTabWidgetFrame*>(m_styleoption);

        opt->selectedTabRect = m_properties[QStringLiteral("selectedTabRect")].toRect();
        opt->shape = m_properties[QStringLiteral("orientation")] == Qt::BottomEdge ? QTabBar::RoundedSouth : QTabBar::RoundedNorth;
        if (minimum())
            opt->selectedTabRect = QRect(value(), 0, minimum(), height());
        opt->tabBarSize = QSize(minimum() , height());
        // oxygen style needs this hack
        opt->leftCornerWidgetSize = QSize(value(), 0);
    }
        break;
    case MenuBar:
        if (!m_styleoption) {
            QStyleOptionMenuItem *menuOpt = new QStyleOptionMenuItem();
            menuOpt->menuItemType = QStyleOptionMenuItem::EmptyArea;
            m_styleoption = menuOpt;
        }

        break;
    case MenuBarItem:
    {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionMenuItem();

        QStyleOptionMenuItem *opt = qstyleoption_cast<QStyleOptionMenuItem*>(m_styleoption);
        opt->text = text();
        opt->menuItemType = QStyleOptionMenuItem::Normal;
        setProperty("_q_showUnderlined", m_hints[QStringLiteral("showUnderlined")].toBool());

        const QFont font = qApp->font("QMenuBar");
        opt->font = font;
        opt->fontMetrics = QFontMetrics(font);
        m_font = opt->font;
    }
        break;
    case Menu: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionMenuItem();
    }
        break;
    case MenuItem:
    case ComboBoxItem:
    {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionMenuItem();

        QStyleOptionMenuItem *opt = qstyleoption_cast<QStyleOptionMenuItem*>(m_styleoption);
        // For GTK style. See below, in setElementType()
        setProperty("_q_isComboBoxPopupItem", m_itemType == ComboBoxItem);

        KyQuickStyleItem::MenuItemType type =
                static_cast<KyQuickStyleItem::MenuItemType>(m_properties[QStringLiteral("type")].toInt());
        if (type == KyQuickStyleItem::ScrollIndicatorType) {
            int scrollerDirection = m_properties[QStringLiteral("scrollerDirection")].toInt();
            opt->menuItemType = QStyleOptionMenuItem::Scroller;
            opt->state |= scrollerDirection == Qt::UpArrow ?
                        QStyle::State_UpArrow : QStyle::State_DownArrow;
        } else if (type == KyQuickStyleItem::SeparatorType) {
            opt->menuItemType = QStyleOptionMenuItem::Separator;
        } else {
            opt->text = text();

            if (type == KyQuickStyleItem::MenuType) {
                opt->menuItemType = QStyleOptionMenuItem::SubMenu;
            } else {
                opt->menuItemType = QStyleOptionMenuItem::Normal;

                QString shortcut = m_properties[QStringLiteral("shortcut")].toString();
                if (!shortcut.isEmpty()) {
                    opt->text += QLatin1Char('\t') + shortcut;
                    opt->tabWidth = qMax(opt->tabWidth, qRound(textWidth(shortcut)));
                }

                if (m_properties[QStringLiteral("checkable")].toBool()) {
                    opt->checked = on();
                    QVariant exclusive = m_properties[QStringLiteral("exclusive")];
                    opt->checkType = exclusive.toBool() ? QStyleOptionMenuItem::Exclusive :
                                                          QStyleOptionMenuItem::NonExclusive;
                }
            }
            if (m_properties[QStringLiteral("icon")].canConvert<QIcon>())
                opt->icon = m_properties[QStringLiteral("icon")].value<QIcon>();
            setProperty("_q_showUnderlined", m_hints[QStringLiteral("showUnderlined")].toBool());

            const QFont font = qApp->font(m_itemType == ComboBoxItem ?"QComboMenuItem" : "QMenu");
            opt->font = font;
            opt->fontMetrics = QFontMetrics(font);
            m_font = opt->font;
        }
    }
        break;
    case CheckBox:
    case RadioButton:
    {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionButton();

        QStyleOptionButton *opt = qstyleoption_cast<QStyleOptionButton*>(m_styleoption);
        if (!on())
            opt->state |= QStyle::State_Off;
        if (m_properties.value(QStringLiteral("partiallyChecked")).toBool())
            opt->state |= QStyle::State_NoChange;
        opt->text = text();
    }
        break;
    case Edit: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionFrame();

        QStyleOptionFrame *opt = qstyleoption_cast<QStyleOptionFrame*>(m_styleoption);
        opt->lineWidth = qMax(1, KyQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr)); //this must be non zero
    }
        break;
    case ComboBox :{
        if (!m_styleoption)
            m_styleoption = new QStyleOptionComboBox();

        QStyleOptionComboBox *opt = qstyleoption_cast<QStyleOptionComboBox*>(m_styleoption);

        const QFont font = qApp->font("QPushButton"); //DAVE - QQC1 code does this, but if you look at QComboBox this doesn't make sense
        opt->fontMetrics = QFontMetrics(font);
        opt->currentText = text();
        opt->editable = m_properties[QStringLiteral("editable")].toBool();

        const QVariant icon = m_properties[QStringLiteral("currentIcon")];
        if (icon.canConvert<QIcon>()) {
            opt->currentIcon = icon.value<QIcon>();
        } else if (icon.canConvert<QString>()) {
            opt->currentIcon = QIcon::fromTheme(icon.value<QString>());
        }
        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(),
                              m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ButtonIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;
    }
        break;
    case SpinBox: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionSpinBox();

        QStyleOptionSpinBox *opt = qstyleoption_cast<QStyleOptionSpinBox*>(m_styleoption);
        opt->frame = true;
        opt->subControls = QStyle::SC_SpinBoxFrame | QStyle::SC_SpinBoxEditField;
        if (value() & 0x1)
            opt->activeSubControls = QStyle::SC_SpinBoxUp;
        else if (value() & (1<<1))
            opt->activeSubControls = QStyle::SC_SpinBoxDown;
        opt->subControls = QStyle::SC_All;
        opt->stepEnabled = {};
        if (value() & (1<<2))
            opt->stepEnabled |= QAbstractSpinBox::StepUpEnabled;
        if (value() & (1<<3))
            opt->stepEnabled |= QAbstractSpinBox::StepDownEnabled;
    }
        break;
    case Slider:
    case Dial:
    {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionSlider();

        QStyleOptionSlider *opt = qstyleoption_cast<QStyleOptionSlider*>(m_styleoption);
        opt->orientation = horizontal() ? Qt::Horizontal : Qt::Vertical;
        opt->upsideDown = !horizontal();

        int min = minimum();
        int max = std::max(min, maximum());

        opt->minimum = min;
        opt->maximum = max;
        opt->sliderPosition = value();
        opt->singleStep = step();

        if (opt->singleStep) {
            qreal numOfSteps = (opt->maximum - opt->minimum) / opt->singleStep;
            // at least 5 pixels between tick marks
            qreal extent = horizontal() ? width() : height();
            if (numOfSteps && (extent / numOfSteps < 5))
                opt->tickInterval = qRound((5 * numOfSteps / extent) + 0.5) * step();
            else
                opt->tickInterval = opt->singleStep;

        } else // default Qt-components implementation
            opt->tickInterval = opt->maximum != opt->minimum ? 1200 / (opt->maximum - opt->minimum) : 0;

        opt->sliderValue = value();
        opt->subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
        opt->tickPosition = activeControl() == QLatin1String("ticks") ?
                    QSlider::TicksBelow : QSlider::NoTicks;
        if (opt->tickPosition != QSlider::NoTicks)
            opt->subControls |= QStyle::SC_SliderTickmarks;

        opt->activeSubControls = QStyle::SC_SliderHandle;
    }
        break;
    case ProgressBar: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionProgressBar();

        QStyleOptionProgressBar *opt = qstyleoption_cast<QStyleOptionProgressBar*>(m_styleoption);
        opt->orientation = horizontal() ? Qt::Horizontal : Qt::Vertical;
        opt->minimum = qMax(0, minimum());
        opt->maximum = qMax(0, maximum());
        opt->progress = value();
    }
        break;
    case GroupBox: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionGroupBox();

        QStyleOptionGroupBox *opt = qstyleoption_cast<QStyleOptionGroupBox*>(m_styleoption);
        opt->text = text();
        opt->lineWidth = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
        opt->subControls = QStyle::SC_GroupBoxLabel;
        opt->features = {};
        if (m_properties[QStringLiteral("sunken")].toBool()) { // Qt draws an ugly line here so I ignore it
            opt->subControls |= QStyle::SC_GroupBoxFrame;
        } else {
            opt->features |= QStyleOptionFrame::Flat;
        }
        if (m_properties[QStringLiteral("checkable")].toBool())
            opt->subControls |= QStyle::SC_GroupBoxCheckBox;

    }
        break;
    case ScrollBar: {
        if (!m_styleoption)
            m_styleoption = new QStyleOptionSlider();

        QStyleOptionSlider *opt = qstyleoption_cast<QStyleOptionSlider*>(m_styleoption);
        opt->minimum = qMax(0, minimum());
        opt->maximum = qMax(0, maximum());
        opt->pageStep = qMax(0, int(horizontal() ? width() : height()));
        opt->orientation = horizontal() ? Qt::Horizontal : Qt::Vertical;
        opt->sliderPosition = value();
        opt->sliderValue = value();
        opt->activeSubControls = (activeControl() == QLatin1String("up"))
                ? QStyle::SC_ScrollBarSubLine : (activeControl() == QLatin1String("down")) ?
                      QStyle::SC_ScrollBarAddLine :
                  (activeControl() == QLatin1String("handle")) ?
                      QStyle::SC_ScrollBarSlider : hover() ? QStyle::SC_ScrollBarGroove : QStyle::SC_None;
        if (raised())
            opt->state |= QStyle::State_On;

        opt->sliderValue = value();
        opt->subControls = QStyle::SC_All;

        setTransient(KyQuickStyleItem::style()->styleHint(QStyle::SH_ScrollBar_Transient, m_styleoption));
        break;
    }
    default:
        break;
    }

    if (!m_styleoption)
        m_styleoption = new QStyleOption();

    if (needsResolvePalette)
        resolvePalette();

    m_styleoption->styleObject = this;
    m_styleoption->direction = qApp->layoutDirection();

    int w = m_textureWidth > 0 ? m_textureWidth : width();
    int h = m_textureHeight > 0 ? m_textureHeight : height();

    m_styleoption->rect = QRect(m_paintMargins, 0, w - 2* m_paintMargins, h);

    if (isEnabled()) {
        m_styleoption->state |= QStyle::State_Enabled;
        m_styleoption->palette.setCurrentColorGroup(QPalette::Active);
    } else {
        m_styleoption->palette.setCurrentColorGroup(QPalette::Disabled);
    }
    if (m_active)
        m_styleoption->state |= QStyle::State_Active;
    else
        m_styleoption->palette.setCurrentColorGroup(QPalette::Inactive);
    if (m_sunken)
        m_styleoption->state |= QStyle::State_Sunken;
    if (m_raised)
        m_styleoption->state |= QStyle::State_Raised;
    if (m_selected)
        m_styleoption->state |= QStyle::State_Selected;
    if (m_focus)
        m_styleoption->state |= QStyle::State_HasFocus;
    if (m_on)
        m_styleoption->state |= QStyle::State_On;
    if (m_hover)
        m_styleoption->state |= QStyle::State_MouseOver;
    if (m_horizontal)
        m_styleoption->state |= QStyle::State_Horizontal;

    // some styles don't draw a focus rectangle if
    // QStyle::State_KeyboardFocusChange is not set
    if (window()) {
         if (m_lastFocusReason == Qt::TabFocusReason || m_lastFocusReason == Qt::BacktabFocusReason) {
             m_styleoption->state |= QStyle::State_KeyboardFocusChange;
         }
    }

    if (sizeHint == QLatin1String("mini")) {
        m_styleoption->state |= QStyle::State_Mini;
    } else if (sizeHint == QLatin1String("small")) {
        m_styleoption->state |= QStyle::State_Small;
    }

}

const char* KyQuickStyleItem::classNameForItem() const
{
    switch(m_itemType) {
    case Button:
        return "QPushButton";
    case RadioButton:
        return "QRadioButton";
    case CheckBox:
        return "QCheckBox";
    case ComboBox:
        return "QComboBox";
    case ComboBoxItem:
        return "QComboMenuItem";
    case ToolBar:
        return "";
    case ToolButton:
        return "QToolButton";
    case Tab:
        return "QTabButton";
    case TabFrame:
        return "QTabBar";
    case Edit:
        return "QTextEdit";
    case GroupBox:
        return "QGroupBox";
    case Header:
        return "QHeaderView";
    case Item:
    case ItemRow:
        return "QAbstractItemView";
    case Menu:
    case MenuItem:
        return "QMenu";
    case MenuBar:
    case MenuBarItem:
        return "QMenuBar";
    default:
        return "";
    }
    Q_UNREACHABLE();
}

void KyQuickStyleItem::resolvePalette()
{
    if (QCoreApplication::testAttribute(Qt::AA_SetPalette))
        return;

    const QVariant controlPalette = m_control ? m_control->property("palette") : QVariant();
    if (controlPalette.isValid()) {
        m_styleoption->palette = controlPalette.value<QPalette>();
    } else {
//        m_styleoption->palette = m_theme->palette();
    }
}

int KyQuickStyleItem::leftPadding() const
{
    switch (m_itemType) {
    case Frame: {
        const QRect cr = KyQuickStyleItem::style()->subElementRect(QStyle::SE_ShapedFrameContents, m_styleoption);
        return cr.left() - m_styleoption->rect.left();
    }
    default:
        return 0;
    }
}

int KyQuickStyleItem::topPadding() const
{
    switch (m_itemType) {
    case Frame: {
        const QRect cr = KyQuickStyleItem::style()->subElementRect(QStyle::SE_ShapedFrameContents, m_styleoption);
        return cr.top() - m_styleoption->rect.top();
    }
    default:
        return 0;
    }
}

int KyQuickStyleItem::rightPadding() const
{
    switch (m_itemType) {
    case Frame: {
        const QRect cr = KyQuickStyleItem::style()->subElementRect(QStyle::SE_ShapedFrameContents, m_styleoption);
        return m_styleoption->rect.right() - cr.right();
    }
    default:
        return 0;
    }
}

int KyQuickStyleItem::bottomPadding() const
{
    switch (m_itemType) {
    case Frame: {
        const QRect cr = KyQuickStyleItem::style()->subElementRect(QStyle::SE_ShapedFrameContents, m_styleoption);
        return m_styleoption->rect.bottom() - cr.bottom();
    }
    default:
        return 0;
    }
}

/*
 *   Property style
 *
 *   Returns a simplified style name.
 *
 *   QMacStyle = "mac"
 *   QWindowsXPStyle = "windowsxp"
 *   QFusionStyle = "fusion"
 */

QString KyQuickStyleItem::styleName() const
{
    QString style = QString::fromLatin1(KyQuickStyleItem::style()->metaObject()->className());
    style = style.toLower();
    if (style.startsWith(QLatin1Char('q')))
        style = style.right(style.length() - 1);
    if (style.endsWith(QLatin1String("style")))
        style = style.left(style.length() - 5);
    return style;
}

QString KyQuickStyleItem::hitTest(int px, int py)
{
    QStyle::SubControl subcontrol = QStyle::SC_All;
    switch (m_itemType) {
    case SpinBox :{
        subcontrol = KyQuickStyleItem::style()->hitTestComplexControl(QStyle::CC_SpinBox,
                                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                                          QPoint(px,py), nullptr);
        if (subcontrol == QStyle::SC_SpinBoxUp)
            return QStringLiteral("up");
        else if (subcontrol == QStyle::SC_SpinBoxDown)
            return QStringLiteral("down");
    }
        break;

    case Slider: {
        subcontrol = KyQuickStyleItem::style()->hitTestComplexControl(QStyle::CC_Slider,
                                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                                          QPoint(px,py), nullptr);
        if (subcontrol == QStyle::SC_SliderHandle)
            return QStringLiteral("handle");
    }
        break;

    case ScrollBar: {
        subcontrol = KyQuickStyleItem::style()->hitTestComplexControl(QStyle::CC_ScrollBar,
                                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                                          QPoint(px,py), nullptr);
        switch (subcontrol) {
        case QStyle::SC_ScrollBarSlider:
            return QStringLiteral("handle");

        case QStyle::SC_ScrollBarSubLine:
            return QStringLiteral("up");

        case QStyle::SC_ScrollBarSubPage:
            return QStringLiteral("upPage");

        case QStyle::SC_ScrollBarAddLine:
            return QStringLiteral("down");

        case QStyle::SC_ScrollBarAddPage:
            return QStringLiteral("downPage");

        default:
            break;
        }
    }
        break;

    default:
        break;
    }
    return QStringLiteral("none");
}

QSize KyQuickStyleItem::sizeFromContents(int width, int height)
{
    initStyleOption();

    QSize size;
    switch (m_itemType) {
    case RadioButton:
        size =  KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_RadioButton, m_styleoption, QSize(width,height));
        break;
    case CheckBox:
        size =  KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_CheckBox, m_styleoption, QSize(width,height));
        break;
    case ToolBar:
        size = QSize(200, styleName().contains(QLatin1String("windows")) ? 30 : 42);
        break;
    case ToolButton: {
        QStyleOptionToolButton *btn = qstyleoption_cast<QStyleOptionToolButton*>(m_styleoption);
        int w = 0;
        int h = 0;
        if (btn->toolButtonStyle != Qt::ToolButtonTextOnly) {
            QSize icon = btn->iconSize;
            w = icon.width();
            h = icon.height();
        }
        if (btn->toolButtonStyle != Qt::ToolButtonIconOnly) {
            QSize textSize = btn->fontMetrics.size(Qt::TextShowMnemonic, btn->text);
            textSize.setWidth(textSize.width() + btn->fontMetrics.horizontalAdvance(QLatin1Char(' '))*2);
            if (btn->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                h += 4 + textSize.height();
                if (textSize.width() > w)
                    w = textSize.width();
            } else if (btn->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                w += 4 + textSize.width();
                if (textSize.height() > h)
                    h = textSize.height();
            } else { // TextOnly
                w = textSize.width();
                h = textSize.height();
            }
        }
        btn->rect.setSize(QSize(w, h));
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_ToolButton, m_styleoption, QSize(w, h)); }
        break;
    case Button: {
        QStyleOptionButton *btn = qstyleoption_cast<QStyleOptionButton*>(m_styleoption);

        int contentWidth = btn->fontMetrics.boundingRect(btn->text).width();
        int contentHeight = btn->fontMetrics.height();

        if (!btn->icon.isNull()) {
            //+4 matches a hardcoded value in QStyle and acts as a margin between the icon and the text.
            contentWidth += btn->iconSize.width() + 4;
            contentHeight = qMax(btn->fontMetrics.height(), btn->iconSize.height());
        }

        int newWidth = qMax(width, contentWidth);
        int newHeight = qMax(height, contentHeight);
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_PushButton, m_styleoption, QSize(newWidth, newHeight)); }
        break;
    case ComboBox: {
        QStyleOptionComboBox *btn = qstyleoption_cast<QStyleOptionComboBox*>(m_styleoption);
        int newWidth = qMax(width, btn->fontMetrics.boundingRect(btn->currentText).width());
        int newHeight = qMax(height, btn->fontMetrics.height());
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_ComboBox, m_styleoption, QSize(newWidth, newHeight)); }
        break;
    case Tab: {
        QStyleOptionTab *tab = qstyleoption_cast<QStyleOptionTab*>(m_styleoption);

        int contentWidth = tab->fontMetrics.boundingRect(tab->text).width();
        int contentHeight = tab->fontMetrics.height();

        if (!tab->icon.isNull()) {
            //+4 matches a hardcoded value in QStyle and acts as a margin between the icon and the text.
            contentWidth += tab->iconSize.width() + 4;
            contentHeight = qMax(contentHeight, tab->iconSize.height());
        }

        contentWidth += KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabHSpace, tab);
        contentHeight += KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabVSpace, tab);

        const int newWidth = qMax(width, contentWidth);
        const int newHeight = qMax(height, contentHeight);

        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_TabBarTab, m_styleoption, QSize(newWidth, newHeight));

        break;
    }
    case Slider:
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_Slider, m_styleoption, QSize(width,height));
        break;
    case ProgressBar:
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_ProgressBar, m_styleoption, QSize(width,height));
        break;
    case SpinBox:
    case Edit:
        {
            // We have to create a new style option since we might be calling with a QStyleOptionSpinBox
            QStyleOptionFrame frame;
            //+2 to be consistent with the hardcoded verticalmargin in QLineEdit
            int contentHeight = frame.fontMetrics.height() + 2;

            frame.state = m_styleoption->state | QStyle::State_Sunken;
            frame.lineWidth = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
            frame.rect = m_styleoption->rect;
            frame.styleObject = this;

            size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_LineEdit,
                                                               &frame, QSize(width, qMax(height, contentHeight)).expandedTo(QApplication::globalStrut()));
            if (m_itemType == SpinBox) {
                size.setWidth(KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_SpinBox,
                                m_styleoption, QSize(width + 2, height)).width());
            }
        }
        break;
    case GroupBox: {
            QStyleOptionGroupBox *box = qstyleoption_cast<QStyleOptionGroupBox*>(m_styleoption);
            QFontMetrics metrics(box->fontMetrics);
            int baseWidth = metrics.boundingRect(box->text + QLatin1Char(' ')).width();
            int baseHeight = metrics.height() + m_contentHeight;
            if (box->subControls & QStyle::SC_GroupBoxCheckBox) {
                baseWidth += KyQuickStyleItem::style()->pixelMetric(QStyle::PM_IndicatorWidth);
                baseWidth += KyQuickStyleItem::style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);
                baseHeight = qMax(baseHeight, KyQuickStyleItem::style()->pixelMetric(QStyle::PM_IndicatorHeight));
            }
            size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_GroupBox, m_styleoption, QSize(qMax(baseWidth, m_contentWidth), baseHeight));
        }
        break;
    case Header:
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_HeaderSection, m_styleoption, QSize(width,height));
        break;
    case ItemRow:
    case Item: //fall through
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_ItemViewItem, m_styleoption, QSize(width,height));
        break;
    case MenuBarItem:
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_MenuBarItem, m_styleoption, QSize(width,height));
        break;
    case MenuBar:
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_MenuBar, m_styleoption, QSize(width,height));
        break;
    case Menu:
        size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_Menu, m_styleoption, QSize(width,height));
        break;
    case MenuItem:
    case ComboBoxItem:
        if (static_cast<QStyleOptionMenuItem *>(m_styleoption)->menuItemType == QStyleOptionMenuItem::Scroller) {
            size.setHeight(qMax(QApplication::globalStrut().height(),
                                KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuScrollerHeight, nullptr, nullptr)));
        } else {
            size = KyQuickStyleItem::style()->sizeFromContents(QStyle::CT_MenuItem, m_styleoption, QSize(width,height));
        }
        break;
    default:
        break;
    }
    return size.expandedTo(QSize(m_contentWidth, m_contentHeight));
}

qreal KyQuickStyleItem::baselineOffset()
{
    QRect r;
    bool ceilResult = true; // By default baseline offset rounding is done upwards
    switch (m_itemType) {
    case RadioButton:
        r = KyQuickStyleItem::style()->subElementRect(QStyle::SE_RadioButtonContents, m_styleoption);
        break;
    case Button:
        r = KyQuickStyleItem::style()->subElementRect(QStyle::SE_PushButtonContents, m_styleoption);
        break;
    case CheckBox:
        r = KyQuickStyleItem::style()->subElementRect(QStyle::SE_CheckBoxContents, m_styleoption);
        break;
    case Edit:
        r = KyQuickStyleItem::style()->subElementRect(QStyle::SE_LineEditContents, m_styleoption);
        break;
    case ComboBox:
        if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(m_styleoption)) {
            r = KyQuickStyleItem::style()->subControlRect(QStyle::CC_ComboBox, combo, QStyle::SC_ComboBoxEditField);
            if (styleName() != QLatin1String("mac"))
                r.adjust(0,0,0,1);
        }
        break;
    case SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(m_styleoption)) {
            r = KyQuickStyleItem::style()->subControlRect(QStyle::CC_SpinBox, spinbox, QStyle::SC_SpinBoxEditField);
            ceilResult = false;
        }
        break;
    default:
        break;
    }
    if (r.height() > 0) {
        const QFontMetrics &fm = m_styleoption->fontMetrics;
        int surplus = r.height() - fm.height();
        if ((surplus & 1) && ceilResult)
            surplus++;
        int result = r.top() + surplus/2 + fm.ascent();
        return result;
    }

    return 0.;
}

void KyQuickStyleItem::updateBaselineOffset()
{
    const qreal baseline = baselineOffset();
    if (baseline > 0)
        setBaselineOffset(baseline);
}

void KyQuickStyleItem::setContentWidth(int arg)
{
    if (m_contentWidth != arg) {
        m_contentWidth = arg;
        emit contentWidthChanged(arg);
    }
}

void KyQuickStyleItem::setContentHeight(int arg)
{
    if (m_contentHeight != arg) {
        m_contentHeight = arg;
        emit contentHeightChanged(arg);
    }
}

void KyQuickStyleItem::updateSizeHint()
{
    QSize implicitSize = sizeFromContents(m_contentWidth, m_contentHeight);
    setImplicitSize(implicitSize.width(), implicitSize.height());
}

void KyQuickStyleItem::updateRect()
{
    initStyleOption();
    m_styleoption->rect.setWidth(width());
    m_styleoption->rect.setHeight(height());
}

int KyQuickStyleItem::pixelMetric(const QString &metric)
{
    if (metric == QLatin1String("scrollbarExtent"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr);
    else if (metric == QLatin1String("defaultframewidth"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption);
    else if (metric == QLatin1String("taboverlap"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabOverlap, nullptr);
    else if (metric == QLatin1String("tabbaseoverlap"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, m_styleoption);
    else if (metric == QLatin1String("tabhspace"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabHSpace, nullptr);
    else if (metric == QLatin1String("indicatorwidth"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr);
    else if (metric == QLatin1String("exclusiveindicatorwidth"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, nullptr);
    else if (metric == QLatin1String("checkboxlabelspacing"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr);
    else if (metric == QLatin1String("ratiobuttonlabelspacing"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_RadioButtonLabelSpacing, nullptr);
    else if (metric == QLatin1String("tabvspace"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabVSpace, nullptr);
    else if (metric == QLatin1String("tabbaseheight"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarBaseHeight, nullptr);
    else if (metric == QLatin1String("tabvshift"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabShiftVertical, nullptr);
    else if (metric == QLatin1String("menubarhmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarHMargin, nullptr);
    else if (metric == QLatin1String("menubarvmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr);
    else if (metric == QLatin1String("menubarpanelwidth"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr);
    else if (metric == QLatin1String("menubaritemspacing"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarItemSpacing, nullptr);
    else if (metric == QLatin1String("spacebelowmenubar"))
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, m_styleoption);
    else if (metric == QLatin1String("menuhmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuHMargin, nullptr);
    else if (metric == QLatin1String("menuvmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuVMargin, nullptr);
    else if (metric == QLatin1String("menupanelwidth"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuPanelWidth, nullptr);
    else if (metric == QLatin1String("submenuoverlap"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_SubMenuOverlap, nullptr);
    else if (metric == QLatin1String("splitterwidth"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_SplitterWidth, nullptr);
    else if (metric == QLatin1String("scrollbarspacing"))
        return abs(KyQuickStyleItem::style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, nullptr));
    else if (metric == QLatin1String("treeviewindentation"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_TreeViewIndentation, nullptr);
    else if (metric == QLatin1String("layouthorizontalspacing"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, nullptr);
    else if (metric == QLatin1String("layoutverticalspacing"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing, nullptr);
    else if (metric == QLatin1String("layoutleftmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr);
    else if (metric == QLatin1String("layouttopmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutTopMargin, nullptr);
    else if (metric == QLatin1String("layoutrightmargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr);
    else if (metric == QLatin1String("layoutbottommargin"))
        return KyQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutBottomMargin, nullptr);
    return 0;
}

QVariant KyQuickStyleItem::styleHint(const QString &metric)
{
    initStyleOption();
    if (metric == QLatin1String("comboboxpopup")) {
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_ComboBox_Popup, m_styleoption);
    } else if (metric == QLatin1String("highlightedTextColor")) {
        return m_styleoption->palette.highlightedText().color().name();
    } else if (metric == QLatin1String("textColor")) {
        QPalette pal = m_styleoption->palette;
        pal.setCurrentColorGroup(active()? QPalette::Active : QPalette::Inactive);
        return pal.text().color().name();
    } else if (metric == QLatin1String("focuswidget")) {
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_FocusFrame_AboveWidget);
    } else if (metric == QLatin1String("tabbaralignment")) {
        int result = KyQuickStyleItem::style()->styleHint(QStyle::SH_TabBar_Alignment);
        if (result == Qt::AlignCenter)
            return QStringLiteral("center");
        return QStringLiteral("left");
    } else if (metric == QLatin1String("externalScrollBars")) {
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents);
    } else if (metric == QLatin1String("scrollToClickPosition"))
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition);
    else if (metric == QLatin1String("activateItemOnSingleClick"))
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    else if (metric == QLatin1String("submenupopupdelay"))
        return KyQuickStyleItem::style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, m_styleoption);
    else if (metric == QLatin1String("wheelScrollLines"))
        return qApp->wheelScrollLines();
    return 0;

    // Add SH_Menu_SpaceActivatesItem
}

void KyQuickStyleItem::setHints(const QVariantMap &str)
{
    if (m_hints != str) {
        m_hints = str;
        initStyleOption();
        updateSizeHint();
        if (m_styleoption->state & QStyle::State_Mini) {
            m_font.setPointSize(9.);
            emit fontChanged();
        } else if (m_styleoption->state & QStyle::State_Small) {
            m_font.setPointSize(11.);
            emit fontChanged();
        } else {
            emit hintChanged();
        }
    }
}

void KyQuickStyleItem::resetHints()
{
    m_hints.clear();
}


void KyQuickStyleItem::setElementType(const QString &str)
{
    if (m_type == str)
        return;

    m_type = str;

    emit elementTypeChanged();
    if (m_styleoption) {
        delete m_styleoption;
        m_styleoption = nullptr;
    }

    // Only enable visible if the widget can animate
    if (str == QLatin1String("menu")) {
        m_itemType = Menu;
    } else if (str == QLatin1String("menuitem")) {
        m_itemType = MenuItem;
    } else if (str == QLatin1String("item") || str == QLatin1String("itemrow") || str == QLatin1String("header")) {
        if (str == QLatin1String("header")) {
            m_itemType = Header;
        } else {
            m_itemType = str == QLatin1String("item") ? Item : ItemRow;
        }
    } else if (str == QLatin1String("itembranchindicator")) {
        m_itemType = ItemBranchIndicator;
    } else if (str == QLatin1String("groupbox")) {
        m_itemType = GroupBox;
    } else if (str == QLatin1String("tab")) {
        m_itemType = Tab;
    } else if (str == QLatin1String("tabframe")) {
        m_itemType = TabFrame;
    } else if (str == QLatin1String("comboboxitem"))  {
        // Gtk uses qobject cast, hence we need to separate this from menuitem
        // On mac, we temporarily use the menu item because it has more accurate
        // palette.
        m_itemType = ComboBoxItem;
    } else if (str == QLatin1String("toolbar")) {
        m_itemType = ToolBar;
    } else if (str == QLatin1String("toolbutton")) {
        m_itemType = ToolButton;
    } else if (str == QLatin1String("slider")) {
        m_itemType = Slider;
    } else if (str == QLatin1String("frame")) {
        m_itemType = Frame;
    } else if (str == QLatin1String("combobox")) {
        m_itemType = ComboBox;
    } else if (str == QLatin1String("splitter")) {
        m_itemType = Splitter;
    } else if (str == QLatin1String("progressbar")) {
        m_itemType = ProgressBar;
    } else if (str == QLatin1String("button")) {
        m_itemType = Button;
    } else if (str == QLatin1String("checkbox")) {
        m_itemType = CheckBox;
    } else if (str == QLatin1String("radiobutton")) {
        m_itemType = RadioButton;
    } else if (str == QLatin1String("edit")) {
        m_itemType = Edit;
    } else if (str == QLatin1String("spinbox")) {
        m_itemType = SpinBox;
    } else if (str == QLatin1String("scrollbar")) {
        m_itemType = ScrollBar;
    } else if (str == QLatin1String("widget")) {
        m_itemType = Widget;
    } else if (str == QLatin1String("focusframe")) {
        m_itemType = FocusFrame;
    } else if (str == QLatin1String("focusrect")) {
        m_itemType = FocusRect;
    } else if (str == QLatin1String("dial")) {
        m_itemType = Dial;
    } else if (str == QLatin1String("statusbar")) {
        m_itemType = StatusBar;
    } else if (str == QLatin1String("machelpbutton")) {
        m_itemType = MacHelpButton;
    } else if (str == QLatin1String("scrollareacorner")) {
        m_itemType = ScrollAreaCorner;
    } else if (str == QLatin1String("menubar")) {
        m_itemType = MenuBar;
    } else if (str == QLatin1String("menubaritem")) {
        m_itemType = MenuBarItem;
    } else {
        m_itemType = Undefined;
    }
    emit leftPaddingChanged();
    emit rightPaddingChanged();
    emit topPaddingChanged();
    emit bottomPaddingChanged();
    updateSizeHint();
}

QRectF KyQuickStyleItem::subControlRect(const QString &subcontrolString)
{
    QStyle::SubControl subcontrol = QStyle::SC_None;
    initStyleOption();
    switch (m_itemType) {
    case SpinBox:
    {
        QStyle::ComplexControl control = QStyle::CC_SpinBox;
        if (subcontrolString == QLatin1String("down"))
            subcontrol = QStyle::SC_SpinBoxDown;
        else if (subcontrolString == QLatin1String("up"))
            subcontrol = QStyle::SC_SpinBoxUp;
        else if (subcontrolString == QLatin1String("edit")){
            subcontrol = QStyle::SC_SpinBoxEditField;
        }
        return KyQuickStyleItem::style()->subControlRect(control,
                                             qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                             subcontrol);

    }
        break;
    case Slider:
    {
        QStyle::ComplexControl control = QStyle::CC_Slider;
        if (subcontrolString == QLatin1String("handle"))
            subcontrol = QStyle::SC_SliderHandle;
        else if (subcontrolString == QLatin1String("groove"))
            subcontrol = QStyle::SC_SliderGroove;
        return KyQuickStyleItem::style()->subControlRect(control,
                                             qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                             subcontrol);

    }
        break;
    case ScrollBar:
    {
        QStyle::ComplexControl control = QStyle::CC_ScrollBar;
        if (subcontrolString == QLatin1String("slider"))
            subcontrol = QStyle::SC_ScrollBarSlider;
        if (subcontrolString == QLatin1String("groove"))
            subcontrol = QStyle::SC_ScrollBarGroove;
        else if (subcontrolString == QLatin1String("handle"))
            subcontrol = QStyle::SC_ScrollBarSlider;
        else if (subcontrolString == QLatin1String("add"))
            subcontrol = QStyle::SC_ScrollBarAddPage;
        else if (subcontrolString == QLatin1String("sub"))
            subcontrol = QStyle::SC_ScrollBarSubPage;
        return KyQuickStyleItem::style()->subControlRect(control,
                                             qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                             subcontrol);
    }
        break;
    case ItemBranchIndicator: {
        QStyleOption opt;
        opt.rect = QRect(0, 0, implicitWidth(), implicitHeight());
        return KyQuickStyleItem::style()->subElementRect(QStyle::SE_TreeViewDisclosureItem, &opt, nullptr);
    }
    default:
        break;
    }
    return QRectF();
}

namespace  {
class QHighDpiPixmapsEnabler1 {
public:
    QHighDpiPixmapsEnabler1()
    :wasEnabled(false)
    {
        if (!qApp->testAttribute(Qt::AA_UseHighDpiPixmaps)) {
            qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
            wasEnabled = true;
        }
    }

    ~QHighDpiPixmapsEnabler1()
    {
        if (wasEnabled)
            qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);
    }
private:
    bool wasEnabled;
};
}

void KyQuickStyleItem::paint(QPainter *painter)
{
    initStyleOption();
    if (QStyleOptionMenuItem *opt = qstyleoption_cast<QStyleOptionMenuItem*>(m_styleoption))
        painter->setFont(opt->font);
    else {
        QFont font;
        if (m_styleoption->state & QStyle::State_Mini) {
            font = qApp->font("QMiniFont");
        } else if (m_styleoption->state & QStyle::State_Small) {
            font = qApp->font("QSmallFont");
        }
        painter->setFont(font);
    }

    // Set AA_UseHighDpiPixmaps when calling style code to make QIcon return
    // "retina" pixmaps. The flag is controlled by the application so we can't
    // set it unconditinally.
    QHighDpiPixmapsEnabler1 enabler;

    switch (m_itemType) {
    case Button:{
        QWidget wid;
        if(m_buttonType == "MaxButton" || m_buttonType == "MinButton" || m_buttonType == "WindowButton") {
            wid.setProperty("isWindowButton", QVariant(0x01));
        } else if(m_buttonType == "CloseButton") {
            wid.setProperty("isWindowButton", QVariant(0x02));
        }

        if(m_buttonType == "BlueButton" || m_buttonType == "ImportantButton") {
            wid.setProperty("isImportant", true);
        }
        if(m_roundButton == "RoundButton") {
            wid.setProperty("isRoundButton", true);
        }
        KyQuickStyleItem::style()->drawControl(QStyle::CE_PushButton, m_styleoption, painter,&wid);
}
        break;
    case ItemRow :{
        QPixmap pixmap;
        // Only draw through style once
        const QString pmKey = QLatin1String("itemrow") % QString::number(m_styleoption->state,16) % activeControl();
        if (!QPixmapCache::find(pmKey, &pixmap) || pixmap.width() < width() || height() != pixmap.height()) {
            int newSize = width();
            pixmap = QPixmap(newSize, height());
            pixmap.fill(Qt::transparent);
            QPainter pixpainter(&pixmap);
            KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelItemViewRow, m_styleoption, &pixpainter);
            if ((styleName() == QLatin1String("mac") || !KyQuickStyleItem::style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected)) && selected()) {
                QPalette pal = QApplication::palette("QAbstractItemView");
                pal.setCurrentColorGroup(m_styleoption->palette.currentColorGroup());
                pixpainter.fillRect(m_styleoption->rect, pal.highlight());
            }
            QPixmapCache::insert(pmKey, pixmap);
        }
        painter->drawPixmap(0, 0, pixmap);
    }
        break;
    case Item:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_ItemViewItem, m_styleoption, painter);
        break;
    case ItemBranchIndicator:
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_IndicatorBranch, m_styleoption, painter);
        break;
    case Header:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_Header, m_styleoption, painter);
        break;
    case ToolButton:
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_ToolButton, qstyleoption_cast<QStyleOptionComplex*>(m_styleoption), painter);
        break;
    case Tab:
        {
            if (m_lastFocusReason != Qt::TabFocusReason && m_lastFocusReason != Qt::BacktabFocusReason) {
                m_styleoption->state &= ~QStyle::State_HasFocus;
            }
            KyQuickStyleItem::style()->drawControl(QStyle::CE_TabBarTab, m_styleoption, painter);
        }
        break;
    case Frame:
        m_styleoption->state |= QStyle::State_Sunken;
        m_styleoption->state &= ~QStyle::State_Raised;
        KyQuickStyleItem::style()->drawControl(QStyle::CE_ShapedFrame, m_styleoption, painter);
        break;
    case FocusFrame:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_FocusFrame, m_styleoption, painter);
        break;
    case FocusRect:
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_FrameFocusRect, m_styleoption, painter);
        break;
    case TabFrame:
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_FrameTabWidget, m_styleoption, painter);
        break;
    case MenuBar:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_MenuBarEmptyArea, m_styleoption, painter);
        break;
    case MenuBarItem:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_MenuBarItem, m_styleoption, painter);
        break;
    case MenuItem:
    case ComboBoxItem: { // fall through
        QStyle::ControlElement menuElement =
                static_cast<QStyleOptionMenuItem *>(m_styleoption)->menuItemType == QStyleOptionMenuItem::Scroller ?
                    QStyle::CE_MenuScroller : QStyle::CE_MenuItem;
        KyQuickStyleItem::style()->drawControl(menuElement, m_styleoption, painter);
        }
        break;
    case CheckBox:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_CheckBox, m_styleoption, painter);
        break;
    case RadioButton:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_RadioButton, m_styleoption, painter);
        break;
    case Edit:
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelLineEdit, m_styleoption, painter);
        break;
    case MacHelpButton:
        //Not managed as mac is not supported
        break;
    case Widget:
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_Widget, m_styleoption, painter);
        break;
    case ScrollAreaCorner:
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelScrollAreaCorner, m_styleoption, painter);
        break;
    case Splitter:
        if (m_styleoption->rect.width() == 1)
            painter->fillRect(0, 0, width(), height(), m_styleoption->palette.dark().color());
        else
            KyQuickStyleItem::style()->drawControl(QStyle::CE_Splitter, m_styleoption, painter);
        break;
    case ComboBox:
    {
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_ComboBox,
                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                          painter);
        // This is needed on mac as it will use the painter color and ignore the palette
        QPen pen = painter->pen();
        painter->setPen(m_styleoption->palette.text().color());
        KyQuickStyleItem::style()->drawControl(QStyle::CE_ComboBoxLabel, m_styleoption, painter);
        painter->setPen(pen);
    }    break;
    case SpinBox:
#ifdef Q_OS_MAC
        // macstyle depends on the embedded qlineedit to fill the editfield background
        if (styleName() == QLatin1String("mac")) {
            QRect editRect = KyQuickStyleItem::style()->subControlRect(QStyle::CC_SpinBox,
                                                           qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                                           QStyle::SC_SpinBoxEditField);
            painter->fillRect(editRect.adjusted(-1, -1, 1, 1), m_styleoption->palette.base());
        }
#endif
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_SpinBox,
                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                          painter);
        break;
    case Slider:
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_Slider,
                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                          painter);
        break;
    case Dial:
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_Dial,
                                          qstyleoption_cast<QStyleOptionComplex*>(m_styleoption),
                                          painter);
        break;
    case ProgressBar:
        KyQuickStyleItem::style()->drawControl(QStyle::CE_ProgressBar, m_styleoption, painter);
        break;
    case ToolBar:
        painter->fillRect(m_styleoption->rect, m_styleoption->palette.window().color());
        KyQuickStyleItem::style()->drawControl(QStyle::CE_ToolBar, m_styleoption, painter);
        painter->save();
        painter->setPen(styleName() != QLatin1String("fusion") ? m_styleoption->palette.dark().color().darker(120) :
                                              m_styleoption->palette.window().color().lighter(107));
        painter->drawLine(m_styleoption->rect.bottomLeft(), m_styleoption->rect.bottomRight());
        painter->restore();
        break;
    case StatusBar:
        {
            painter->fillRect(m_styleoption->rect, m_styleoption->palette.window().color());
            painter->setPen(m_styleoption->palette.dark().color().darker(120));
            painter->drawLine(m_styleoption->rect.topLeft(), m_styleoption->rect.topRight());
            KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelStatusBar, m_styleoption, painter);
        }
        break;
    case GroupBox:
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_GroupBox, qstyleoption_cast<QStyleOptionComplex*>(m_styleoption), painter);
        break;
    case ScrollBar:
        KyQuickStyleItem::style()->drawComplexControl(QStyle::CC_ScrollBar, qstyleoption_cast<QStyleOptionSlider *>(m_styleoption), painter);
        break;
    case Menu: {
        QStyleHintReturnMask val;
        KyQuickStyleItem::style()->styleHint(QStyle::SH_Menu_Mask, m_styleoption, nullptr, &val);
        painter->save();
        painter->setClipRegion(val.region);
        painter->fillRect(m_styleoption->rect, m_styleoption->palette.window());
        painter->restore();
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelMenu, m_styleoption, painter);

        if (int fw = KyQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuPanelWidth)) {
            QStyleOptionFrame frame;
            frame.state = QStyle::State_None;
            frame.lineWidth = fw;
            frame.midLineWidth = 0;
            frame.rect = m_styleoption->rect;
            frame.styleObject = this;
            frame.palette = m_styleoption->palette;
            KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, painter);
        }
    }
        break;
    default:
        break;
    }
}

qreal KyQuickStyleItem::textWidth(const QString &text)
{
    QFontMetricsF fm = QFontMetricsF(m_styleoption->fontMetrics);
    return fm.boundingRect(text).width();
}

qreal KyQuickStyleItem::textHeight(const QString &text)
{
    QFontMetricsF fm = QFontMetricsF(m_styleoption->fontMetrics);
    return text.isEmpty() ? fm.height() :
                            fm.boundingRect(text).height();
}

QString KyQuickStyleItem::elidedText(const QString &text, int elideMode, int width)
{
    return m_styleoption->fontMetrics.elidedText(text, Qt::TextElideMode(elideMode), width);
}

bool KyQuickStyleItem::hasThemeIcon(const QString &icon) const
{
    return QIcon::hasThemeIcon(icon);
}

bool KyQuickStyleItem::event(QEvent *ev)
{
    if (ev->type() == QEvent::StyleAnimationUpdate) {
        if (isVisible()) {
            ev->accept();
            polish();
        }
        return true;
    }
    return QQuickItem::event(ev);
}

void KyQuickStyleItem::setTextureWidth(int w)
{
    if (m_textureWidth == w)
        return;
    m_textureWidth = w;
    emit textureWidthChanged(m_textureWidth);
    update();
}

void KyQuickStyleItem::setTextureHeight(int h)
{
    if (m_textureHeight == h)
        return;
    m_textureHeight = h;
    emit textureHeightChanged(m_textureHeight);
    update();
}

QQuickItem *KyQuickStyleItem::control() const
{
    return m_control;
}

void KyQuickStyleItem::setControl(QQuickItem *control)
{
    if (control == m_control) {
        return;
    }

    if (m_control) {
        m_control->removeEventFilter(this);
        disconnect(m_control, nullptr, this, nullptr);
    }

    m_control = control;

    if (m_control) {
        m_control->installEventFilter(this);

        if (m_control->window()) {
            m_window = m_control->window();
            m_window->installEventFilter(this);
        }
        connect(m_control, &QQuickItem::windowChanged, this,
                [this](QQuickWindow *window) {
            if (m_window) {
                m_window->removeEventFilter(this);
            }
            m_window = window;
            if (m_window) {
                m_window->installEventFilter(this);
            }
        });
    }

    emit controlChanged();
}

QSGNode *KyQuickStyleItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    if (m_image.isNull()) {
        delete node;
        return nullptr;
    }

    QSGNinePatchNode *styleNode = static_cast<QSGNinePatchNode *>(node);
    if (!styleNode)
        styleNode = window()->createNinePatchNode();

#ifdef QSG_RUNTIME_DESCRIPTION
    qsgnode_set_description(styleNode,
                            QString::fromLatin1("%1:%2, '%3'")
                            .arg(styleName())
                            .arg(elementType())
                            .arg(text()));
#endif

    styleNode->setTexture(window()->createTextureFromImage(m_image, QQuickWindow::TextureCanUseAtlas));
    styleNode->setBounds(boundingRect());
    styleNode->setDevicePixelRatio(window()->devicePixelRatio());
    styleNode->setPadding(m_border.left(), m_border.top(), m_border.right(), m_border.bottom());
    styleNode->update();

    return styleNode;
}

void KyQuickStyleItem::updatePolish()
{
    if (width() >= 1 && height() >= 1) { // Note these are reals so 1 pixel is minimum
        float devicePixelRatio = window() ? window()->devicePixelRatio() : qApp->devicePixelRatio();
        int w = m_textureWidth > 0 ? m_textureWidth : width();
        int h = m_textureHeight > 0 ? m_textureHeight : height();
        m_image = QImage(w * devicePixelRatio, h * devicePixelRatio, QImage::Format_ARGB32_Premultiplied);
        m_image.setDevicePixelRatio(devicePixelRatio);
        m_image.fill(Qt::transparent);
        QPainter painter(&m_image);
        painter.setLayoutDirection(qApp->layoutDirection());
        paint(&painter);
        QQuickItem::update();
    } else if (!m_image.isNull()) {
        m_image = QImage();
        QQuickItem::update();
    }
}

bool KyQuickStyleItem::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_control) {
        if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
            QFocusEvent *fe = static_cast<QFocusEvent *>(event);
            m_lastFocusReason = fe->reason();
        }
    } else if (watched == m_window.data()) {
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_Alt) {
                updateItem();
            }

        }
    }

    return QQuickItem::eventFilter(watched, event);
}

void KyQuickStyleItem::styleChanged()
{
    if (!qApp->style() || QApplication::closingDown()) {
        return;
    }

    Q_ASSERT(qApp->style() != sender());

    connect(qApp->style(), &QObject::destroyed, this, &KyQuickStyleItem::styleChanged);

    updateSizeHint();
    updateItem();
}

QPixmap QQuickTableRowImageProvider1::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED (requestedSize);
    int width = 16;
    int height = 16;
    if (size)
        *size = QSize(width, height);

    QPixmap pixmap(width, height);

    QStyleOptionViewItem opt;
    opt.state |= QStyle::State_Enabled;
    opt.rect = QRect(0, 0, width, height);
    QString style = QString::fromLatin1(KyQuickStyleItem::style()->metaObject()->className());
    opt.features = {};

    if (id.contains(QLatin1String("selected")))
        opt.state |= QStyle::State_Selected;

    if (id.contains(QLatin1String("active"))) {
        opt.state |= QStyle::State_Active;
        opt.palette.setCurrentColorGroup(QPalette::Active);
    } else
        opt.palette.setCurrentColorGroup(QPalette::Inactive);

    if (id.contains(QLatin1String("alternate")))
        opt.features |= QStyleOptionViewItem::Alternate;

    QPalette pal = QApplication::palette("QAbstractItemView");
    if (opt.state & QStyle::State_Selected && (style.contains(QLatin1String("Mac")) ||
                                               !KyQuickStyleItem::style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected))) {
        pal.setCurrentColorGroup(opt.palette.currentColorGroup());
        pixmap.fill(pal.highlight().color());
    } else {
        pixmap.fill(pal.base().color());
        QPainter pixpainter(&pixmap);
        KyQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, &pixpainter);
    }
    return pixmap;
}
