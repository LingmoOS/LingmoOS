/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kstandardaction.h"
#include "kopenaction_p.h"
#include "kstandardaction_p.h"
#include "moc_kstandardaction_p.cpp"

#include <KAboutData>
#include <KAcceleratorManager>
#include <KLocalizedString>
#include <KStandardShortcutWatcher>
#include <QGuiApplication>
#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>

namespace KStandardAction
{
AutomaticAction::AutomaticAction(const QIcon &icon,
                                 const QString &text,
                                 KStandardShortcut::StandardShortcut standardShortcut,
                                 const char *slot,
                                 QObject *parent)
    : QAction(parent)
{
    setText(text);
    setIcon(icon);

    const QList<QKeySequence> shortcut = KStandardShortcut::shortcut(standardShortcut);
    setShortcuts(shortcut);
    setProperty("defaultShortcuts", QVariant::fromValue(shortcut));
    connect(KStandardShortcut::shortcutWatcher(),
            &KStandardShortcut::StandardShortcutWatcher::shortcutChanged,
            this,
            [standardShortcut, this](KStandardShortcut::StandardShortcut id, const QList<QKeySequence> &newShortcut) {
                if (id != standardShortcut) {
                    return;
                }
                setShortcuts(newShortcut);
                setProperty("defaultShortcuts", QVariant::fromValue(newShortcut));
            });

    connect(this, SIGNAL(triggered()), this, slot);
}

QStringList stdNames()
{
    return internal_stdNames();
}

QList<StandardAction> actionIds()
{
    QList<StandardAction> result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        result.append(g_rgActionInfo[i].id);
    }

    return result;
}

KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id)
{
    const KStandardActionInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->idAccel : KStandardShortcut::AccelNone;
}

class ShowMenubarActionFilter : public QObject
{
public:
    ShowMenubarActionFilter(QAction *parent)
        : QObject(parent)
        , wasNative(false)
        , wasChecked(false)
        , wasVisible(false)
    {
    }

    bool eventFilter(QObject * /*watched*/, QEvent *e) override
    {
        if (e->type() == QEvent::Show) {
            updateAction();
        }
        return false;
    }

    void updateAction()
    {
        bool allMenuBarsNative = true;
        bool hasAnyMenuBar = false;
        const auto lstWidget = qApp->topLevelWidgets();
        for (QWidget *w : lstWidget) {
            QMainWindow *mw = qobject_cast<QMainWindow *>(w);
            if (mw) {
                mw->installEventFilter(this); // this is just in case a new main window appeared
                                              // if we were filtering it already it is almost a noop
                if (mw->layout() && mw->layout()->menuBar()) {
                    QMenuBar *mb = qobject_cast<QMenuBar *>(mw->layout()->menuBar());
                    if (mb) {
                        hasAnyMenuBar = true;
                        if (!mb->isNativeMenuBar()) {
                            allMenuBarsNative = false;
                        }
                    }
                }
            }
        }

        if (!hasAnyMenuBar) {
            return;
        }

        QAction *showMenubarAction = static_cast<QAction *>(parent());
        if (allMenuBarsNative && !wasNative) {
            wasNative = true;
            wasChecked = showMenubarAction->isChecked();
            wasVisible = showMenubarAction->isVisible();

            showMenubarAction->setChecked(true);
            showMenubarAction->setVisible(false);
        } else if (!allMenuBarsNative && wasNative) {
            showMenubarAction->setChecked(wasChecked);
            showMenubarAction->setVisible(wasVisible);
        }
    }

    bool wasNative;
    bool wasChecked;
    bool wasVisible;
};

QAction *_k_createInternal(StandardAction id, QObject *parent)
{
    static bool stdNamesInitialized = false;

    if (!stdNamesInitialized) {
        KAcceleratorManager::addStandardActionNames(stdNames());
        stdNamesInitialized = true;
    }

    QAction *pAction = nullptr;
    const KStandardActionInfo *pInfo = infoPtr(id);

    // qCDebug(KCONFIG_WIDGETS_LOG) << "KStandardAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << " )"; // ellis

    if (pInfo) {
        QString sLabel;
        QString iconName = pInfo->psIconName.toString();

        switch (id) {
        case Back:
            sLabel = i18nc("go back", "&Back");
            if (QGuiApplication::isRightToLeft()) {
                iconName = QStringLiteral("go-next");
            }
            break;

        case Forward:
            sLabel = i18nc("go forward", "&Forward");
            if (QGuiApplication::isRightToLeft()) {
                iconName = QStringLiteral("go-previous");
            }
            break;

        case Home:
            sLabel = i18nc("home page", "&Home");
            break;
        case Preferences:
        case AboutApp:
        case HelpContents: {
            QString appDisplayName = QGuiApplication::applicationDisplayName();
            if (appDisplayName.isEmpty()) {
                appDisplayName = QCoreApplication::applicationName();
            }
            sLabel = pInfo->psLabel.subs(appDisplayName).toString();
        } break;
        default:
            sLabel = pInfo->psLabel.toString();
        }

        if (QGuiApplication::isRightToLeft()) {
            switch (id) {
            case Prior:
                iconName = QStringLiteral("go-next-view-page");
                break;
            case Next:
                iconName = QStringLiteral("go-previous-view-page");
                break;
            case FirstPage:
                iconName = QStringLiteral("go-last-view-page");
                break;
            case LastPage:
                iconName = QStringLiteral("go-first-view-page");
                break;
            case DocumentBack:
                iconName = QStringLiteral("go-next");
                break;
            case DocumentForward:
                iconName = QStringLiteral("go-previous");
                break;
            default:
                break;
            }
        }

        if (id == Donate) {
            const QString currencyCode = QLocale().currencySymbol(QLocale::CurrencyIsoCode).toLower();
            if (!currencyCode.isEmpty()) {
                iconName = QStringLiteral("help-donate-%1").arg(currencyCode);
            }
        }

        QIcon icon = iconName.isEmpty() ? QIcon() : QIcon::fromTheme(iconName);

        switch (id) {
        case Open:
            pAction = new KOpenAction(parent);
            break;
        case OpenRecent:
            pAction = new KRecentFilesAction(parent);
            break;
        case ShowMenubar: {
            pAction = new KToggleAction(parent);
            pAction->setWhatsThis(
                i18n("Show Menubar<p>"
                     "Shows the menubar again after it has been hidden</p>"));
            pAction->setChecked(true);

            ShowMenubarActionFilter *mf = new ShowMenubarActionFilter(pAction);
            const auto lstWidget = qApp->topLevelWidgets();
            for (QWidget *w : lstWidget) {
                if (qobject_cast<QMainWindow *>(w)) {
                    w->installEventFilter(mf);
                }
            }
            mf->updateAction();
            break;
        }
        case ShowToolbar:
            pAction = new KToggleAction(parent);
            pAction->setChecked(true);
            break;
        case ShowStatusbar:
            pAction = new KToggleAction(parent);
            pAction->setWhatsThis(
                i18n("Show Statusbar<p>"
                     "Shows the statusbar, which is the bar at the bottom of the window used for status information.</p>"));
            pAction->setChecked(true);
            break;
        case FullScreen:
            pAction = new KToggleFullScreenAction(parent);
            pAction->setCheckable(true);
            break;
        // Same as default, but with the app icon
        case AboutApp: {
            pAction = new QAction(parent);
            icon = qApp->windowIcon();
            break;
        }
        case HamburgerMenu: {
            pAction = new KHamburgerMenu(parent);
            break;
        }

        default:
            pAction = new QAction(parent);
            break;
        }

        // Set the text before setting the MenuRole, as on OS X setText will do some heuristic role guessing.
        // This ensures user menu items get the intended role out of the list below.
        pAction->setText(sLabel);

        switch (id) {
        case Quit:
            pAction->setMenuRole(QAction::QuitRole);
            break;

        case Preferences:
            pAction->setMenuRole(QAction::PreferencesRole);
            break;

        case AboutApp:
            pAction->setMenuRole(QAction::AboutRole);
            break;

        default:
            pAction->setMenuRole(QAction::NoRole);
            break;
        }

        if (!pInfo->psToolTip.isEmpty()) {
            pAction->setToolTip(pInfo->psToolTip.toString());
        }
        pAction->setIcon(icon);

        QList<QKeySequence> cut = KStandardShortcut::shortcut(pInfo->idAccel);
        if (!cut.isEmpty()) {
            // emulate KActionCollection::setDefaultShortcuts to allow the use of "configure shortcuts"
            pAction->setShortcuts(cut);
            pAction->setProperty("defaultShortcuts", QVariant::fromValue(cut));
        }
        pAction->connect(KStandardShortcut::shortcutWatcher(),
                         &KStandardShortcut::StandardShortcutWatcher::shortcutChanged,
                         pAction,
                         [pAction, shortcut = pInfo->idAccel](KStandardShortcut::StandardShortcut id, const QList<QKeySequence> &newShortcut) {
                             if (id != shortcut) {
                                 return;
                             }
                             pAction->setShortcuts(newShortcut);
                             pAction->setProperty("defaultShortcuts", QVariant::fromValue(newShortcut));
                         });

        pAction->setObjectName(pInfo->psName.toString());
    }

    if (pAction && parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, pAction->objectName()), Q_ARG(QAction *, pAction));
    }

    return pAction;
}

QAction *create(StandardAction id, const QObject *recvr, const char *slot, QObject *parent)
{
    QAction *pAction = _k_createInternal(id, parent);
    if (recvr && slot) {
        if (id == OpenRecent) {
            // FIXME QAction port: probably a good idea to find a cleaner way to do this
            // Open Recent is a special case - provide the selected URL
            QObject::connect(pAction, SIGNAL(urlSelected(QUrl)), recvr, slot);
        } else if (id == ConfigureToolbars) { // #200815
            QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot, Qt::QueuedConnection);
        } else {
            QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot);
        }
    }
    return pAction;
}

QString name(StandardAction id)
{
    const KStandardActionInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->psName.toString() : QString();
}

QAction *openNew(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(New, recvr, slot, parent);
}

QAction *open(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Open, recvr, slot, parent);
}

KRecentFilesAction *openRecent(const QObject *recvr, const char *slot, QObject *parent)
{
    return (KRecentFilesAction *)KStandardAction::create(OpenRecent, recvr, slot, parent);
}

QAction *save(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Save, recvr, slot, parent);
}

QAction *saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SaveAs, recvr, slot, parent);
}

QAction *revert(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Revert, recvr, slot, parent);
}

QAction *print(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Print, recvr, slot, parent);
}

QAction *printPreview(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(PrintPreview, recvr, slot, parent);
}

QAction *close(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Close, recvr, slot, parent);
}

QAction *mail(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Mail, recvr, slot, parent);
}

QAction *quit(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Quit, recvr, slot, parent);
}

QAction *undo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Undo, recvr, slot, parent);
}

QAction *redo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Redo, recvr, slot, parent);
}

QAction *cut(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Cut, recvr, slot, parent);
}

QAction *copy(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Copy, recvr, slot, parent);
}

QAction *paste(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Paste, recvr, slot, parent);
}

QAction *clear(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Clear, recvr, slot, parent);
}

QAction *selectAll(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SelectAll, recvr, slot, parent);
}

QAction *deselect(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Deselect, recvr, slot, parent);
}

QAction *find(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Find, recvr, slot, parent);
}

QAction *findNext(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FindNext, recvr, slot, parent);
}

QAction *findPrev(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FindPrev, recvr, slot, parent);
}

QAction *replace(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Replace, recvr, slot, parent);
}

QAction *actualSize(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ActualSize, recvr, slot, parent);
}

QAction *fitToPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FitToPage, recvr, slot, parent);
}

QAction *fitToWidth(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FitToWidth, recvr, slot, parent);
}

QAction *fitToHeight(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FitToHeight, recvr, slot, parent);
}

QAction *zoomIn(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ZoomIn, recvr, slot, parent);
}

QAction *zoomOut(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ZoomOut, recvr, slot, parent);
}

QAction *zoom(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Zoom, recvr, slot, parent);
}

QAction *redisplay(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Redisplay, recvr, slot, parent);
}

QAction *up(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Up, recvr, slot, parent);
}

QAction *back(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Back, recvr, slot, parent);
}

QAction *forward(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Forward, recvr, slot, parent);
}

QAction *home(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Home, recvr, slot, parent);
}

QAction *prior(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Prior, recvr, slot, parent);
}

QAction *next(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Next, recvr, slot, parent);
}

QAction *goTo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Goto, recvr, slot, parent);
}

QAction *gotoPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(GotoPage, recvr, slot, parent);
}

QAction *gotoLine(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(GotoLine, recvr, slot, parent);
}

QAction *firstPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FirstPage, recvr, slot, parent);
}

QAction *lastPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(LastPage, recvr, slot, parent);
}

QAction *documentBack(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(DocumentBack, recvr, slot, parent);
}

QAction *documentForward(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(DocumentForward, recvr, slot, parent);
}

QAction *addBookmark(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(AddBookmark, recvr, slot, parent);
}

QAction *editBookmarks(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(EditBookmarks, recvr, slot, parent);
}

QAction *spelling(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Spelling, recvr, slot, parent);
}

static QAction *buildAutomaticAction(QObject *parent, StandardAction id, const char *slot)
{
    const KStandardActionInfo *p = infoPtr(id);
    if (!p) {
        return nullptr;
    }

    AutomaticAction *action = new AutomaticAction(QIcon::fromTheme(p->psIconName.toString()), p->psLabel.toString(), p->idAccel, slot, parent);

    action->setObjectName(p->psName.toString());
    if (!p->psToolTip.isEmpty()) {
        action->setToolTip(p->psToolTip.toString());
    }

    if (parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, action->objectName()), Q_ARG(QAction *, action));
    }

    return action;
}

QAction *cut(QObject *parent)
{
    return buildAutomaticAction(parent, Cut, SLOT(cut()));
}

QAction *copy(QObject *parent)
{
    return buildAutomaticAction(parent, Copy, SLOT(copy()));
}

QAction *paste(QObject *parent)
{
    return buildAutomaticAction(parent, Paste, SLOT(paste()));
}

QAction *clear(QObject *parent)
{
    return buildAutomaticAction(parent, Clear, SLOT(clear()));
}

QAction *selectAll(QObject *parent)
{
    return buildAutomaticAction(parent, SelectAll, SLOT(selectAll()));
}

KToggleAction *showMenubar(const QObject *recvr, const char *slot, QObject *parent)
{
    QAction *ret = KStandardAction::create(ShowMenubar, recvr, slot, parent);
    Q_ASSERT(qobject_cast<KToggleAction *>(ret));
    return static_cast<KToggleAction *>(ret);
}

KToggleAction *showStatusbar(const QObject *recvr, const char *slot, QObject *parent)
{
    QAction *ret = KStandardAction::create(ShowStatusbar, recvr, slot, parent);
    Q_ASSERT(qobject_cast<KToggleAction *>(ret));
    return static_cast<KToggleAction *>(ret);
}

KToggleFullScreenAction *fullScreen(const QObject *recvr, const char *slot, QWidget *window, QObject *parent)
{
    KToggleFullScreenAction *ret;
    ret = static_cast<KToggleFullScreenAction *>(KStandardAction::create(FullScreen, recvr, slot, parent));
    ret->setWindow(window);

    return ret;
}

QAction *keyBindings(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(KeyBindings, recvr, slot, parent);
}

QAction *preferences(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Preferences, recvr, slot, parent);
}

QAction *configureToolbars(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ConfigureToolbars, recvr, slot, parent);
}

QAction *configureNotifications(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ConfigureNotifications, recvr, slot, parent);
}

QAction *helpContents(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(HelpContents, recvr, slot, parent);
}

QAction *whatsThis(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(WhatsThis, recvr, slot, parent);
}

QAction *reportBug(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ReportBug, recvr, slot, parent);
}

QAction *switchApplicationLanguage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SwitchApplicationLanguage, recvr, slot, parent);
}

QAction *aboutApp(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(AboutApp, recvr, slot, parent);
}

QAction *aboutKDE(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(AboutKDE, recvr, slot, parent);
}

QAction *deleteFile(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(DeleteFile, recvr, slot, parent);
}

QAction *renameFile(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(RenameFile, recvr, slot, parent);
}

QAction *moveToTrash(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(MoveToTrash, recvr, slot, parent);
}

QAction *donate(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Donate, recvr, slot, parent);
}

KHamburgerMenu *hamburgerMenu(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KHamburgerMenu *>(KStandardAction::create(HamburgerMenu, recvr, slot, parent));
}

}
