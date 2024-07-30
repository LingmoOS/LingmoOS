/*
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd. <https://www.qt.io/licensing/>
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 David Redondo <david@david-redondo.de>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial
*/

#include "kquickstyleitem_p.h"

#include <QApplication>
#include <QPainter>
#include <QPixmapCache>
#include <QQuickWindow>
#include <QStringBuilder>
#include <QStyle>
#include <QStyleFactory>
#include <qsgninepatchnode.h>
#include <qstyleoption.h>

#include <KConfigGroup>
#include <ksharedconfig.h>

#include <Kirigami/Platform/PlatformTheme>
#include <Kirigami/Platform/TabletModeWatcher>

#include <numeric>

std::unique_ptr<QStyle> KQuickStyleItem::s_style = nullptr;

QStyle *KQuickStyleItem::style()
{
    if (s_style) {
        return s_style.get();
    } else {
        return qApp->style();
    }
}

static bool isKeyFocusReason(Qt::FocusReason reason)
{
    return reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason || reason == Qt::ShortcutFocusReason;
}

KQuickStyleItem::KQuickStyleItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_styleoption(nullptr)
    , m_itemType(Undefined)
    , m_sunken(false)
    , m_raised(false)
    , m_flat(false)
    , m_active(true)
    , m_selected(false)
    , m_focus(false)
    , m_hover(false)
    , m_on(false)
    , m_horizontal(true)
    , m_transient(false)
    , m_sharedWidget(false)
    , m_minimum(0)
    , m_maximum(100)
    , m_value(0)
    , m_step(0)
    , m_paintMargins(0)
    , m_contentWidth(0)
    , m_contentHeight(0)
    , m_textureWidth(0)
    , m_textureHeight(0)
    , m_focusReason(Qt::NoFocusReason)
{
    // Check that we really are a QApplication before attempting to access the
    // application style.
    //
    // Functions used in this file which are safe to access through qApp are:
    //
    //   qApp->font() and qApp->fontChanged() - provided by QGuiApplication
    //   qApp->font("classname") - provided by QApplication but safe
    //   qApp->layoutDirection() - provided by QGuiApplication
    //   qApp->wheelScrollLines() - uses styleHints() provided by QGuiApplication
    //   qApp->testAttribute() and qApp->setAttribute() - provided by QCoreApplication
    //   qApp->devicePixelRatio() - provided by QGuiApplication
    //   qApp->palette("classname") - provided by QApplication but safe
    //
    // but any use of qApp->style(), even if only trying to test that it exists,
    // will assert.  Use KQuickStyleItem::style() as above.
    //
    // Any use of any other function provided by QApplication must either be checked
    // to ensure that it is safe to use if not a QApplication, or guarded.

    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        // We are a QApplication.  Unfortunately there is no styleChanged signal
        // available, and it only sends QEvent::StyleChange events to all QWidgets.
        // So watch for the existing application style being destroyed, which means
        // that a new one is being applied.  See QApplication::setStyle().
        QStyle *style = qApp->style();
        if (style) {
            connect(style, &QObject::destroyed, this, &KQuickStyleItem::styleChanged);
        }
    } else if (!s_style) {
        // We are not a QApplication.  Create an internal copy of the configured
        // desktop style, to be used for metrics, options and painting.
        KSharedConfig::Ptr kdeglobals = KSharedConfig::openConfig();
        KConfigGroup cg(kdeglobals, QStringLiteral("KDE"));
        const QString defaultStyleName = QStringLiteral("Fusion");
        s_style.reset(QStyleFactory::create(cg.readEntry("widgetStyle", defaultStyleName)));
        if (!s_style) {
            s_style.reset(QStyleFactory::create(defaultStyleName));
            // Prevent inevitable crashes on nullptr dereference
            if (!s_style) {
                qWarning() << "org.kde.desktop: Could not find any QStyle such as Breeze or Fusion";
                ::exit(EXIT_FAILURE);
            }
        }
    }

    m_font = qApp->font();
    setFlag(QQuickItem::ItemHasContents, true);
    setSmooth(false);

    qGuiApp->installEventFilter(this);

    Kirigami::Platform::TabletModeWatcher::self()->addWatcher(this);
}

KQuickStyleItem::~KQuickStyleItem()
{
    if (const QStyleOptionButton *aux = qstyleoption_cast<const QStyleOptionButton *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionViewItem *aux = qstyleoption_cast<const QStyleOptionViewItem *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionHeader *aux = qstyleoption_cast<const QStyleOptionHeader *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionToolButton *aux = qstyleoption_cast<const QStyleOptionToolButton *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionToolBar *aux = qstyleoption_cast<const QStyleOptionToolBar *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionTab *aux = qstyleoption_cast<const QStyleOptionTab *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionFrame *aux = qstyleoption_cast<const QStyleOptionFrame *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionFocusRect *aux = qstyleoption_cast<const QStyleOptionFocusRect *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionTabWidgetFrame *aux = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionMenuItem *aux = qstyleoption_cast<const QStyleOptionMenuItem *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionComboBox *aux = qstyleoption_cast<const QStyleOptionComboBox *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionSpinBox *aux = qstyleoption_cast<const QStyleOptionSpinBox *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionSlider *aux = qstyleoption_cast<const QStyleOptionSlider *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionProgressBar *aux = qstyleoption_cast<const QStyleOptionProgressBar *>(m_styleoption)) {
        delete aux;
    } else if (const QStyleOptionGroupBox *aux = qstyleoption_cast<const QStyleOptionGroupBox *>(m_styleoption)) {
        delete aux;
    } else {
        delete m_styleoption;
    }

    m_styleoption = nullptr;
    Kirigami::Platform::TabletModeWatcher::self()->removeWatcher(this);
}

void KQuickStyleItem::initStyleOption()
{
    if (!m_theme) {
        m_theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
        Q_ASSERT(m_theme);

        connect(m_theme, &Kirigami::Platform::PlatformTheme::colorsChanged, this, [this]() {
            // we need to reset the palette event if Qt::AA_SetPalette attribute has been set
            m_styleoption->palette = m_theme->palette();
            polish();
        });
    }
    Q_ASSERT(m_theme);

    if (m_styleoption) {
        m_styleoption->state = {};
    }

    QString sizeHint = m_hints.value(QStringLiteral("size")).toString();

    bool needsResolvePalette = true;
    bool preventMirroring = false;

    switch (m_itemType) {
    case Button: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionButton();
        }

        QStyleOptionButton *opt = qstyleoption_cast<QStyleOptionButton *>(m_styleoption);
        opt->text = text();

        if (m_iconDirty || opt->icon.isNull()) {
            opt->icon = iconFromIconProperty();
            m_iconDirty = false;
        }

        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(), m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KQuickStyleItem::style()->pixelMetric(QStyle::PM_ButtonIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;
        opt->features = activeControl() == QLatin1String("default") ? QStyleOptionButton::DefaultButton : QStyleOptionButton::None;
        if (m_flat) {
            opt->features |= QStyleOptionButton::Flat;
        }
        const QFont font = qApp->font("QPushButton");
        opt->fontMetrics = QFontMetrics(font);
        bool hasMenu = m_properties[QStringLiteral("menu")].toBool();
        if (hasMenu) {
            opt->features |= QStyleOptionButton::HasMenu;
        }
        break;
    }
    case ItemRow: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionViewItem();
        }

        QStyleOptionViewItem *opt = qstyleoption_cast<QStyleOptionViewItem *>(m_styleoption);
        opt->features = {};
        if (activeControl() == QLatin1String("alternate")) {
            opt->features |= QStyleOptionViewItem::Alternate;
        }
        break;
    }

    case Splitter: {
        // Use defaults
        break;
    }

    case Item: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionViewItem();
        }
        QStyleOptionViewItem *opt = qstyleoption_cast<QStyleOptionViewItem *>(m_styleoption);
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
        if (!m_styleoption) {
            m_styleoption = new QStyleOption;
        }

        if (m_properties.value(QStringLiteral("isItem")).toBool()) {
            m_styleoption->state = QStyle::State_Item;
        }
        if (m_properties.value(QStringLiteral("hasChildren")).toBool()) {
            m_styleoption->state |= QStyle::State_Children;
        }
        if (m_properties.value(QStringLiteral("hasSibling")).toBool()) { // Even this one could go away
            m_styleoption->state |= QStyle::State_Sibling;
        }
        if (m_on) {
            m_styleoption->state |= QStyle::State_Open;
        }
        break;
    }
    case Header: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionHeader();
        }

        QStyleOptionHeader *opt = qstyleoption_cast<QStyleOptionHeader *>(m_styleoption);
        opt->text = text();
        opt->textAlignment = static_cast<Qt::AlignmentFlag>(m_properties.value(QStringLiteral("textalignment")).toInt());
        opt->sortIndicator = activeControl() == QLatin1String("down") ? QStyleOptionHeader::SortDown
            : activeControl() == QLatin1String("up")                  ? QStyleOptionHeader::SortUp
                                                                      : QStyleOptionHeader::None;
        QString headerpos = m_properties.value(QStringLiteral("headerpos")).toString();
        if (headerpos == QLatin1String("beginning")) {
            opt->position = QStyleOptionHeader::Beginning;
        } else if (headerpos == QLatin1String("end")) {
            opt->position = QStyleOptionHeader::End;
        } else if (headerpos == QLatin1String("only")) {
            opt->position = QStyleOptionHeader::OnlyOneSection;
        } else {
            opt->position = QStyleOptionHeader::Middle;
        }
        opt->orientation = m_properties.value(QStringLiteral("orientation")).value<Qt::Orientation>();
        if (opt->orientation == Qt::Orientation(0)) {
            opt->orientation = Qt::Horizontal;
        }
        const QFont font = qApp->font("QHeaderView");
        opt->fontMetrics = QFontMetrics(font);
        break;
    }
    case DelayButton:
    case ToolButton: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionToolButton();
        }

        QStyleOptionToolButton *opt = qstyleoption_cast<QStyleOptionToolButton *>(m_styleoption);
        opt->subControls = QStyle::SC_ToolButton;

        if (m_flat) {
            opt->state |= QStyle::State_AutoRaise;
        }

        opt->activeSubControls = QStyle::SC_ToolButton;
        opt->text = text();

        if (m_iconDirty || opt->icon.isNull()) {
            opt->icon = iconFromIconProperty();
            m_iconDirty = false;
        }

        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(), m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            const auto metric = m_flat ? QStyle::PM_ToolBarIconSize : QStyle::PM_ButtonIconSize;
            int e = KQuickStyleItem::style()->pixelMetric(metric, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;

        if (m_properties.value(QStringLiteral("menu")).toBool()) {
            opt->features |= QStyleOptionToolButton::HasMenu;
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

        const QFont font = m_font.isCopyOf(qApp->font()) ? qApp->font("QToolButton") : m_font;
        opt->font = font;
        opt->fontMetrics = QFontMetrics(font);
        break;
    }
    case ToolBar: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionToolBar();
        }
        break;
    }
    case Tab: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionTab();
        }

        QStyleOptionTab *opt = qstyleoption_cast<QStyleOptionTab *>(m_styleoption);
        opt->text = text();

        if (m_iconDirty || opt->icon.isNull()) {
            opt->icon = iconFromIconProperty();
            m_iconDirty = false;
        }

        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(), m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;

        if (m_properties.value(QStringLiteral("hasFrame")).toBool()) {
            opt->features |= QStyleOptionTab::HasFrame;
        }

        QString orientation = m_properties.value(QStringLiteral("orientation")).toString();
        QString position = m_properties.value(QStringLiteral("tabpos")).toString();
        QString selectedPosition = m_properties.value(QStringLiteral("selectedpos")).toString();

        opt->shape = orientation == QLatin1String("Bottom") ? QTabBar::RoundedSouth : QTabBar::RoundedNorth;
        if (position == QLatin1String("beginning")) {
            opt->position = QStyleOptionTab::Beginning;
        } else if (position == QLatin1String("end")) {
            opt->position = QStyleOptionTab::End;
        } else if (position == QLatin1String("only")) {
            opt->position = QStyleOptionTab::OnlyOneTab;
        } else {
            opt->position = QStyleOptionTab::Middle;
        }

        if (selectedPosition == QLatin1String("next")) {
            opt->selectedPosition = QStyleOptionTab::NextIsSelected;
        } else if (selectedPosition == QLatin1String("previous")) {
            opt->selectedPosition = QStyleOptionTab::PreviousIsSelected;
        } else {
            opt->selectedPosition = QStyleOptionTab::NotAdjacent;
        }

        break;
    }

    case Frame: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionFrame();
        }

        QStyleOptionFrame *opt = qstyleoption_cast<QStyleOptionFrame *>(m_styleoption);
        opt->frameShape = QFrame::StyledPanel;
        opt->lineWidth = KQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
        opt->midLineWidth = opt->lineWidth;
        break;
    }
    case FocusRect: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionFocusRect();
        }
        // Needed on windows
        m_styleoption->state |= QStyle::State_KeyboardFocusChange;
        break;
    }
    case TabFrame: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionTabWidgetFrame();
        }
        QStyleOptionTabWidgetFrame *opt = qstyleoption_cast<QStyleOptionTabWidgetFrame *>(m_styleoption);

        opt->selectedTabRect = m_properties[QStringLiteral("selectedTabRect")].toRect();
        opt->shape = m_properties[QStringLiteral("orientation")] == Qt::BottomEdge ? QTabBar::RoundedSouth : QTabBar::RoundedNorth;
        if (minimum()) {
            opt->selectedTabRect = QRect(value(), 0, minimum(), height());
        }
        opt->tabBarSize = QSize(minimum(), height());
        // oxygen style needs this hack
        opt->leftCornerWidgetSize = QSize(value(), 0);
        break;
    }
    case MenuBar:
        if (!m_styleoption) {
            QStyleOptionMenuItem *menuOpt = new QStyleOptionMenuItem();
            menuOpt->menuItemType = QStyleOptionMenuItem::EmptyArea;
            m_styleoption = menuOpt;
        }

        break;
    case MenuBarItem: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionMenuItem();
        }

        QStyleOptionMenuItem *opt = qstyleoption_cast<QStyleOptionMenuItem *>(m_styleoption);
        opt->text = text();
        opt->menuItemType = QStyleOptionMenuItem::Normal;
        setProperty("_q_showUnderlined", m_hints[QStringLiteral("showUnderlined")].toBool());

        const QFont font = qApp->font("QMenuBar");
        opt->font = font;
        opt->fontMetrics = QFontMetrics(font);
        m_font = opt->font;
        break;
    }
    case Menu: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionMenuItem();
        }
        break;
    }
    case MenuItem:
    case ComboBoxItem: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionMenuItem();
        }

        QStyleOptionMenuItem *opt = qstyleoption_cast<QStyleOptionMenuItem *>(m_styleoption);
        // For GTK style. See below, in setElementType()
        setProperty("_q_isComboBoxPopupItem", m_itemType == ComboBoxItem);

        KQuickStyleItem::MenuItemType type = static_cast<KQuickStyleItem::MenuItemType>(m_properties[QStringLiteral("type")].toInt());
        if (type == KQuickStyleItem::ScrollIndicatorType) {
            int scrollerDirection = m_properties[QStringLiteral("scrollerDirection")].toInt();
            opt->menuItemType = QStyleOptionMenuItem::Scroller;
            opt->state |= scrollerDirection == Qt::UpArrow ? QStyle::State_UpArrow : QStyle::State_DownArrow;
        } else if (type == KQuickStyleItem::SeparatorType) {
            opt->menuItemType = QStyleOptionMenuItem::Separator;
        } else {
            opt->text = text();

            if (type == KQuickStyleItem::MenuType) {
                opt->menuItemType = QStyleOptionMenuItem::SubMenu;
            } else {
                opt->menuItemType = QStyleOptionMenuItem::Normal;

                QString shortcut = m_properties[QStringLiteral("shortcut")].toString();
                if (!shortcut.isEmpty()) {
                    opt->text += QLatin1Char('\t') + shortcut;
                    opt->reservedShortcutWidth = qMax(opt->reservedShortcutWidth, qRound(textWidth(shortcut)));
                }

                if (m_properties[QStringLiteral("checkable")].toBool()) {
                    opt->checked = on();
                    QVariant exclusive = m_properties[QStringLiteral("exclusive")];
                    opt->checkType = exclusive.toBool() ? QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
                }
            }

            if (m_iconDirty || opt->icon.isNull()) {
                opt->icon = iconFromIconProperty();
                m_iconDirty = false;
            }

            setProperty("_q_showUnderlined", m_hints[QStringLiteral("showUnderlined")].toBool());

            const QFont font = qApp->font(m_itemType == ComboBoxItem ? "QComboMenuItem" : "QMenu");
            opt->font = font;
            opt->fontMetrics = QFontMetrics(font);
            m_font = opt->font;
        }
        break;
    }
    case CheckBox:
    case RadioButton: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionButton();
        }

        QStyleOptionButton *opt = qstyleoption_cast<QStyleOptionButton *>(m_styleoption);
        if (!on()) {
            opt->state |= QStyle::State_Off;
        }
        if (m_properties.value(QStringLiteral("partiallyChecked")).toBool()) {
            opt->state |= QStyle::State_NoChange;
        }
        opt->text = text();

        if (m_iconDirty || opt->icon.isNull()) {
            opt->icon = iconFromIconProperty();
            m_iconDirty = false;
        }

        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(), m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KQuickStyleItem::style()->pixelMetric(QStyle::PM_ButtonIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;
        break;
    }
    case Edit: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionFrame();
        }

        QStyleOptionFrame *opt = qstyleoption_cast<QStyleOptionFrame *>(m_styleoption);
        opt->lineWidth = qMax(1, KQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr)); // this must be non zero
        break;
    }
    case ComboBox: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionComboBox();
        }

        QStyleOptionComboBox *opt = qstyleoption_cast<QStyleOptionComboBox *>(m_styleoption);

        const QFont font = qApp->font("QPushButton"); // DAVE - QQC1 code does this, but if you look at QComboBox this doesn't make sense
        opt->fontMetrics = QFontMetrics(font);
        opt->currentText = text();
        opt->editable = m_properties[QStringLiteral("editable")].toBool();

        const QVariant icon = m_properties[QStringLiteral("currentIcon")];
        if (icon.canConvert<QIcon>()) {
            opt->currentIcon = icon.value<QIcon>();
        } else if (icon.canConvert<QString>()) {
            opt->currentIcon = m_theme->iconFromTheme(icon.value<QString>(), m_properties[QStringLiteral("iconColor")].value<QColor>());
        }
        auto iconSize = QSize(m_properties[QStringLiteral("iconWidth")].toInt(), m_properties[QStringLiteral("iconHeight")].toInt());
        if (iconSize.isEmpty()) {
            int e = KQuickStyleItem::style()->pixelMetric(QStyle::PM_ButtonIconSize, m_styleoption, nullptr);
            if (iconSize.width() <= 0) {
                iconSize.setWidth(e);
            }
            if (iconSize.height() <= 0) {
                iconSize.setHeight(e);
            }
        }
        opt->iconSize = iconSize;
        opt->frame = !m_flat;

        break;
    }
    case SpinBox: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionSpinBox();
        }

        QStyleOptionSpinBox *opt = qstyleoption_cast<QStyleOptionSpinBox *>(m_styleoption);
        opt->frame = true;
        if (value() & 0x1) {
            opt->activeSubControls = QStyle::SC_SpinBoxUp;
        } else if (value() & (1 << 1)) {
            opt->activeSubControls = QStyle::SC_SpinBoxDown;
        } else {
            opt->activeSubControls = QStyle::SC_None;
        }
        opt->subControls = QStyle::SC_All;
        opt->stepEnabled = {};
        if (value() & (1 << 2)) {
            opt->stepEnabled |= QAbstractSpinBox::StepUpEnabled;
        }
        if (value() & (1 << 3)) {
            opt->stepEnabled |= QAbstractSpinBox::StepDownEnabled;
        }
        break;
    }
    case Slider:
    case Dial: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionSlider();
        }

        QStyleOptionSlider *opt = qstyleoption_cast<QStyleOptionSlider *>(m_styleoption);
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
            if (numOfSteps && (extent / numOfSteps < 5)) {
                opt->tickInterval = qRound((5 * numOfSteps / extent) + 0.5) * step();
            } else {
                opt->tickInterval = opt->singleStep;
            }

        } else { // default Qt-components implementation
            opt->tickInterval = opt->maximum != opt->minimum ? 1200 / (opt->maximum - opt->minimum) : 0;
        }

        opt->sliderValue = value();
        opt->subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
        opt->tickPosition = activeControl() == QLatin1String("ticks") ? QSlider::TicksBelow : QSlider::NoTicks;
        if (opt->tickPosition != QSlider::NoTicks) {
            opt->subControls |= QStyle::SC_SliderTickmarks;
        }

        opt->activeSubControls = QStyle::SC_SliderHandle;
        break;
    }
    case ProgressBar: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionProgressBar();
        }

        QStyleOptionProgressBar *opt = qstyleoption_cast<QStyleOptionProgressBar *>(m_styleoption);
        if (horizontal()) {
            opt->state |= QStyle::State_Horizontal;
        } else {
            opt->state &= ~QStyle::State_Horizontal;
        }
        opt->minimum = qMax(0, minimum());
        opt->maximum = qMax(0, maximum());
        opt->progress = value();
        break;
    }
    case GroupBox: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionGroupBox();
        }

        QStyleOptionGroupBox *opt = qstyleoption_cast<QStyleOptionGroupBox *>(m_styleoption);
        opt->text = text();
        opt->lineWidth = KQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
        opt->subControls = QStyle::SC_GroupBoxLabel;
        opt->features = {};
        if (m_properties[QStringLiteral("sunken")].toBool()) { // Qt draws an ugly line here so I ignore it
            opt->subControls |= QStyle::SC_GroupBoxFrame;
        } else {
            opt->features |= QStyleOptionFrame::Flat;
        }
        if (m_properties[QStringLiteral("checkable")].toBool()) {
            opt->subControls |= QStyle::SC_GroupBoxCheckBox;
        }

        break;
    }
    case ScrollBar: {
        if (!m_styleoption) {
            m_styleoption = new QStyleOptionSlider();
        }

        QStyleOptionSlider *opt = qstyleoption_cast<QStyleOptionSlider *>(m_styleoption);
        opt->minimum = qMax(0, minimum());
        opt->maximum = qMax(0, maximum());
        opt->pageStep = qMax(0, int(horizontal() ? width() : height()));
        opt->orientation = horizontal() ? Qt::Horizontal : Qt::Vertical;
        if (horizontal()) {
            // mirrored horizontal scrollbars are not something you want to interact with
            preventMirroring = true;
        }
        opt->sliderPosition = value();
        opt->sliderValue = value();
        opt->activeSubControls = (activeControl() == QLatin1String("up")) ? QStyle::SC_ScrollBarSubLine
            : (activeControl() == QLatin1String("down"))                  ? QStyle::SC_ScrollBarAddLine
            : (activeControl() == QLatin1String("handle"))                ? QStyle::SC_ScrollBarSlider
            : hover()                                                     ? QStyle::SC_ScrollBarGroove
                                                                          : QStyle::SC_None;
        if (raised()) {
            opt->state |= QStyle::State_On;
        }

        opt->subControls = QStyle::SC_All;

        setTransient(KQuickStyleItem::style()->styleHint(QStyle::SH_ScrollBar_Transient, m_styleoption));
        break;
    }
    default:
        break;
    }

    if (!m_styleoption) {
        m_styleoption = new QStyleOption();
    }

    if (needsResolvePalette) {
        resolvePalette();
    }

    m_styleoption->styleObject = this;
    const auto mirror = m_control == nullptr ? qApp->layoutDirection() == Qt::RightToLeft : m_control->property("mirrored").toBool();
    m_styleoption->direction = (mirror && !preventMirroring) ? Qt::RightToLeft : Qt::LeftToRight;

    int w = m_textureWidth > 0 ? m_textureWidth : width();
    int h = m_textureHeight > 0 ? m_textureHeight : height();

    m_styleoption->rect = QRect(m_paintMargins, 0, w - 2 * m_paintMargins, h);

    if (isEnabled()) {
        m_styleoption->state |= QStyle::State_Enabled;
        m_styleoption->palette.setCurrentColorGroup(QPalette::Active);
    } else {
        m_styleoption->palette.setCurrentColorGroup(QPalette::Disabled);
    }

    if (m_active) {
        m_styleoption->state |= QStyle::State_Active;
    } else {
        m_styleoption->palette.setCurrentColorGroup(QPalette::Inactive);
    }

    if (m_sunken) {
        m_styleoption->state |= QStyle::State_Sunken;
    }
    if (!m_sunken || m_raised) {
        m_styleoption->state |= QStyle::State_Raised;
    }
    if (m_selected) {
        m_styleoption->state |= QStyle::State_Selected;
    }
    if (m_focus) {
        m_styleoption->state |= QStyle::State_HasFocus;
    }
    if (m_on) {
        m_styleoption->state |= QStyle::State_On;
    }
    if (m_hover) {
        m_styleoption->state |= QStyle::State_MouseOver;
    }
    if (m_horizontal) {
        m_styleoption->state |= QStyle::State_Horizontal;
    }

    // some styles don't draw a focus rectangle if
    // QStyle::State_KeyboardFocusChange is not set
    if (window()) {
        if (isKeyFocusReason(m_focusReason)) {
            m_styleoption->state |= QStyle::State_KeyboardFocusChange;
        }
    }

    if (sizeHint == QLatin1String("mini")) {
        m_styleoption->state |= QStyle::State_Mini;
    } else if (sizeHint == QLatin1String("small")) {
        m_styleoption->state |= QStyle::State_Small;
    }

}

QIcon KQuickStyleItem::iconFromIconProperty() const
{
    QIcon icon;
    const QVariant iconProperty = m_properties[QStringLiteral("icon")];
    switch (iconProperty.userType()) {
    case QMetaType::QIcon:
        icon = iconProperty.value<QIcon>();
        break;
    case QMetaType::QUrl:
    case QMetaType::QString: {
        QString iconSource = iconProperty.toString();
        if (iconSource.startsWith(QLatin1String("qrc:/"))) {
            iconSource = iconSource.mid(3);
        } else if (iconSource.startsWith(QLatin1String("file:/"))) {
            iconSource = QUrl(iconSource).toLocalFile();
        }
        if (iconSource.contains(QLatin1String("/"))) {
            icon = QIcon(iconSource);
        } else {
            icon = m_theme->iconFromTheme(iconSource, m_properties[QStringLiteral("iconColor")].value<QColor>());
        }
        break;
    }
    default:
        break;
    }
    return icon;
}

const char *KQuickStyleItem::classNameForItem() const
{
    switch (m_itemType) {
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
    case DelayButton:
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

void KQuickStyleItem::resolvePalette()
{
    if (QCoreApplication::testAttribute(Qt::AA_SetPalette)) {
        return;
    }

    // In theory here we should conside  m_control->property("palette")
    // Whether the client code did set a custom palette color, but we can't read and write it
    // from c++ anymore as is not a QPalette but a wrapper type we don't have access to,
    // this kind of binding will need to be done on QML side
    m_styleoption->palette = m_theme->palette();
}

int KQuickStyleItem::topPadding() const
{
    return padding(Qt::TopEdge);
}

int KQuickStyleItem::leftPadding() const
{
    return padding(Qt::LeftEdge);
}

int KQuickStyleItem::rightPadding() const
{
    return padding(Qt::RightEdge);
}

int KQuickStyleItem::bottomPadding() const
{
    return padding(Qt::BottomEdge);
}

int KQuickStyleItem::padding(Qt::Edge edge) const
{
    if (m_itemType == Frame) {
        const QRect cr = KQuickStyleItem::style()->subElementRect(QStyle::SE_ShapedFrameContents, m_styleoption);
        switch (edge) {
        case Qt::TopEdge:
            return cr.top() - m_styleoption->rect.top();
        case Qt::LeftEdge:
            return cr.left() - m_styleoption->rect.left();
        case Qt::RightEdge:
            return m_styleoption->rect.right() - cr.right();
        case Qt::BottomEdge:
            return m_styleoption->rect.bottom() - cr.bottom();
        }
    }
    return 0;
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

QString KQuickStyleItem::styleName() const
{
    const QString fullName = QString::fromLatin1(KQuickStyleItem::style()->metaObject()->className());
    QStringView shortName = fullName;
    if (shortName.startsWith(QLatin1Char('q'), Qt::CaseInsensitive)) {
        shortName = shortName.sliced(1);
    }
    const QLatin1String suffix("style");
    if (shortName.endsWith(suffix, Qt::CaseInsensitive)) {
        shortName.chop(suffix.length());
    }
    return shortName.toString().toLower();
}

QString KQuickStyleItem::hitTest(int px, int py)
{
    QStyle::SubControl subcontrol = QStyle::SC_All;
    switch (m_itemType) {
    case SpinBox: {
        subcontrol = KQuickStyleItem::style()->hitTestComplexControl(QStyle::CC_SpinBox,
                                                                     qstyleoption_cast<QStyleOptionComplex *>(m_styleoption),
                                                                     QPoint(px, py),
                                                                     nullptr);
        if (subcontrol == QStyle::SC_SpinBoxUp) {
            return QStringLiteral("up");
        } else if (subcontrol == QStyle::SC_SpinBoxDown) {
            return QStringLiteral("down");
        }
        break;
    }

    case Slider: {
        subcontrol = KQuickStyleItem::style()->hitTestComplexControl(QStyle::CC_Slider,
                                                                     qstyleoption_cast<QStyleOptionComplex *>(m_styleoption),
                                                                     QPoint(px, py),
                                                                     nullptr);
        if (subcontrol == QStyle::SC_SliderHandle) {
            return QStringLiteral("handle");
        }
        break;
    }

    case ScrollBar: {
        subcontrol = KQuickStyleItem::style()->hitTestComplexControl(QStyle::CC_ScrollBar,
                                                                     qstyleoption_cast<QStyleOptionComplex *>(m_styleoption),
                                                                     QPoint(px, py),
                                                                     nullptr);
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
        break;
    }

    default:
        break;
    }
    return QStringLiteral("none");
}

QRect KQuickStyleItem::computeBoundingRect(const QList<QRect> &rects)
{
    const auto region = std::accumulate(rects.begin(), rects.end(), QRegion());
    return region.boundingRect();
}

QSize KQuickStyleItem::sizeFromContents(int width, int height)
{
    initStyleOption();

    QSize size;
    switch (m_itemType) {
    case RadioButton:
    case CheckBox: {
        auto contentType = (m_itemType == RadioButton) ? QStyle::CT_RadioButton : QStyle::CT_CheckBox;
        QStyleOptionButton *btn = qstyleoption_cast<QStyleOptionButton *>(m_styleoption);
        QSize contentSize = btn->fontMetrics.size(Qt::TextShowMnemonic, btn->text);
        if (!btn->icon.isNull()) {
            contentSize.setWidth(contentSize.width() + btn->iconSize.width());
            contentSize.setHeight(std::max(contentSize.height(), btn->iconSize.height()));
        }
        size = KQuickStyleItem::style()->sizeFromContents(contentType, m_styleoption, contentSize);
        break;
    }
    case ToolBar:
        size = QSize(200, styleName().contains(QLatin1String("windows")) ? 30 : 42);
        break;
    case DelayButton:
    case ToolButton: {
        QStyleOptionToolButton *btn = qstyleoption_cast<QStyleOptionToolButton *>(m_styleoption);
        int w = 0;
        int h = 0;
        if (btn->toolButtonStyle != Qt::ToolButtonTextOnly) {
            QSize icon = btn->iconSize;
            if (btn->toolButtonStyle == Qt::ToolButtonIconOnly || !btn->icon.isNull()) {
                w = icon.width();
                h = icon.height();
            }
        }
        if (btn->toolButtonStyle != Qt::ToolButtonIconOnly) {
            QSize textSize = btn->fontMetrics.size(Qt::TextShowMnemonic, btn->text);
            textSize.setWidth(textSize.width() + btn->fontMetrics.horizontalAdvance(QLatin1Char(' ')) * 2);
            if (btn->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                h += 4 + textSize.height();
                if (textSize.width() > w) {
                    w = textSize.width();
                }
            } else if (btn->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                w += 4 + textSize.width();
                if (textSize.height() > h) {
                    h = textSize.height();
                }
            } else { // TextOnly
                w = textSize.width();
                h = textSize.height();
            }
        }
        btn->rect.setSize(QSize(w, h));
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_ToolButton, m_styleoption, QSize(w, h));
        break;
    }
    case Button: {
        QStyleOptionButton *btn = qstyleoption_cast<QStyleOptionButton *>(m_styleoption);

        int contentWidth = btn->fontMetrics.boundingRect(btn->text).width();
        int contentHeight = btn->fontMetrics.height();

        if (!btn->icon.isNull()) {
            //+4 matches a hardcoded value in QStyle and acts as a margin between the icon and the text.
            contentWidth += btn->iconSize.width() + 4;
            contentHeight = qMax(btn->fontMetrics.height(), btn->iconSize.height());
        }

        int newWidth = qMax(width, contentWidth);
        int newHeight = qMax(height, contentHeight);
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_PushButton, m_styleoption, QSize(newWidth, newHeight));
        break;
    }
    case ComboBox: {
        QStyleOptionComboBox *opt = qstyleoption_cast<QStyleOptionComboBox *>(m_styleoption);
        int newWidth = qMax(width, m_contentWidth) + (opt->currentIcon.isNull() ? 0 : opt->iconSize.width());
        int newHeight = qMax(height, opt->fontMetrics.height());
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_ComboBox, m_styleoption, QSize(newWidth, newHeight));
        break;
    }
    case Tab: {
        QStyleOptionTab *tab = qstyleoption_cast<QStyleOptionTab *>(m_styleoption);

        int contentWidth = tab->fontMetrics.boundingRect(tab->text).width();
        int contentHeight = tab->fontMetrics.height();

        if (!tab->icon.isNull()) {
            //+4 matches a hardcoded value in QStyle and acts as a margin between the icon and the text.
            contentWidth += tab->iconSize.width() + 4;
            contentHeight = qMax(contentHeight, tab->iconSize.height());
        }

        contentWidth += KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabHSpace, tab);
        contentHeight += KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabVSpace, tab);

        const int newWidth = qMax(width, contentWidth);
        const int newHeight = qMax(height, contentHeight);

        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_TabBarTab, m_styleoption, QSize(newWidth, newHeight));

        break;
    }
    case Slider: {
        const auto opt = qstyleoption_cast<const QStyleOptionSlider *>(m_styleoption);
        const auto tickPosition = opt->tickPosition;

        // Copied from QSlider
        const int SliderLength = 84, TickSpace = 5;
        int thick = style()->pixelMetric(QStyle::PM_SliderThickness, m_styleoption, nullptr);
        if (tickPosition & QSlider::TicksAbove) {
            thick += TickSpace;
        }
        if (tickPosition & QSlider::TicksBelow) {
            thick += TickSpace;
        }
        int w = thick;
        int h = SliderLength;
        if (opt->orientation == Qt::Horizontal) {
            w = SliderLength;
            h = thick;
        }
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_Slider, m_styleoption, QSize(w, h));
        break;
    }
    case ProgressBar:
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_ProgressBar, m_styleoption, QSize(width, height));
        break;

    case ScrollBar: {
        const auto opt = qstyleoption_cast<const QStyleOptionSlider *>(m_styleoption);

        // Copied from QScrollBar
        const int scrollBarExtent = KQuickStyleItem::style()->pixelMetric(QStyle::PM_ScrollBarExtent, opt, nullptr);
        const int scrollBarSliderMin = KQuickStyleItem::style()->pixelMetric(QStyle::PM_ScrollBarSliderMin, opt, nullptr);
        if (opt->orientation == Qt::Horizontal) {
            size = QSize(scrollBarExtent * 2 + scrollBarSliderMin, scrollBarExtent);
        } else {
            size = QSize(scrollBarExtent, scrollBarExtent * 2 + scrollBarSliderMin);
        }
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_ScrollBar, opt, size);
        break;
    }
    case SpinBox:
    case Edit: {
        // We have to create a new style option since we might be calling with a QStyleOptionSpinBox
        QStyleOptionFrame frame;
        //+2 to be consistent with the hardcoded verticalmargin in QLineEdit
        int contentHeight = frame.fontMetrics.height() + 2;

        frame.state = m_styleoption->state | QStyle::State_Sunken;
        frame.lineWidth = KQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption, nullptr);
        frame.rect = m_styleoption->rect;
        frame.styleObject = this;

        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_LineEdit, &frame, QSize(width, qMax(height, contentHeight)));
        if (m_itemType == SpinBox) {
            size.setWidth(KQuickStyleItem::style()->sizeFromContents(QStyle::CT_SpinBox, m_styleoption, QSize(width + 2, height)).width());
        }
        break;
    }
    case GroupBox: {
        QStyleOptionGroupBox *box = qstyleoption_cast<QStyleOptionGroupBox *>(m_styleoption);
        QFontMetrics metrics(box->fontMetrics);
        int baseWidth = metrics.boundingRect(box->text + QLatin1Char(' ')).width();
        int baseHeight = metrics.height() + m_contentHeight;
        if (box->subControls & QStyle::SC_GroupBoxCheckBox) {
            baseWidth += KQuickStyleItem::style()->pixelMetric(QStyle::PM_IndicatorWidth);
            baseWidth += KQuickStyleItem::style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);
            baseHeight = qMax(baseHeight, KQuickStyleItem::style()->pixelMetric(QStyle::PM_IndicatorHeight));
        }
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_GroupBox, m_styleoption, QSize(qMax(baseWidth, m_contentWidth), baseHeight));
        break;
    }
    case Header:
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_HeaderSection, m_styleoption, QSize(width, height));
        break;
    case ItemRow:
    case Item: // fall through
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_ItemViewItem, m_styleoption, QSize(width, height));
        break;
    case Splitter: {
        const auto hw = handleWidth();
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_Splitter, m_styleoption, QSize(hw, hw));
        break;
    }
    case MenuBarItem:
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_MenuBarItem, m_styleoption, QSize(width, height));
        break;
    case MenuBar:
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_MenuBar, m_styleoption, QSize(width, height));
        break;
    case Menu:
        size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_Menu, m_styleoption, QSize(width, height));
        break;
    case MenuItem:
    case ComboBoxItem:
        if (static_cast<QStyleOptionMenuItem *>(m_styleoption)->menuItemType == QStyleOptionMenuItem::Scroller) {
            size.setHeight(KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuScrollerHeight, nullptr, nullptr));
        } else {
            size = KQuickStyleItem::style()->sizeFromContents(QStyle::CT_MenuItem, m_styleoption, QSize(width, height));
        }
        break;
    default:
        break;
    }
    return size.expandedTo(QSize(m_contentWidth, m_contentHeight));
}

int KQuickStyleItem::handleWidth() const
{
    // TODO: emulate QSplitter::handleWidth using contentWidth or a custom property?
    return style()->pixelMetric(QStyle::PM_SplitterWidth, nullptr);
}

qreal KQuickStyleItem::baselineOffset() const
{
    QRect r;
    bool ceilResult = true; // By default baseline offset rounding is done upwards
    switch (m_itemType) {
    case Button:
        r = KQuickStyleItem::style()->subElementRect(QStyle::SE_PushButtonContents, m_styleoption);
        break;
    case CheckBox:
        r = KQuickStyleItem::style()->subElementRect(QStyle::SE_CheckBoxContents, m_styleoption);
        break;
    case ComboBox:
        if (const auto option = qstyleoption_cast<const QStyleOptionComboBox *>(m_styleoption)) {
            r = KQuickStyleItem::style()->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxEditField);
            if (styleName() != QLatin1String("mac")) {
                r.adjust(0, 0, 0, 1);
            }
        }
        break;
    case DelayButton:
    case ToolButton:
        if (const auto option = qstyleoption_cast<const QStyleOptionComplex *>(m_styleoption)) {
            r = KQuickStyleItem::style()->subControlRect(QStyle::CC_ToolButton, option, QStyle::SC_ToolButton);
        }
        break;
    case Edit:
        r = KQuickStyleItem::style()->subElementRect(QStyle::SE_LineEditContents, m_styleoption);
        break;
    case ProgressBar:
        r = m_styleoption->rect;
        r.adjust(0, 0, 0, -2);
        break;
    case RadioButton:
        r = KQuickStyleItem::style()->subElementRect(QStyle::SE_RadioButtonContents, m_styleoption);
        break;
    case Slider:
        r = m_styleoption->rect;
        r.adjust(0, 0, 0, -2);
        break;
    case SpinBox:
        if (const auto option = qstyleoption_cast<const QStyleOptionSpinBox *>(m_styleoption)) {
            r = KQuickStyleItem::style()->subControlRect(QStyle::CC_SpinBox, option, QStyle::SC_SpinBoxEditField);
            ceilResult = false;
        }
        break;
    default:
        break;
    }
    if (r.height() > 0) {
        const QFontMetrics &fm = m_styleoption->fontMetrics;
        int surplus = r.height() - fm.height();
        if ((surplus & 1) && ceilResult) {
            surplus++;
        }
        int result = r.top() + surplus / 2 + fm.ascent();
        return result;
    }

    return 0.;
}

void KQuickStyleItem::updateBaselineOffset()
{
    const qreal baseline = baselineOffset();
    if (baseline > 0) {
        setBaselineOffset(baseline);
    }
}

void KQuickStyleItem::setContentWidth(int arg)
{
    if (m_contentWidth != arg) {
        m_contentWidth = arg;
        updateSizeHint();
        Q_EMIT contentWidthChanged(arg);
    }
}

void KQuickStyleItem::setContentHeight(int arg)
{
    if (m_contentHeight != arg) {
        m_contentHeight = arg;
        updateSizeHint();
        updateBaselineOffset();
        Q_EMIT contentHeightChanged(arg);
    }
}

void KQuickStyleItem::updateSizeHint()
{
    QSize implicitSize = sizeFromContents(m_contentWidth, m_contentHeight);
    setImplicitSize(implicitSize.width(), implicitSize.height());
}

void KQuickStyleItem::updateRect()
{
    initStyleOption();
    m_styleoption->rect.setWidth(width());
    m_styleoption->rect.setHeight(height());
}

int KQuickStyleItem::pixelMetric(const QString &metric)
{
    if (metric == QLatin1String("scrollbarExtent")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr);
    } else if (metric == QLatin1String("defaultframewidth")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, m_styleoption);
    } else if (metric == QLatin1String("taboverlap")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabOverlap, nullptr);
    } else if (metric == QLatin1String("tabbaseoverlap")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, m_styleoption);
    } else if (metric == QLatin1String("tabhspace")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabHSpace, nullptr);
    } else if (metric == QLatin1String("indicatorwidth")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr);
    } else if (metric == QLatin1String("exclusiveindicatorwidth")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, nullptr);
    } else if (metric == QLatin1String("checkboxlabelspacing")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr);
    } else if (metric == QLatin1String("radiobuttonlabelspacing")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_RadioButtonLabelSpacing, nullptr);
    } else if (metric == QLatin1String("tabvspace")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabVSpace, nullptr);
    } else if (metric == QLatin1String("tabbaseheight")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarBaseHeight, nullptr);
    } else if (metric == QLatin1String("tabvshift")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TabBarTabShiftVertical, nullptr);
    } else if (metric == QLatin1String("menubarhmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarHMargin, nullptr);
    } else if (metric == QLatin1String("menubarvmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr);
    } else if (metric == QLatin1String("menubarpanelwidth")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr);
    } else if (metric == QLatin1String("menubaritemspacing")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuBarItemSpacing, nullptr);
    } else if (metric == QLatin1String("spacebelowmenubar")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, m_styleoption);
    } else if (metric == QLatin1String("menuhmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuHMargin, nullptr);
    } else if (metric == QLatin1String("menuvmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuVMargin, nullptr);
    } else if (metric == QLatin1String("menupanelwidth")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuPanelWidth, nullptr);
    } else if (metric == QLatin1String("submenuoverlap")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_SubMenuOverlap, nullptr);
    } else if (metric == QLatin1String("splitterwidth")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_SplitterWidth, nullptr);
    } else if (metric == QLatin1String("scrollbarspacing")) {
        return abs(KQuickStyleItem::style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, nullptr));
    } else if (metric == QLatin1String("treeviewindentation")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_TreeViewIndentation, nullptr);
    } else if (metric == QLatin1String("layouthorizontalspacing")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, nullptr);
    } else if (metric == QLatin1String("layoutverticalspacing")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing, nullptr);
    } else if (metric == QLatin1String("layoutleftmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr);
    } else if (metric == QLatin1String("layouttopmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutTopMargin, nullptr);
    } else if (metric == QLatin1String("layoutrightmargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutRightMargin, nullptr);
    } else if (metric == QLatin1String("layoutbottommargin")) {
        return KQuickStyleItem::style()->pixelMetric(QStyle::PM_LayoutBottomMargin, nullptr);
    }
    return 0;
}

QVariant KQuickStyleItem::styleHint(const QString &metric)
{
    initStyleOption();
    if (metric == QLatin1String("comboboxpopup")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_ComboBox_Popup, m_styleoption);
    } else if (metric == QLatin1String("highlightedTextColor")) {
        return m_styleoption->palette.highlightedText().color().name();
    } else if (metric == QLatin1String("textColor")) {
        QPalette pal = m_styleoption->palette;
        pal.setCurrentColorGroup(active() ? QPalette::Active : QPalette::Inactive);
        return pal.text().color().name();
    } else if (metric == QLatin1String("focuswidget")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_FocusFrame_AboveWidget);
    } else if (metric == QLatin1String("tabbaralignment")) {
        int result = KQuickStyleItem::style()->styleHint(QStyle::SH_TabBar_Alignment);
        if (result == Qt::AlignCenter) {
            return QStringLiteral("center");
        }
        return QStringLiteral("left");
    } else if (metric == QLatin1String("externalScrollBars")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents);
    } else if (metric == QLatin1String("scrollToClickPosition")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition);
    } else if (metric == QLatin1String("activateItemOnSingleClick")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    } else if (metric == QLatin1String("submenupopupdelay")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, m_styleoption);
    } else if (metric == QLatin1String("wheelScrollLines")) {
        return qApp->wheelScrollLines();
    } else if (metric == QLatin1String("dialogButtonsHaveIcons")) {
        return KQuickStyleItem::style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons);
    }
    return 0;

    // Add SH_Menu_SpaceActivatesItem
}

void KQuickStyleItem::setHints(const QVariantMap &hints)
{
    if (m_hints != hints) {
        m_hints = hints;
        initStyleOption();
        updateSizeHint();
        polish();
        if (m_styleoption->state & QStyle::State_Mini) {
            m_font.setPointSize(9.);
            Q_EMIT fontChanged();
        } else if (m_styleoption->state & QStyle::State_Small) {
            m_font.setPointSize(11.);
            Q_EMIT fontChanged();
        } else {
            Q_EMIT hintChanged();
        }
    }
}

void KQuickStyleItem::resetHints()
{
    m_hints.clear();
}

void KQuickStyleItem::setFont(const QFont &font)
{
    if (font == m_font) {
        return;
    }
    m_font = font;
    updateSizeHint();
    polish();
    Q_EMIT fontChanged();
}

void KQuickStyleItem::resetFont()
{
    setFont(qApp->font());
}

void KQuickStyleItem::setElementType(const QString &str)
{
    if (m_type == str) {
        return;
    }

    m_type = str;

    Q_EMIT elementTypeChanged();
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
    } else if (str == QLatin1String("comboboxitem")) {
        // Gtk uses qobject cast, hence we need to separate this from menuitem
        // On macOS, we temporarily use the menu item because it has more accurate
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
    } else if (str == QLatin1String("delaybutton")) {
        m_itemType = DelayButton;
    } else {
        m_itemType = Undefined;
    }
    Q_EMIT leftPaddingChanged();
    Q_EMIT rightPaddingChanged();
    Q_EMIT topPaddingChanged();
    Q_EMIT bottomPaddingChanged();
    updateSizeHint();
    polish();
}

QRectF KQuickStyleItem::subControlRect(const QString &subcontrolString)
{
    QStyle::SubControl subcontrol = QStyle::SC_None;
    initStyleOption();
    switch (m_itemType) {
    case SpinBox: {
        QStyle::ComplexControl control = QStyle::CC_SpinBox;
        if (subcontrolString == QLatin1String("down")) {
            subcontrol = QStyle::SC_SpinBoxDown;
        } else if (subcontrolString == QLatin1String("up")) {
            subcontrol = QStyle::SC_SpinBoxUp;
        } else if (subcontrolString == QLatin1String("edit")) {
            subcontrol = QStyle::SC_SpinBoxEditField;
        }
        return KQuickStyleItem::style()->subControlRect(control, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), subcontrol);

        break;
    }
    case Slider: {
        QStyle::ComplexControl control = QStyle::CC_Slider;
        if (subcontrolString == QLatin1String("handle")) {
            subcontrol = QStyle::SC_SliderHandle;
        } else if (subcontrolString == QLatin1String("groove")) {
            subcontrol = QStyle::SC_SliderGroove;
        }
        return KQuickStyleItem::style()->subControlRect(control, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), subcontrol);

        break;
    }
    case ScrollBar: {
        QStyle::ComplexControl control = QStyle::CC_ScrollBar;
        if (subcontrolString == QLatin1String("slider")) {
            subcontrol = QStyle::SC_ScrollBarSlider;
        } else if (subcontrolString == QLatin1String("groove")) {
            subcontrol = QStyle::SC_ScrollBarGroove;
        } else if (subcontrolString == QLatin1String("handle")) {
            subcontrol = QStyle::SC_ScrollBarSlider;
        } else if (subcontrolString == QLatin1String("add")) {
            subcontrol = QStyle::SC_ScrollBarAddPage;
        } else if (subcontrolString == QLatin1String("sub")) {
            subcontrol = QStyle::SC_ScrollBarSubPage;
        }
        return KQuickStyleItem::style()->subControlRect(control, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), subcontrol);
        break;
    }
    case ItemBranchIndicator: {
        QStyleOption opt;
        opt.rect = QRect(0, 0, implicitWidth(), implicitHeight());
        return KQuickStyleItem::style()->subElementRect(QStyle::SE_TreeViewDisclosureItem, &opt, nullptr);
    }
    default:
        break;
    }
    return QRectF();
}

void KQuickStyleItem::paint(QPainter *painter)
{
    initStyleOption();
    if (QStyleOptionMenuItem *opt = qstyleoption_cast<QStyleOptionMenuItem *>(m_styleoption)) {
        painter->setFont(opt->font);
    } else {
        QFont font;
        if (m_styleoption->state & QStyle::State_Mini) {
            font = qApp->font("QMiniFont");
        } else if (m_styleoption->state & QStyle::State_Small) {
            font = qApp->font("QSmallFont");
        } else {
            font = QApplication::font(classNameForItem());
        }
        painter->setFont(font);
    }

    switch (m_itemType) {
    case Button:
        KQuickStyleItem::style()->drawControl(QStyle::CE_PushButton, m_styleoption, painter);
        break;
    case ItemRow: {
        QPixmap pixmap;
        // Only draw through style once
        const QString pmKey = QLatin1String("itemrow") % QString::number(m_styleoption->state, 16) % activeControl();
        if (!QPixmapCache::find(pmKey, &pixmap) || pixmap.width() < width() || height() != pixmap.height()) {
            int newSize = width();
            pixmap = QPixmap(newSize, height());
            pixmap.fill(Qt::transparent);
            QPainter pixpainter(&pixmap);
            KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelItemViewRow, m_styleoption, &pixpainter);
            if ((styleName() == QLatin1String("mac") || !KQuickStyleItem::style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected)) && selected()) {
                QPalette pal = QApplication::palette("QAbstractItemView");
                pal.setCurrentColorGroup(m_styleoption->palette.currentColorGroup());
                pixpainter.fillRect(m_styleoption->rect, pal.highlight());
            }
            QPixmapCache::insert(pmKey, pixmap);
        }
        painter->drawPixmap(0, 0, pixmap);
        break;
    }
    case Item:
        KQuickStyleItem::style()->drawControl(QStyle::CE_ItemViewItem, m_styleoption, painter);
        break;
    case ItemBranchIndicator:
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_IndicatorBranch, m_styleoption, painter);
        break;
    case Header:
        KQuickStyleItem::style()->drawControl(QStyle::CE_Header, m_styleoption, painter);
        break;
    case ToolButton:
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_ToolButton, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), painter);
        break;
    case Tab: {
        if (!isKeyFocusReason(m_focusReason)) {
            m_styleoption->state &= ~QStyle::State_HasFocus;
        }
        KQuickStyleItem::style()->drawControl(QStyle::CE_TabBarTab, m_styleoption, painter);
        break;
    }
    case Frame:
        m_styleoption->state |= QStyle::State_Sunken;
        m_styleoption->state &= ~QStyle::State_Raised;
        KQuickStyleItem::style()->drawControl(QStyle::CE_ShapedFrame, m_styleoption, painter);
        break;
    case FocusFrame:
        KQuickStyleItem::style()->drawControl(QStyle::CE_FocusFrame, m_styleoption, painter);
        break;
    case FocusRect:
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_FrameFocusRect, m_styleoption, painter);
        break;
    case TabFrame:
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_FrameTabWidget, m_styleoption, painter);
        break;
    case MenuBar:
        KQuickStyleItem::style()->drawControl(QStyle::CE_MenuBarEmptyArea, m_styleoption, painter);
        break;
    case MenuBarItem:
        KQuickStyleItem::style()->drawControl(QStyle::CE_MenuBarItem, m_styleoption, painter);
        break;
    case MenuItem:
    case ComboBoxItem: { // fall through
        QStyle::ControlElement menuElement =
            static_cast<QStyleOptionMenuItem *>(m_styleoption)->menuItemType == QStyleOptionMenuItem::Scroller ? QStyle::CE_MenuScroller : QStyle::CE_MenuItem;
        KQuickStyleItem::style()->drawControl(menuElement, m_styleoption, painter);
        break;
    }
    case CheckBox:
        KQuickStyleItem::style()->drawControl(QStyle::CE_CheckBox, m_styleoption, painter);
        break;
    case RadioButton:
        KQuickStyleItem::style()->drawControl(QStyle::CE_RadioButton, m_styleoption, painter);
        break;
    case Edit:
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelLineEdit, m_styleoption, painter);
        break;
    case MacHelpButton:
        // Not managed as mac is not supported
        break;
    case Widget:
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_Widget, m_styleoption, painter);
        break;
    case ScrollAreaCorner:
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelScrollAreaCorner, m_styleoption, painter);
        break;
    case Splitter:
        KQuickStyleItem::style()->drawControl(QStyle::CE_Splitter, m_styleoption, painter);
        break;
    case ComboBox: {
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_ComboBox, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), painter);
        // This is needed on macOS as it will use the painter color and ignore the palette
        QPen pen = painter->pen();
        painter->setPen(m_styleoption->palette.text().color());
        KQuickStyleItem::style()->drawControl(QStyle::CE_ComboBoxLabel, m_styleoption, painter);
        painter->setPen(pen);
        break;
    }
    case SpinBox:
#ifdef Q_OS_MAC
        // macstyle depends on the embedded qlineedit to fill the editfield background
        if (styleName() == QLatin1String("mac")) {
            QRect editRect = KQuickStyleItem::style()->subControlRect(QStyle::CC_SpinBox,
                                                                      qstyleoption_cast<QStyleOptionComplex *>(m_styleoption),
                                                                      QStyle::SC_SpinBoxEditField);
            painter->fillRect(editRect.adjusted(-1, -1, 1, 1), m_styleoption->palette.base());
        }
#endif
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_SpinBox, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), painter);
        break;
    case Slider:
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_Slider, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), painter);
        break;
    case Dial:
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_Dial, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), painter);
        break;
    case ProgressBar:
        KQuickStyleItem::style()->drawControl(QStyle::CE_ProgressBar, m_styleoption, painter);
        break;
    case ToolBar:
        painter->fillRect(m_styleoption->rect, m_styleoption->palette.window().color());
        KQuickStyleItem::style()->drawControl(QStyle::CE_ToolBar, m_styleoption, painter);
        painter->save();
        painter->setPen(styleName() != QLatin1String("fusion") ? m_styleoption->palette.dark().color().darker(120)
                                                               : m_styleoption->palette.window().color().lighter(107));
        painter->drawLine(m_styleoption->rect.bottomLeft(), m_styleoption->rect.bottomRight());
        painter->restore();
        break;
    case StatusBar: {
        painter->fillRect(m_styleoption->rect, m_styleoption->palette.window().color());
        painter->setPen(m_styleoption->palette.dark().color().darker(120));
        painter->drawLine(m_styleoption->rect.topLeft(), m_styleoption->rect.topRight());
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelStatusBar, m_styleoption, painter);
        break;
    }
    case GroupBox:
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_GroupBox, qstyleoption_cast<QStyleOptionComplex *>(m_styleoption), painter);
        break;
    case ScrollBar:
        KQuickStyleItem::style()->drawComplexControl(QStyle::CC_ScrollBar, qstyleoption_cast<QStyleOptionSlider *>(m_styleoption), painter);
        break;
    case Menu: {
        QStyleHintReturnMask val;
        KQuickStyleItem::style()->styleHint(QStyle::SH_Menu_Mask, m_styleoption, nullptr, &val);
        painter->save();
        painter->setClipRegion(val.region);
        painter->fillRect(m_styleoption->rect, m_styleoption->palette.window());
        painter->restore();
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelMenu, m_styleoption, painter);

        if (int fw = KQuickStyleItem::style()->pixelMetric(QStyle::PM_MenuPanelWidth)) {
            QStyleOptionFrame frame;
            frame.state = QStyle::State_None;
            frame.lineWidth = fw;
            frame.midLineWidth = 0;
            frame.rect = m_styleoption->rect;
            frame.styleObject = this;
            frame.palette = m_styleoption->palette;
            KQuickStyleItem::style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, painter);
        }
        break;
    }
    case DelayButton: { // Adapted from Spectacle's ProgressButton made by David Redondo.
        // Draw Button without text and icon, note the missing text and icon in options
        QStyleOption baseStyleOptions = *m_styleoption;
        baseStyleOptions.state.setFlag(QStyle::State_Enabled, !baseStyleOptions.state.testFlag(QStyle::State_Sunken));
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &baseStyleOptions, painter);
        qreal progress = qreal(value()) / qreal(maximum());
        if (!qFuzzyIsNull(progress)) {
            // Draw overlay
            QStyleOption overlayOption;
            overlayOption.palette = m_styleoption->palette;
            overlayOption.palette.setBrush(QPalette::Button, m_styleoption->palette.highlight());
            overlayOption.rect = m_styleoption->direction == Qt::LeftToRight ?
                QRect(0, 0, m_styleoption->rect.width() * progress, m_styleoption->rect.height())
                : QRect(QPoint(m_styleoption->rect.width() * (1 - progress), 0), m_styleoption->rect.size());
            overlayOption.state.setFlag(QStyle::State_Sunken, m_styleoption->state.testFlag(QStyle::State_Sunken));
            painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
            painter->setOpacity(0.5);
            KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &overlayOption, painter);
        }
        // Finally draw text and icon and outline
        painter->setOpacity(1);
        KQuickStyleItem::style()->drawControl(QStyle::CE_ToolButtonLabel, m_styleoption, painter);
        break;
    }
    default:
        break;
    }
}

qreal KQuickStyleItem::textWidth(const QString &text)
{
    QFontMetricsF fm = QFontMetricsF(m_styleoption->fontMetrics);
    return fm.boundingRect(text).width();
}

qreal KQuickStyleItem::textHeight(const QString &text)
{
    QFontMetricsF fm = QFontMetricsF(m_styleoption->fontMetrics);
    return text.isEmpty() ? fm.height() : fm.boundingRect(text).height();
}

QString KQuickStyleItem::elidedText(const QString &text, int elideMode, int width)
{
    return m_styleoption->fontMetrics.elidedText(text, Qt::TextElideMode(elideMode), width);
}

bool KQuickStyleItem::hasThemeIcon(const QString &icon) const
{
    return QIcon::hasThemeIcon(icon);
}

void KQuickStyleItem::updateFocusReason()
{
    if (m_control) {
        auto reason = m_focusReasonProperty.read(m_control).value<Qt::FocusReason>();
        if (m_focusReason != reason) {
            m_focusReason = reason;
            polish();
        }
    }
}

bool KQuickStyleItem::event(QEvent *ev)
{
    if (ev->type() == QEvent::StyleAnimationUpdate) {
        if (isVisible()) {
            ev->accept();
            polish();
        }
        return true;
    } else if (ev->type() == Kirigami::Platform::TabletModeChangedEvent::type) {
        Q_EMIT leftPaddingChanged();
        Q_EMIT rightPaddingChanged();
        Q_EMIT topPaddingChanged();
        Q_EMIT bottomPaddingChanged();
        updateSizeHint();
        polish();
        return true;
    }

    return QQuickItem::event(ev);
}

void KQuickStyleItem::setTextureWidth(int w)
{
    if (m_textureWidth == w) {
        return;
    }
    m_textureWidth = w;
    Q_EMIT textureWidthChanged(m_textureWidth);
    update();
}

void KQuickStyleItem::setTextureHeight(int h)
{
    if (m_textureHeight == h) {
        return;
    }
    m_textureHeight = h;
    Q_EMIT textureHeightChanged(m_textureHeight);
    update();
}

QQuickItem *KQuickStyleItem::control() const
{
    return m_control;
}

void KQuickStyleItem::setControl(QQuickItem *control)
{
    if (control == m_control) {
        return;
    }

    if (m_control) {
        m_control->removeEventFilter(this);
        disconnect(m_control, nullptr, this, nullptr);
    }

    m_control = control;
    m_focusReasonProperty = {};
    m_focusReason = Qt::NoFocusReason;

    if (m_control) {
        // focusReasonChanged is a signal of QQuickControl class which is not exposed.
        const auto mo = m_control->metaObject();
        m_focusReasonProperty = mo->property(mo->indexOfProperty("focusReason"));
        if (m_focusReasonProperty.isValid()) {
            // wish there was something like QMetaMethod::fromSignal but for slots
            const auto slot = metaObject()->method(metaObject()->indexOfSlot("updateFocusReason()"));
            // also wish there was a way to connect QMetaMethod to arbitrary member function
            connect(m_control, m_focusReasonProperty.notifySignal(), this, slot);
            updateFocusReason();
        }

        m_control->installEventFilter(this);

        if (m_control->window()) {
            m_window = m_control->window();
            m_window->installEventFilter(this);
        }
        connect(m_control, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
            if (m_window) {
                m_window->removeEventFilter(this);
            }
            m_window = window;
            if (m_window) {
                m_window->installEventFilter(this);
            }
        });
    }

    Q_EMIT controlChanged();
}

QSGNode *KQuickStyleItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    if (m_image.isNull()) {
        delete node;
        return nullptr;
    }

    QSGNinePatchNode *styleNode = static_cast<QSGNinePatchNode *>(node);
    if (!styleNode) {
        styleNode = window()->createNinePatchNode();
    }

#ifdef QSG_RUNTIME_DESCRIPTION
    qsgnode_set_description(styleNode, QString::fromLatin1("%1:%2, '%3'").arg(styleName(), elementType(), text()));
#endif

    styleNode->setTexture(window()->createTextureFromImage(m_image, QQuickWindow::TextureCanUseAtlas));

    auto scale = window()->effectiveDevicePixelRatio();
    QRectF bounds = QRectF(boundingRect().topLeft(), QSizeF{m_image.width() / scale, m_image.height() / scale});

    QPointF globalPixelPos = mapToScene(bounds.topLeft()) * scale;
    QPointF posAdjust = QPointF(globalPixelPos.x() - std::round(globalPixelPos.x()), globalPixelPos.y() - std::round(globalPixelPos.y()))
        / scale;
    bounds.moveTopLeft(bounds.topLeft() - posAdjust);

    styleNode->setBounds(bounds);
    styleNode->setDevicePixelRatio(scale);
    styleNode->setPadding(m_border.left(), m_border.top(), m_border.right(), m_border.bottom());
    styleNode->update();

    return styleNode;
}

void KQuickStyleItem::updatePolish()
{
    if (isVisible() && width() >= 1 && height() >= 1) { // Note these are reals so 1 pixel is minimum
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 3)
        // Need to request the focus reason every time, the signal is not entirely reliable: https://bugreports.qt.io/browse/QTBUG-125725
        updateFocusReason();
#endif
        const qreal devicePixelRatio = window() ? window()->effectiveDevicePixelRatio() : qApp->devicePixelRatio();
        const QSize size = QSize(m_textureWidth > 0 ? m_textureWidth : width(), m_textureHeight > 0 ? m_textureHeight : height()) * devicePixelRatio;

        if (m_image.size() != size) {
            m_image = QImage(size, QImage::Format_ARGB32_Premultiplied);
        }
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

bool KQuickStyleItem::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_control) {
        // Page accepts mouse events without doing anything with them (for a workaround wrt dragging from empty areas of flickables) when the interaction is
        // pure mouse, steal events from them, so a parent handler can initiate a window drag from empty areas, either Kirigami.ApplicationWindow or the breeze
        // style from a QQwuickwidget
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->source() == Qt::MouseEventNotSynthesized && watched->inherits("QQuickPage")) {
                event->setAccepted(false);
                return true;
            }
        }
    } else if (watched == m_window.data()) {
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_Alt) {
                polish();
            }
        }
    } else if (watched == qGuiApp) {
        if (event->type() == QEvent::ApplicationFontChange) {
            QMetaObject::invokeMethod(this, &KQuickStyleItem::updateSizeHint, Qt::QueuedConnection);
        }
    }

    return QQuickItem::eventFilter(watched, event);
}

void KQuickStyleItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == QQuickItem::ItemVisibleHasChanged || change == QQuickItem::ItemEnabledHasChanged || change == QQuickItem::ItemDevicePixelRatioHasChanged) {
        polish();
    }

    QQuickItem::itemChange(change, value);
}

void KQuickStyleItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    if (!newGeometry.isEmpty() && newGeometry != oldGeometry) {
        polish();
        updateRect();

        if (newGeometry.height() != oldGeometry.height()) {
            updateBaselineOffset();
        }
    }
}

void KQuickStyleItem::styleChanged()
{
    // It should be safe to use qApp->style() unguarded here, because the signal
    // will only have been connected if qApp is a QApplication.
    Q_ASSERT(qobject_cast<QApplication *>(QCoreApplication::instance()));
    auto *style = qApp->style();
    if (!style || QCoreApplication::closingDown()) {
        return;
    }

    Q_ASSERT(style != sender());

    connect(style, &QObject::destroyed, this, &KQuickStyleItem::styleChanged);

    updateSizeHint();
    polish();
    Q_EMIT styleNameChanged();
}

QPixmap QQuickTableRowImageProvider1::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    int width = 16;
    int height = 16;
    if (size) {
        *size = QSize(width, height);
    }

    QPixmap pixmap(width, height);

    QStyleOptionViewItem opt;
    opt.state |= QStyle::State_Enabled;
    opt.rect = QRect(0, 0, width, height);
    QString style = QString::fromLatin1(KQuickStyleItem::style()->metaObject()->className());
    opt.features = {};

    if (id.contains(QLatin1String("selected"))) {
        opt.state |= QStyle::State_Selected;
    }

    if (id.contains(QLatin1String("active"))) {
        opt.state |= QStyle::State_Active;
        opt.palette.setCurrentColorGroup(QPalette::Active);
    } else {
        opt.palette.setCurrentColorGroup(QPalette::Inactive);
    }

    if (id.contains(QLatin1String("alternate"))) {
        opt.features |= QStyleOptionViewItem::Alternate;
    }

    QPalette pal = QApplication::palette("QAbstractItemView");
    if (opt.state & QStyle::State_Selected
        && (style.contains(QLatin1String("Mac")) || !KQuickStyleItem::style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected))) {
        pal.setCurrentColorGroup(opt.palette.currentColorGroup());
        pixmap.fill(pal.highlight().color());
    } else {
        pixmap.fill(pal.base().color());
        QPainter pixpainter(&pixmap);
        KQuickStyleItem::style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, &pixpainter);
    }
    return pixmap;
}

#include "moc_kquickpadding_p.cpp"
#include "moc_kquickstyleitem_p.cpp"
