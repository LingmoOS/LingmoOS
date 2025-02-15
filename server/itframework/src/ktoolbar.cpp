/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 1999 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktoolbar.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDomElement>
#include <QDrag>
#include <QFrame>
#include <QLayout>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointer>
#ifdef QT_DBUS_LIB
#include <QDBusConnection>
#include <QDBusMessage>
#endif

#include <KAuthorized>
#include <KConfig>
#include <KConfigGroup>
#include <KIconTheme>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KStandardAction>
#include <KToggleAction>

#include "kactioncollection.h"
#include "kedittoolbar.h"
#include "kxmlguifactory.h"
#include "kxmlguiwindow.h"

#include "ktoolbarhelper_p.h"

/*
 Toolbar settings (e.g. icon size or toolButtonStyle)
 =====================================================

 We have the following stack of settings (in order of priority) :
   - user-specified settings (loaded/saved in KConfig)
   - developer-specified settings in the XMLGUI file (if using xmlgui) (cannot change at runtime)
   - KDE-global default (user-configurable; can change at runtime)
 and when switching between kparts, they are saved as xml in memory,
 which, in the unlikely case of no-kmainwindow-autosaving, could be
 different from the user-specified settings saved in KConfig and would have
 priority over it.

 So, in summary, without XML:
   Global config / User settings (loaded/saved in kconfig)
 and with XML:
   Global config / App-XML attributes / User settings (loaded/saved in kconfig)

 And all those settings (except the KDE-global defaults) have to be stored in memory
 since we cannot retrieve them at random points in time, not knowing the xml document
 nor config file that holds these settings. Hence the iconSizeSettings and toolButtonStyleSettings arrays.

 For instance, if you change the KDE-global default, whether this makes a change
 on a given toolbar depends on whether there are settings at Level_AppXML or Level_UserSettings.
 Only if there are no settings at those levels, should the change of KDEDefault make a difference.
*/
enum SettingLevel { Level_KDEDefault, Level_AppXML, Level_UserSettings, NSettingLevels };
enum { Unset = -1 };

class KToolBarPrivate
{
public:
    KToolBarPrivate(KToolBar *qq)
        : q(qq)
        , isMainToolBar(false)
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
        , enableContext(true)
#endif
        , unlockedMovable(true)
        , contextOrient(nullptr)
        , contextMode(nullptr)
        , contextSize(nullptr)
        , contextButtonTitle(nullptr)
        , contextShowText(nullptr)
        , contextButtonAction(nullptr)
        , contextTop(nullptr)
        , contextLeft(nullptr)
        , contextRight(nullptr)
        , contextBottom(nullptr)
        , contextIcons(nullptr)
        , contextTextRight(nullptr)
        , contextText(nullptr)
        , contextTextUnder(nullptr)
        , contextLockAction(nullptr)
        , dropIndicatorAction(nullptr)
        , context(nullptr)
        , dragAction(nullptr)
    {
    }

    void slotAppearanceChanged();
    void slotContextAboutToShow();
    void slotContextAboutToHide();
    void slotContextLeft();
    void slotContextRight();
    void slotContextShowText();
    void slotContextTop();
    void slotContextBottom();
    void slotContextIcons();
    void slotContextText();
    void slotContextTextRight();
    void slotContextTextUnder();
    void slotContextIconSize(QAction *action);
    void slotLockToolBars(bool lock);

    void init(bool readConfig = true, bool isMainToolBar = false);
    QString getPositionAsString() const;
    QMenu *contextMenu(const QPoint &globalPos);
    void setLocked(bool locked);
    void adjustSeparatorVisibility();
    void loadKDESettings();
    void applyCurrentSettings();

    QAction *findAction(const QString &actionName, KXMLGUIClient **client = nullptr) const;

    static Qt::ToolButtonStyle toolButtonStyleFromString(const QString &style);
    static QString toolButtonStyleToString(Qt::ToolButtonStyle);
    static Qt::ToolBarArea positionFromString(const QString &position);
    static Qt::ToolButtonStyle toolButtonStyleSetting();

    KToolBar *const q;
    bool isMainToolBar : 1;
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
    bool enableContext : 1;
#endif
    bool unlockedMovable : 1;
    static bool s_editable;
    static bool s_locked;

    QSet<KXMLGUIClient *> xmlguiClients;

    QMenu *contextOrient;
    QMenu *contextMode;
    QMenu *contextSize;

    QAction *contextButtonTitle;
    QAction *contextShowText;
    QAction *contextButtonAction;
    QAction *contextTop;
    QAction *contextLeft;
    QAction *contextRight;
    QAction *contextBottom;
    QAction *contextIcons;
    QAction *contextTextRight;
    QAction *contextText;
    QAction *contextTextUnder;
    KToggleAction *contextLockAction;

    struct ContextIconInfo {
        QAction *iconAction = nullptr;
        int iconSize = 0;
    };

    std::vector<ContextIconInfo> m_contextIconSizes;

    class IntSetting
    {
    public:
        IntSetting()
        {
            for (int &value : values) {
                value = Unset;
            }
        }
        int currentValue() const
        {
            int val = Unset;
            for (int value : values) {
                if (value != Unset) {
                    val = value;
                }
            }
            return val;
        }
        // Default value as far as the user is concerned is kde-global + app-xml.
        // If currentValue()==defaultValue() then nothing to write into kconfig.
        int defaultValue() const
        {
            int val = Unset;
            for (int level = 0; level < Level_UserSettings; ++level) {
                if (values[level] != Unset) {
                    val = values[level];
                }
            }
            return val;
        }
        QString toString() const
        {
            QString str;
            for (int value : values) {
                str += QString::number(value) + QLatin1Char(' ');
            }
            return str;
        }
        int &operator[](int index)
        {
            return values[index];
        }

    private:
        int values[NSettingLevels];
    };
    IntSetting iconSizeSettings;
    IntSetting toolButtonStyleSettings; // either Qt::ToolButtonStyle or -1, hence "int".

    QList<QAction *> actionsBeingDragged;
    QAction *dropIndicatorAction;

    QMenu *context;
    QAction *dragAction;
    QPoint dragStartPosition;
};

bool KToolBarPrivate::s_editable = false;
bool KToolBarPrivate::s_locked = true;

void KToolBarPrivate::init(bool readConfig, bool _isMainToolBar)
{
    isMainToolBar = _isMainToolBar;
    loadKDESettings();

    // also read in our configurable settings (for non-xmlgui toolbars)
    if (readConfig) {
        KConfigGroup cg(KSharedConfig::openConfig(), QString());
        q->applySettings(cg);
    }

    if (q->mainWindow()) {
        // Get notified when settings change
        QObject::connect(q, &QToolBar::allowedAreasChanged, q->mainWindow(), &KMainWindow::setSettingsDirty);
        QObject::connect(q, &QToolBar::iconSizeChanged, q->mainWindow(), &KMainWindow::setSettingsDirty);
        QObject::connect(q, &QToolBar::toolButtonStyleChanged, q->mainWindow(), &KMainWindow::setSettingsDirty);
        QObject::connect(q, &QToolBar::movableChanged, q->mainWindow(), &KMainWindow::setSettingsDirty);
        QObject::connect(q, &QToolBar::orientationChanged, q->mainWindow(), &KMainWindow::setSettingsDirty);
    }

    if (!KAuthorized::authorize(QStringLiteral("movable_toolbars"))) {
        q->setMovable(false);
    } else {
        q->setMovable(!KToolBar::toolBarsLocked());
    }

    q->toggleViewAction()->setEnabled(KAuthorized::authorizeAction(QStringLiteral("options_show_toolbar")));

    QObject::connect(q, &QToolBar::movableChanged, q, &KToolBar::slotMovableChanged);

    q->setAcceptDrops(true);

#ifdef QT_DBUS_LIB
    QDBusConnection::sessionBus()
        .connect(QString(), QStringLiteral("/KToolBar"), QStringLiteral("org.kde.KToolBar"), QStringLiteral("styleChanged"), q, SLOT(slotAppearanceChanged()));
#endif
    QObject::connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, q, [this]() {
        slotAppearanceChanged();
    });
}

QString KToolBarPrivate::getPositionAsString() const
{
    if (!q->mainWindow()) {
        return QStringLiteral("None");
    }
    // get all of the stuff to save
    switch (q->mainWindow()->toolBarArea(const_cast<KToolBar *>(q))) {
    case Qt::BottomToolBarArea:
        return QStringLiteral("Bottom");
    case Qt::LeftToolBarArea:
        return QStringLiteral("Left");
    case Qt::RightToolBarArea:
        return QStringLiteral("Right");
    case Qt::TopToolBarArea:
    default:
        return QStringLiteral("Top");
    }
}

QMenu *KToolBarPrivate::contextMenu(const QPoint &globalPos)
{
    if (!context) {
        context = new QMenu(q);
        context->setIcon(QIcon::fromTheme(QStringLiteral("configure-toolbars")));
        context->setTitle(i18nc("@title:menu", "Toolbar Settings"));

        contextButtonTitle = context->addSection(i18nc("@title:menu", "Show Text"));
        contextShowText = context->addAction(QString(), q, [this]() {
            slotContextShowText();
        });

        context->addSection(i18nc("@title:menu", "Toolbar Settings"));

        contextOrient = new QMenu(i18nc("Toolbar orientation", "Orientation"), context);

        contextTop = contextOrient->addAction(i18nc("toolbar position string", "Top"), q, [this]() {
            slotContextTop();
        });
        contextTop->setChecked(true);
        contextLeft = contextOrient->addAction(i18nc("toolbar position string", "Left"), q, [this]() {
            slotContextLeft();
        });
        contextRight = contextOrient->addAction(i18nc("toolbar position string", "Right"), q, [this]() {
            slotContextRight();
        });
        contextBottom = contextOrient->addAction(i18nc("toolbar position string", "Bottom"), q, [this]() {
            slotContextBottom();
        });

        QActionGroup *positionGroup = new QActionGroup(contextOrient);
        const auto orientActions = contextOrient->actions();
        for (QAction *action : orientActions) {
            action->setActionGroup(positionGroup);
            action->setCheckable(true);
        }

        contextMode = new QMenu(i18n("Text Position"), context);

        contextIcons = contextMode->addAction(i18nc("@item:inmenu", "Icons Only"), q, [this]() {
            slotContextIcons();
        });
        contextText = contextMode->addAction(i18nc("@item:inmenu", "Text Only"), q, [this]() {
            slotContextText();
        });
        contextTextRight = contextMode->addAction(i18nc("@item:inmenu", "Text Alongside Icons"), q, [this]() {
            slotContextTextRight();
        });
        contextTextUnder = contextMode->addAction(i18nc("@item:inmenu", "Text Under Icons"), q, [this]() {
            slotContextTextUnder();
        });

        QActionGroup *textGroup = new QActionGroup(contextMode);
        const auto modeActions = contextMode->actions();
        for (QAction *action : modeActions) {
            action->setActionGroup(textGroup);
            action->setCheckable(true);
        }

        contextSize = new QMenu(i18n("Icon Size"), context);

        auto *act = contextSize->addAction(i18nc("@item:inmenu Icon size", "Default"));
        q->connect(act, &QAction::triggered, q, [this, act]() {
            slotContextIconSize(act);
        });
        m_contextIconSizes.push_back({act, iconSizeSettings.defaultValue()});

        // Query the current theme for available sizes
        KIconTheme *theme = KIconLoader::global()->theme();
        QList<int> avSizes;
        if (theme) {
            avSizes = theme->querySizes(isMainToolBar ? KIconLoader::MainToolbar : KIconLoader::Toolbar);
        }

        std::sort(avSizes.begin(), avSizes.end());

        if (avSizes.count() < 10) {
            // Fixed or threshold type icons
            for (int it : std::as_const(avSizes)) {
                QString text;
                if (it < 19) {
                    text = i18n("Small (%1x%2)", it, it);
                } else if (it < 25) {
                    text = i18n("Medium (%1x%2)", it, it);
                } else if (it < 35) {
                    text = i18n("Large (%1x%2)", it, it);
                } else {
                    text = i18n("Huge (%1x%2)", it, it);
                }

                auto *act = contextSize->addAction(text);
                q->connect(act, &QAction::triggered, q, [this, act]() {
                    slotContextIconSize(act);
                });
                m_contextIconSizes.push_back({act, it});
            }
        } else {
            // Scalable icons.
            const int progression[] = {16, 22, 32, 48, 64, 96, 128, 192, 256};

            for (int p : progression) {
                for (int it : std::as_const(avSizes)) {
                    if (it >= p) {
                        QString text;
                        if (it < 19) {
                            text = i18n("Small (%1x%2)", it, it);
                        } else if (it < 25) {
                            text = i18n("Medium (%1x%2)", it, it);
                        } else if (it < 35) {
                            text = i18n("Large (%1x%2)", it, it);
                        } else {
                            text = i18n("Huge (%1x%2)", it, it);
                        }

                        auto *act = contextSize->addAction(text);
                        q->connect(act, &QAction::triggered, q, [this, act]() {
                            slotContextIconSize(act);
                        });
                        m_contextIconSizes.push_back({act, it});
                        break;
                    }
                }
            }
        }

        QActionGroup *sizeGroup = new QActionGroup(contextSize);
        const auto sizeActions = contextSize->actions();
        for (QAction *action : sizeActions) {
            action->setActionGroup(sizeGroup);
            action->setCheckable(true);
        }

        if (!q->toolBarsLocked() && !q->isMovable()) {
            unlockedMovable = false;
        }

        delete contextLockAction;
        contextLockAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("system-lock-screen")), i18n("Lock Toolbar Positions"), q);
        contextLockAction->setChecked(q->toolBarsLocked());
        QObject::connect(contextLockAction, &KToggleAction::toggled, q, [this](bool checked) {
            slotLockToolBars(checked);
        });

        // Now add the actions to the menu
        context->addMenu(contextMode);
        context->addMenu(contextSize);
        context->addMenu(contextOrient);
        context->addSeparator();

        QObject::connect(context, &QMenu::aboutToShow, q, [this]() {
            slotContextAboutToShow();
        });
    }

    contextButtonAction = q->actionAt(q->mapFromGlobal(globalPos));
    if (contextButtonAction) {
        contextShowText->setText(contextButtonAction->text());
        contextShowText->setIcon(contextButtonAction->icon());
        contextShowText->setCheckable(true);
    }

    contextOrient->menuAction()->setVisible(!q->toolBarsLocked());
    // Unplugging a submenu from abouttohide leads to the popupmenu floating around
    // So better simply call that code from after exec() returns (DF)
    // connect(context, SIGNAL(aboutToHide()), this, SLOT(slotContextAboutToHide()));

    // For tool buttons with delay popup or menu button popup (split button)
    // show the actions of that button for ease of access.
    // (no need to wait for the menu to open or aim at the arrow)
    if (auto *contextToolButton = qobject_cast<QToolButton *>(q->widgetForAction(contextButtonAction))) {
        if (contextToolButton->popupMode() == QToolButton::DelayedPopup || contextToolButton->popupMode() == QToolButton::MenuButtonPopup) {
            if (auto *actionMenu = contextButtonAction->menu()) {
                // In case it is populated on demand
                Q_EMIT actionMenu->aboutToShow();

                auto *contextMenu = new QMenu(q);
                contextMenu->setAttribute(Qt::WA_DeleteOnClose);

                const auto actions = actionMenu->actions();
                if (!actions.isEmpty()) {
                    for (QAction *action : actions) {
                        contextMenu->addAction(action);
                    }

                    // Now add the configure actions as submenu
                    contextMenu->addSeparator();
                    contextMenu->addMenu(context);
                    return contextMenu;
                }
            }
        }
    }

    return context;
}

void KToolBarPrivate::setLocked(bool locked)
{
    if (unlockedMovable) {
        q->setMovable(!locked);
    }
}

void KToolBarPrivate::adjustSeparatorVisibility()
{
    bool visibleNonSeparator = false;
    int separatorToShow = -1;

    for (int index = 0; index < q->actions().count(); ++index) {
        QAction *action = q->actions().at(index);
        if (action->isSeparator()) {
            if (visibleNonSeparator) {
                separatorToShow = index;
                visibleNonSeparator = false;
            } else {
                action->setVisible(false);
            }
        } else if (!visibleNonSeparator) {
            if (action->isVisible()) {
                visibleNonSeparator = true;
                if (separatorToShow != -1) {
                    q->actions().at(separatorToShow)->setVisible(true);
                    separatorToShow = -1;
                }
            }
        }
    }

    if (separatorToShow != -1) {
        q->actions().at(separatorToShow)->setVisible(false);
    }
}

Qt::ToolButtonStyle KToolBarPrivate::toolButtonStyleFromString(const QString &_style)
{
    QString style = _style.toLower();
    if (style == QLatin1String("textbesideicon") || style == QLatin1String("icontextright")) {
        return Qt::ToolButtonTextBesideIcon;
    } else if (style == QLatin1String("textundericon") || style == QLatin1String("icontextbottom")) {
        return Qt::ToolButtonTextUnderIcon;
    } else if (style == QLatin1String("textonly")) {
        return Qt::ToolButtonTextOnly;
    } else {
        return Qt::ToolButtonIconOnly;
    }
}

QString KToolBarPrivate::toolButtonStyleToString(Qt::ToolButtonStyle style)
{
    switch (style) {
    case Qt::ToolButtonIconOnly:
    default:
        return QStringLiteral("IconOnly");
    case Qt::ToolButtonTextBesideIcon:
        return QStringLiteral("TextBesideIcon");
    case Qt::ToolButtonTextOnly:
        return QStringLiteral("TextOnly");
    case Qt::ToolButtonTextUnderIcon:
        return QStringLiteral("TextUnderIcon");
    }
}

Qt::ToolBarArea KToolBarPrivate::positionFromString(const QString &position)
{
    Qt::ToolBarArea newposition = Qt::TopToolBarArea;
    if (position == QLatin1String("left")) {
        newposition = Qt::LeftToolBarArea;
    } else if (position == QLatin1String("bottom")) {
        newposition = Qt::BottomToolBarArea;
    } else if (position == QLatin1String("right")) {
        newposition = Qt::RightToolBarArea;
    } else if (position == QLatin1String("none")) {
        newposition = Qt::NoToolBarArea;
    }
    return newposition;
}

// Global setting was changed
void KToolBarPrivate::slotAppearanceChanged()
{
    loadKDESettings();
    applyCurrentSettings();
}

Qt::ToolButtonStyle KToolBarPrivate::toolButtonStyleSetting()
{
    KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
    const QString fallback = KToolBarPrivate::toolButtonStyleToString(Qt::ToolButtonTextBesideIcon);
    return KToolBarPrivate::toolButtonStyleFromString(group.readEntry("ToolButtonStyle", fallback));
}

void KToolBarPrivate::loadKDESettings()
{
    iconSizeSettings[Level_KDEDefault] = q->iconSizeDefault();

    if (isMainToolBar) {
        toolButtonStyleSettings[Level_KDEDefault] = toolButtonStyleSetting();
    } else {
        const QString fallBack = toolButtonStyleToString(Qt::ToolButtonTextBesideIcon);
        /**
          TODO: if we get complaints about text beside icons on small screens,
                try the following code out on such systems - aseigo.
        // if we are on a small screen with a non-landscape ratio, then
        // we revert to text under icons since width is probably not our
        // friend in such cases
        QDesktopWidget *desktop = QApplication::desktop();
        QRect screenGeom = desktop->screenGeometry(desktop->primaryScreen());
        qreal ratio = screenGeom.width() / qreal(screenGeom.height());

        if (screenGeom.width() < 1024 && ratio <= 1.4) {
            fallBack = "TextUnderIcon";
        }
        **/

        KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
        const QString value = group.readEntry("ToolButtonStyleOtherToolbars", fallBack);
        toolButtonStyleSettings[Level_KDEDefault] = KToolBarPrivate::toolButtonStyleFromString(value);
    }
}

// Call this after changing something in d->iconSizeSettings or d->toolButtonStyleSettings
void KToolBarPrivate::applyCurrentSettings()
{
    // qCDebug(DEBUG_KXMLGUI) << q->objectName() << "iconSizeSettings:" << iconSizeSettings.toString() << "->" << iconSizeSettings.currentValue();
    const int currentIconSize = iconSizeSettings.currentValue();
    q->setIconSize(QSize(currentIconSize, currentIconSize));
    // qCDebug(DEBUG_KXMLGUI) << q->objectName() << "toolButtonStyleSettings:" << toolButtonStyleSettings.toString() << "->" <<
    // toolButtonStyleSettings.currentValue();
    q->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(toolButtonStyleSettings.currentValue()));

    // And remember to save the new look later
    KMainWindow *kmw = q->mainWindow();
    if (kmw) {
        kmw->setSettingsDirty();
    }
}

QAction *KToolBarPrivate::findAction(const QString &actionName, KXMLGUIClient **clientOut) const
{
    for (KXMLGUIClient *client : xmlguiClients) {
        QAction *action = client->actionCollection()->action(actionName);
        if (action) {
            if (clientOut) {
                *clientOut = client;
            }
            return action;
        }
    }
    return nullptr;
}

void KToolBarPrivate::slotContextAboutToShow()
{
    /**
     * The idea here is to reuse the "static" part of the menu to save time.
     * But the "Toolbars" action is dynamic (can be a single action or a submenu)
     * and ToolBarHandler::setupActions() deletes it, so better not keep it around.
     * So we currently plug/unplug the last two actions of the menu.
     * Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
     */

    KXmlGuiWindow *kmw = qobject_cast<KXmlGuiWindow *>(q->mainWindow());

    // try to find "configure toolbars" action
    QAction *configureAction = nullptr;
    const char *actionName = KStandardAction::name(KStandardAction::ConfigureToolbars);
    configureAction = findAction(QLatin1String(actionName));

    if (!configureAction && kmw) {
        configureAction = kmw->actionCollection()->action(QLatin1String(actionName));
    }

    if (configureAction) {
        context->addAction(configureAction);
    }

    context->addAction(contextLockAction);

    if (kmw) {
        kmw->setupToolbarMenuActions();
        // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
        QAction *tbAction = kmw->toolBarMenuAction();
        if (!q->toolBarsLocked() && tbAction && !tbAction->associatedWidgets().isEmpty()) {
            context->addAction(tbAction);
        }
    }

    KEditToolBar::setGlobalDefaultToolBar(q->QObject::objectName().toLatin1().constData());

    // Check the actions that should be checked
    switch (q->toolButtonStyle()) {
    case Qt::ToolButtonIconOnly:
    default:
        contextIcons->setChecked(true);
        break;
    case Qt::ToolButtonTextBesideIcon:
        contextTextRight->setChecked(true);
        break;
    case Qt::ToolButtonTextOnly:
        contextText->setChecked(true);
        break;
    case Qt::ToolButtonTextUnderIcon:
        contextTextUnder->setChecked(true);
        break;
    }

    auto it = std::find_if(m_contextIconSizes.cbegin(), m_contextIconSizes.cend(), [this](const ContextIconInfo &info) {
        return info.iconSize == q->iconSize().width();
    });
    if (it != m_contextIconSizes.cend()) {
        it->iconAction->setChecked(true);
    }

    switch (q->mainWindow()->toolBarArea(q)) {
    case Qt::BottomToolBarArea:
        contextBottom->setChecked(true);
        break;
    case Qt::LeftToolBarArea:
        contextLeft->setChecked(true);
        break;
    case Qt::RightToolBarArea:
        contextRight->setChecked(true);
        break;
    default:
    case Qt::TopToolBarArea:
        contextTop->setChecked(true);
        break;
    }

    const bool showButtonSettings = contextButtonAction //
        && !contextShowText->text().isEmpty() //
        && contextTextRight->isChecked();
    contextButtonTitle->setVisible(showButtonSettings);
    contextShowText->setVisible(showButtonSettings);
    if (showButtonSettings) {
        contextShowText->setChecked(contextButtonAction->priority() >= QAction::NormalPriority);
    }
}

void KToolBarPrivate::slotContextAboutToHide()
{
    // We have to unplug whatever slotContextAboutToShow plugged into the menu.
    // Unplug the toolbar menu action
    KXmlGuiWindow *kmw = qobject_cast<KXmlGuiWindow *>(q->mainWindow());
    if (kmw && kmw->toolBarMenuAction()) {
        if (kmw->toolBarMenuAction()->associatedWidgets().count() > 1) {
            context->removeAction(kmw->toolBarMenuAction());
        }
    }

    // Unplug the configure toolbars action too, since it's afterwards anyway
    QAction *configureAction = nullptr;
    const char *actionName = KStandardAction::name(KStandardAction::ConfigureToolbars);
    configureAction = findAction(QLatin1String(actionName));

    if (!configureAction && kmw) {
        configureAction = kmw->actionCollection()->action(QLatin1String(actionName));
    }

    if (configureAction) {
        context->removeAction(configureAction);
    }

    context->removeAction(contextLockAction);
}

void KToolBarPrivate::slotContextLeft()
{
    q->mainWindow()->addToolBar(Qt::LeftToolBarArea, q);
}

void KToolBarPrivate::slotContextRight()
{
    q->mainWindow()->addToolBar(Qt::RightToolBarArea, q);
}

void KToolBarPrivate::slotContextShowText()
{
    Q_ASSERT(contextButtonAction);
    const QAction::Priority priority = contextShowText->isChecked() ? QAction::NormalPriority : QAction::LowPriority;
    contextButtonAction->setPriority(priority);

    // Find to which xml file and componentData the action belongs to
    QString componentName;
    QString filename;
    KXMLGUIClient *client;
    if (findAction(contextButtonAction->objectName(), &client)) {
        componentName = client->componentName();
        filename = client->xmlFile();
    }
    if (filename.isEmpty()) {
        componentName = QCoreApplication::applicationName();
        filename = componentName + QLatin1String("ui.rc");
    }

    // Save the priority state of the action
    const QString configFile = KXMLGUIFactory::readConfigFile(filename, componentName);

    QDomDocument document;
    document.setContent(configFile);
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement(document);
    QDomElement actionElem = KXMLGUIFactory::findActionByName(elem, contextButtonAction->objectName(), true);
    actionElem.setAttribute(QStringLiteral("priority"), priority);
    KXMLGUIFactory::saveConfigFile(document, filename, componentName);
}

void KToolBarPrivate::slotContextTop()
{
    q->mainWindow()->addToolBar(Qt::TopToolBarArea, q);
}

void KToolBarPrivate::slotContextBottom()
{
    q->mainWindow()->addToolBar(Qt::BottomToolBarArea, q);
}

void KToolBarPrivate::slotContextIcons()
{
    q->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBarPrivate::slotContextText()
{
    q->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBarPrivate::slotContextTextUnder()
{
    q->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBarPrivate::slotContextTextRight()
{
    q->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButtonStyleSettings[Level_UserSettings] = q->toolButtonStyle();
}

void KToolBarPrivate::slotContextIconSize(QAction *action)
{
    if (action) {
        auto it = std::find_if(m_contextIconSizes.cbegin(), m_contextIconSizes.cend(), [action](const ContextIconInfo &info) {
            return info.iconAction == action;
        });
        if (it != m_contextIconSizes.cend()) {
            q->setIconDimensions(it->iconSize);
        }
    }
}

void KToolBarPrivate::slotLockToolBars(bool lock)
{
    q->setToolBarsLocked(lock);
}

KToolBar::KToolBar(QWidget *parent, bool isMainToolBar, bool readConfig)
    : QToolBar(parent)
    , d(new KToolBarPrivate(this))
{
    d->init(readConfig, isMainToolBar);

    // KToolBar is auto-added to the top area of the main window if parent is a QMainWindow
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(parent)) {
        mw->addToolBar(this);
    }
}

KToolBar::KToolBar(const QString &objectName, QWidget *parent, bool readConfig)
    : QToolBar(parent)
    , d(new KToolBarPrivate(this))
{
    setObjectName(objectName);
    // mainToolBar -> isMainToolBar = true  -> buttonStyle is configurable
    // others      -> isMainToolBar = false -> ### hardcoded default for buttonStyle !!! should be configurable? -> hidden key added
    d->init(readConfig, (objectName == QLatin1String("mainToolBar")));

    // KToolBar is auto-added to the top area of the main window if parent is a QMainWindow
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(parent)) {
        mw->addToolBar(this);
    }
}

KToolBar::KToolBar(const QString &objectName, QMainWindow *parent, Qt::ToolBarArea area, bool newLine, bool isMainToolBar, bool readConfig)
    : QToolBar(parent)
    , d(new KToolBarPrivate(this))
{
    setObjectName(objectName);
    d->init(readConfig, isMainToolBar);

    if (newLine) {
        mainWindow()->addToolBarBreak(area);
    }

    mainWindow()->addToolBar(area, this);

    if (newLine) {
        mainWindow()->addToolBarBreak(area);
    }
}

KToolBar::~KToolBar()
{
    delete d->contextLockAction;
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
void KToolBar::setContextMenuEnabled(bool enable)
{
    d->enableContext = enable;
}
#endif

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
bool KToolBar::contextMenuEnabled() const
{
    return d->enableContext;
}
#endif

void KToolBar::saveSettings(KConfigGroup &cg)
{
    Q_ASSERT(!cg.name().isEmpty());

    const int currentIconSize = iconSize().width();
    // qCDebug(DEBUG_KXMLGUI) << objectName() << currentIconSize << d->iconSizeSettings.toString() << "defaultValue=" << d->iconSizeSettings.defaultValue();
    if (!cg.hasDefault("IconSize") && currentIconSize == d->iconSizeSettings.defaultValue()) {
        cg.revertToDefault("IconSize");
        d->iconSizeSettings[Level_UserSettings] = Unset;
    } else {
        cg.writeEntry("IconSize", currentIconSize);
        d->iconSizeSettings[Level_UserSettings] = currentIconSize;
    }

    const Qt::ToolButtonStyle currentToolButtonStyle = toolButtonStyle();
    if (!cg.hasDefault("ToolButtonStyle") && currentToolButtonStyle == d->toolButtonStyleSettings.defaultValue()) {
        cg.revertToDefault("ToolButtonStyle");
        d->toolButtonStyleSettings[Level_UserSettings] = Unset;
    } else {
        cg.writeEntry("ToolButtonStyle", d->toolButtonStyleToString(currentToolButtonStyle));
        d->toolButtonStyleSettings[Level_UserSettings] = currentToolButtonStyle;
    }
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
void KToolBar::setXMLGUIClient(KXMLGUIClient *client)
{
    d->xmlguiClients.clear();
    d->xmlguiClients << client;
}
#endif

void KToolBar::addXMLGUIClient(KXMLGUIClient *client)
{
    d->xmlguiClients << client;
}

void KToolBar::removeXMLGUIClient(KXMLGUIClient *client)
{
    d->xmlguiClients.remove(client);
}

void KToolBar::contextMenuEvent(QContextMenuEvent *event)
{
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
    if (mainWindow() && d->enableContext) {
        QPointer<KToolBar> guard(this);
        const QPoint globalPos = event->globalPos();
        d->contextMenu(globalPos)->exec(globalPos);

        // "Configure Toolbars" recreates toolbars, so we might not exist anymore.
        if (guard) {
            d->slotContextAboutToHide();
        }
        return;
    }
#endif

    QToolBar::contextMenuEvent(event);
}

void KToolBar::loadState(const QDomElement &element)
{
    QMainWindow *mw = mainWindow();
    if (!mw) {
        return;
    }

    {
        const QString &i18nText = KToolbarHelper::i18nToolBarName(element);
        if (!i18nText.isEmpty()) {
            setWindowTitle(i18nText);
        }
    }

    /*
      This method is called in order to load toolbar settings from XML.
      However this can be used in two rather different cases:
      - for the initial loading of the app's XML. In that case the settings
      are only the defaults (Level_AppXML), the user's KConfig settings will override them

      - for later re-loading when switching between parts in KXMLGUIFactory.
      In that case the XML contains the final settings, not the defaults.
      We do need the defaults, and the toolbar might have been completely
      deleted and recreated meanwhile. So we store the app-default settings
      into the XML.
    */
    bool loadingAppDefaults = true;
    if (element.hasAttribute(QStringLiteral("tempXml"))) {
        // this isn't the first time, so the app-xml defaults have been saved into the (in-memory) XML
        loadingAppDefaults = false;
        const QString iconSizeDefault = element.attribute(QStringLiteral("iconSizeDefault"));
        if (!iconSizeDefault.isEmpty()) {
            d->iconSizeSettings[Level_AppXML] = iconSizeDefault.toInt();
        }
        const QString toolButtonStyleDefault = element.attribute(QStringLiteral("toolButtonStyleDefault"));
        if (!toolButtonStyleDefault.isEmpty()) {
            d->toolButtonStyleSettings[Level_AppXML] = d->toolButtonStyleFromString(toolButtonStyleDefault);
        }
    } else {
        // loading app defaults
        bool newLine = false;
        QString attrNewLine = element.attribute(QStringLiteral("newline")).toLower();
        if (!attrNewLine.isEmpty()) {
            newLine = (attrNewLine == QLatin1String("true"));
        }
        if (newLine && mw) {
            mw->insertToolBarBreak(this);
        }
    }

    int newIconSize = -1;
    if (element.hasAttribute(QStringLiteral("iconSize"))) {
        bool ok;
        newIconSize = element.attribute(QStringLiteral("iconSize")).trimmed().toInt(&ok);
        if (!ok) {
            newIconSize = -1;
        }
    }
    if (newIconSize != -1) {
        d->iconSizeSettings[loadingAppDefaults ? Level_AppXML : Level_UserSettings] = newIconSize;
    }

    const QString newToolButtonStyle = element.attribute(QStringLiteral("iconText"));
    if (!newToolButtonStyle.isEmpty()) {
        d->toolButtonStyleSettings[loadingAppDefaults ? Level_AppXML : Level_UserSettings] = d->toolButtonStyleFromString(newToolButtonStyle);
    }

    bool hidden = false;
    {
        QString attrHidden = element.attribute(QStringLiteral("hidden")).toLower();
        if (!attrHidden.isEmpty()) {
            hidden = (attrHidden == QLatin1String("true"));
        }
    }

    Qt::ToolBarArea pos = Qt::NoToolBarArea;
    {
        QString attrPosition = element.attribute(QStringLiteral("position")).toLower();
        if (!attrPosition.isEmpty()) {
            pos = KToolBarPrivate::positionFromString(attrPosition);
        }
    }
    if (pos != Qt::NoToolBarArea) {
        mw->addToolBar(pos, this);
    }

    setVisible(!hidden);

    d->applyCurrentSettings();
}

// Called when switching between xmlgui clients, in order to find any unsaved settings
// again when switching back to the current xmlgui client.
void KToolBar::saveState(QDomElement &current) const
{
    Q_ASSERT(!current.isNull());

    current.setAttribute(QStringLiteral("tempXml"), QStringLiteral("true"));

    current.setAttribute(QStringLiteral("noMerge"), QStringLiteral("1"));
    current.setAttribute(QStringLiteral("position"), d->getPositionAsString().toLower());
    current.setAttribute(QStringLiteral("hidden"), isHidden() ? QStringLiteral("true") : QStringLiteral("false"));

    const int currentIconSize = iconSize().width();
    if (currentIconSize == d->iconSizeSettings.defaultValue()) {
        current.removeAttribute(QStringLiteral("iconSize"));
    } else {
        current.setAttribute(QStringLiteral("iconSize"), iconSize().width());
    }

    if (toolButtonStyle() == d->toolButtonStyleSettings.defaultValue()) {
        current.removeAttribute(QStringLiteral("iconText"));
    } else {
        current.setAttribute(QStringLiteral("iconText"), d->toolButtonStyleToString(toolButtonStyle()));
    }

    // Note: if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
    // then the stuff below shouldn't always be done. This is not the case currently though.
    if (d->iconSizeSettings[Level_AppXML] != Unset) {
        current.setAttribute(QStringLiteral("iconSizeDefault"), d->iconSizeSettings[Level_AppXML]);
    }
    if (d->toolButtonStyleSettings[Level_AppXML] != Unset) {
        const Qt::ToolButtonStyle bs = static_cast<Qt::ToolButtonStyle>(d->toolButtonStyleSettings[Level_AppXML]);
        current.setAttribute(QStringLiteral("toolButtonStyleDefault"), d->toolButtonStyleToString(bs));
    }
}

// called by KMainWindow::applyMainWindowSettings to read from the user settings
void KToolBar::applySettings(const KConfigGroup &cg)
{
    Q_ASSERT(!cg.name().isEmpty());

    if (cg.hasKey("IconSize")) {
        d->iconSizeSettings[Level_UserSettings] = cg.readEntry("IconSize", 0);
    }
    if (cg.hasKey("ToolButtonStyle")) {
        d->toolButtonStyleSettings[Level_UserSettings] = d->toolButtonStyleFromString(cg.readEntry("ToolButtonStyle", QString()));
    }

    d->applyCurrentSettings();
}

KMainWindow *KToolBar::mainWindow() const
{
    return qobject_cast<KMainWindow *>(const_cast<QObject *>(parent()));
}

void KToolBar::setIconDimensions(int size)
{
    QToolBar::setIconSize(QSize(size, size));
    d->iconSizeSettings[Level_UserSettings] = size;
}

int KToolBar::iconSizeDefault() const
{
    return KIconLoader::global()->currentSize(d->isMainToolBar ? KIconLoader::MainToolbar : KIconLoader::Toolbar);
}

void KToolBar::slotMovableChanged(bool movable)
{
    if (movable && !KAuthorized::authorize(QStringLiteral("movable_toolbars"))) {
        setMovable(false);
    }
}

void KToolBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (toolBarsEditable() && event->proposedAction() & (Qt::CopyAction | Qt::MoveAction)
        && event->mimeData()->hasFormat(QStringLiteral("application/x-kde-action-list"))) {
        QByteArray data = event->mimeData()->data(QStringLiteral("application/x-kde-action-list"));

        QDataStream stream(data);

        QStringList actionNames;

        stream >> actionNames;

        const auto allCollections = KActionCollection::allCollections();
        for (const QString &actionName : std::as_const(actionNames)) {
            for (KActionCollection *ac : allCollections) {
                QAction *newAction = ac->action(actionName);
                if (newAction) {
                    d->actionsBeingDragged.append(newAction);
                    break;
                }
            }
        }

        if (!d->actionsBeingDragged.isEmpty()) {
            QAction *overAction = actionAt(event->pos());

            QFrame *dropIndicatorWidget = new QFrame(this);
            dropIndicatorWidget->resize(8, height() - 4);
            dropIndicatorWidget->setFrameShape(QFrame::VLine);
            dropIndicatorWidget->setLineWidth(3);

            d->dropIndicatorAction = insertWidget(overAction, dropIndicatorWidget);

            insertAction(overAction, d->dropIndicatorAction);

            event->acceptProposedAction();
            return;
        }
    }

    QToolBar::dragEnterEvent(event);
}

void KToolBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (toolBarsEditable()) {
        Q_FOREVER {
            if (d->dropIndicatorAction) {
                QAction *overAction = nullptr;
                const auto actions = this->actions();
                for (QAction *action : actions) {
                    // want to make it feel that half way across an action you're dropping on the other side of it
                    QWidget *widget = widgetForAction(action);
                    if (event->pos().x() < widget->pos().x() + (widget->width() / 2)) {
                        overAction = action;
                        break;
                    }
                }

                if (overAction != d->dropIndicatorAction) {
                    // Check to see if the indicator is already in the right spot
                    int dropIndicatorIndex = actions.indexOf(d->dropIndicatorAction);
                    if (dropIndicatorIndex + 1 < actions.count()) {
                        if (actions.at(dropIndicatorIndex + 1) == overAction) {
                            break;
                        }
                    } else if (!overAction) {
                        break;
                    }

                    insertAction(overAction, d->dropIndicatorAction);
                }

                event->accept();
                return;
            }
            break;
        }
    }

    QToolBar::dragMoveEvent(event);
}

void KToolBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    // Want to clear this even if toolBarsEditable was changed mid-drag (unlikely)
    delete d->dropIndicatorAction;
    d->dropIndicatorAction = nullptr;
    d->actionsBeingDragged.clear();

    if (toolBarsEditable()) {
        event->accept();
        return;
    }

    QToolBar::dragLeaveEvent(event);
}

void KToolBar::dropEvent(QDropEvent *event)
{
    if (toolBarsEditable()) {
        for (QAction *action : std::as_const(d->actionsBeingDragged)) {
            if (actions().contains(action)) {
                removeAction(action);
            }
            insertAction(d->dropIndicatorAction, action);
        }
    }

    // Want to clear this even if toolBarsEditable was changed mid-drag (unlikely)
    delete d->dropIndicatorAction;
    d->dropIndicatorAction = nullptr;
    d->actionsBeingDragged.clear();

    if (toolBarsEditable()) {
        event->accept();
        return;
    }

    QToolBar::dropEvent(event);
}

void KToolBar::mousePressEvent(QMouseEvent *event)
{
    if (toolBarsEditable() && event->button() == Qt::LeftButton) {
        if (QAction *action = actionAt(event->pos())) {
            d->dragAction = action;
            d->dragStartPosition = event->pos();
            event->accept();
            return;
        }
    }

    QToolBar::mousePressEvent(event);
}

void KToolBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!toolBarsEditable() || !d->dragAction) {
        QToolBar::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - d->dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        event->accept();
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);

        QStringList actionNames;
        actionNames << d->dragAction->objectName();

        stream << actionNames;
    }

    mimeData->setData(QStringLiteral("application/x-kde-action-list"), data);

    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

    if (dropAction == Qt::MoveAction) {
        // Only remove from this toolbar if it was moved to another toolbar
        // Otherwise the receiver moves it.
        if (drag->target() != this) {
            removeAction(d->dragAction);
        }
    }

    d->dragAction = nullptr;
    event->accept();
}

void KToolBar::mouseReleaseEvent(QMouseEvent *event)
{
    // Want to clear this even if toolBarsEditable was changed mid-drag (unlikely)
    if (d->dragAction) {
        d->dragAction = nullptr;
        event->accept();
        return;
    }

    QToolBar::mouseReleaseEvent(event);
}

bool KToolBar::eventFilter(QObject *watched, QEvent *event)
{
    // Generate context menu events for disabled buttons too...
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->buttons() & Qt::RightButton) {
            if (QWidget *ww = qobject_cast<QWidget *>(watched)) {
                if (ww->parent() == this && !ww->isEnabled()) {
                    QCoreApplication::postEvent(this, new QContextMenuEvent(QContextMenuEvent::Mouse, me->pos(), me->globalPos()));
                }
            }
        }

    } else if (event->type() == QEvent::ParentChange) {
        // Make sure we're not leaving stale event filters around,
        // when a child is reparented somewhere else
        if (QWidget *ww = qobject_cast<QWidget *>(watched)) {
            if (!this->isAncestorOf(ww)) {
                // New parent is not a subwidget - remove event filter
                ww->removeEventFilter(this);
                const auto children = ww->findChildren<QWidget *>();
                for (QWidget *child : children) {
                    child->removeEventFilter(this);
                }
            }
        }
    }

    // Redirect mouse events to the toolbar when drag + drop editing is enabled
    if (toolBarsEditable()) {
        if (QWidget *ww = qobject_cast<QWidget *>(watched)) {
            switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(), me->button(), me->buttons(), me->modifiers());
                mousePressEvent(&newEvent);
                return true;
            }
            case QEvent::MouseMove: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(), me->button(), me->buttons(), me->modifiers());
                mouseMoveEvent(&newEvent);
                return true;
            }
            case QEvent::MouseButtonRelease: {
                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QMouseEvent newEvent(me->type(), mapFromGlobal(ww->mapToGlobal(me->pos())), me->globalPos(), me->button(), me->buttons(), me->modifiers());
                mouseReleaseEvent(&newEvent);
                return true;
            }
            default:
                break;
            }
        }
    }

    return QToolBar::eventFilter(watched, event);
}

void KToolBar::actionEvent(QActionEvent *event)
{
    if (event->type() == QEvent::ActionRemoved) {
        QWidget *widget = widgetForAction(event->action());
        if (widget) {
            widget->removeEventFilter(this);

            const auto children = widget->findChildren<QWidget *>();
            for (QWidget *child : children) {
                child->removeEventFilter(this);
            }
        }
    }

    QToolBar::actionEvent(event);

    if (event->type() == QEvent::ActionAdded) {
        QWidget *widget = widgetForAction(event->action());
        if (widget) {
            widget->installEventFilter(this);

            const auto children = widget->findChildren<QWidget *>();
            for (QWidget *child : children) {
                child->installEventFilter(this);
            }
            // Center widgets that do not have any use for more space. See bug 165274
            if (!(widget->sizePolicy().horizontalPolicy() & QSizePolicy::GrowFlag)
                // ... but do not center when using text besides icon in vertical toolbar. See bug 243196
                && !(orientation() == Qt::Vertical && toolButtonStyle() == Qt::ToolButtonTextBesideIcon)) {
                const int index = layout()->indexOf(widget);
                if (index != -1) {
                    layout()->itemAt(index)->setAlignment(Qt::AlignJustify);
                }
            }
        }
    }

    d->adjustSeparatorVisibility();
}

bool KToolBar::toolBarsEditable()
{
    return KToolBarPrivate::s_editable;
}

void KToolBar::setToolBarsEditable(bool editable)
{
    if (KToolBarPrivate::s_editable != editable) {
        KToolBarPrivate::s_editable = editable;
    }
}

void KToolBar::setToolBarsLocked(bool locked)
{
    if (KToolBarPrivate::s_locked != locked) {
        KToolBarPrivate::s_locked = locked;

        const auto windows = KMainWindow::memberList();
        for (KMainWindow *mw : windows) {
            const auto toolbars = mw->findChildren<KToolBar *>();
            for (KToolBar *toolbar : toolbars) {
                toolbar->d->setLocked(locked);
            }
        }
    }
}

bool KToolBar::toolBarsLocked()
{
    return KToolBarPrivate::s_locked;
}

void KToolBar::emitToolbarStyleChanged()
{
#ifdef QT_DBUS_LIB
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KToolBar"), QStringLiteral("org.kde.KToolBar"), QStringLiteral("styleChanged"));
    QDBusConnection::sessionBus().send(message);
#endif
}

#include "moc_ktoolbar.cpp"
