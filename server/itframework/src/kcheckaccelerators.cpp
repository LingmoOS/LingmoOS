/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 1998, 1999, 2000 KDE Team
    SPDX-FileCopyrightText: 2008 Nick Shaforostoff <shaforostoff@kde.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcheckaccelerators.h"

#include <QAction>
#include <QApplication>
#include <QChar>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QShortcutEvent>
#include <QTabBar>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

class KCheckAcceleratorsInitializer : public QObject
{
    Q_OBJECT
public:
    explicit KCheckAcceleratorsInitializer(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

public Q_SLOTS:
    void initiateIfNeeded()
    {
        KConfigGroup cg(KSharedConfig::openConfig(), "Development");
        QString sKey = cg.readEntry("CheckAccelerators").trimmed();
        int key = 0;
        if (!sKey.isEmpty()) {
            QList<QKeySequence> cuts = QKeySequence::listFromString(sKey);
            if (!cuts.isEmpty()) {
                key = cuts.first()[0];
            }
        }
        const bool autoCheck = cg.readEntry("AutoCheckAccelerators", true);
        const bool copyWidgetText = cg.readEntry("CopyWidgetText", false);
        if (!copyWidgetText && key == 0 && !autoCheck) {
            deleteLater();
            return;
        }

        new KCheckAccelerators(qApp, key, autoCheck, copyWidgetText);
        deleteLater();
    }
};

static void startupFunc()
{
    // Static because in some cases this is called multiple times
    // but if an application had any of the bad cases we always want
    // to skip the check
    static bool doCheckAccelerators = true;

    if (!doCheckAccelerators) {
        return;
    }

    QCoreApplication *app = QCoreApplication::instance();
    if (!app) {
        // We're being loaded by something that doesn't have a QCoreApplication
        // this would probably crash at some later point since we do use qApp->
        // quite a lot, so skip the magic
        doCheckAccelerators = false;
        return;
    }

    if (!QCoreApplication::startingUp()) {
        // If the app has already started, this means we're not being run as part of
        // qt_call_pre_routines, which most probably means that we're being run as
        // part of KXmlGui being loaded as part of some plugin of the app, so don't
        // do any magic
        doCheckAccelerators = false;
        return;
    }

    if (!QCoreApplication::eventDispatcher()) {
        // We are called with event dispatcher being null when KXmlGui is being
        // loaded through plasma-integration instead of being linked to the app
        // (i.e. QtCreator vs Okular) For apps that don't link directly to KXmlGui
        // do not do the accelerator magic
        doCheckAccelerators = false;
        return;
    }

    KCheckAcceleratorsInitializer *initializer = new KCheckAcceleratorsInitializer(app);
    // Call initiateIfNeeded once we're in the event loop
    // This is to prevent using KSharedConfig before main() can set the app name
    QMetaObject::invokeMethod(initializer, "initiateIfNeeded", Qt::QueuedConnection);
}

Q_COREAPP_STARTUP_FUNCTION(startupFunc)

KCheckAccelerators::KCheckAccelerators(QObject *parent, int key_, bool autoCheck_, bool copyWidgetText_)
    : QObject(parent)
    , key(key_)
    , block(false)
    , autoCheck(autoCheck_)
    , copyWidgetText(copyWidgetText_)
    , drklash(nullptr)
{
    setObjectName(QStringLiteral("kapp_accel_filter"));

    KConfigGroup cg(KSharedConfig::openConfig(), "Development");
    alwaysShow = cg.readEntry("AlwaysShowCheckAccelerators", false);
    copyWidgetTextCommand = cg.readEntry("CopyWidgetTextCommand", QString());

    parent->installEventFilter(this);
    connect(&autoCheckTimer, &QTimer::timeout, this, &KCheckAccelerators::autoCheckSlot);
}

bool KCheckAccelerators::eventFilter(QObject *obj, QEvent *e)
{
    if (block) {
        return false;
    }

    switch (e->type()) { // just simplify debuggin
    case QEvent::ShortcutOverride:
        if (key && (static_cast<QKeyEvent *>(e)->key() == key)) {
            block = true;
            checkAccelerators(false);
            block = false;
            e->accept();
            return true;
        }
        break;
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
        // Only care about widgets; this also avoids starting the timer in other
        // threads
        if (!static_cast<QChildEvent *>(e)->child()->isWidgetType()) {
            break;
        }
        Q_FALLTHROUGH();
    // fall-through
    case QEvent::Resize:
    case QEvent::LayoutRequest:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        if (autoCheck) {
            autoCheckTimer.setSingleShot(true);
            autoCheckTimer.start(20); // 20 ms
        }
        break;
    // case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
        if (copyWidgetText && static_cast<QMouseEvent *>(e)->button() == Qt::MiddleButton) {
            // kWarning()<<"obj"<<obj;
            QWidget *w = static_cast<QWidget *>(obj)->childAt(static_cast<QMouseEvent *>(e)->pos());
            if (!w) {
                w = static_cast<QWidget *>(obj);
            }
            if (!w) {
                return false;
            }
            // kWarning()<<"MouseButtonDblClick"<<w;
            QString text;
            if (qobject_cast<QLabel *>(w)) {
                text = static_cast<QLabel *>(w)->text();
            } else if (qobject_cast<QAbstractButton *>(w)) {
                text = static_cast<QAbstractButton *>(w)->text();
            } else if (qobject_cast<QComboBox *>(w)) {
                text = static_cast<QComboBox *>(w)->currentText();
            } else if (qobject_cast<QTabBar *>(w)) {
                text = static_cast<QTabBar *>(w)->tabText(static_cast<QTabBar *>(w)->tabAt(static_cast<QMouseEvent *>(e)->pos()));
            } else if (qobject_cast<QGroupBox *>(w)) {
                text = static_cast<QGroupBox *>(w)->title();
            } else if (qobject_cast<QMenu *>(obj)) {
                QAction *a = static_cast<QMenu *>(obj)->actionAt(static_cast<QMouseEvent *>(e)->pos());
                if (!a) {
                    return false;
                }
                text = a->text();
                if (text.isEmpty()) {
                    text = a->iconText();
                }
            }
            if (text.isEmpty()) {
                return false;
            }

            if (static_cast<QMouseEvent *>(e)->modifiers() == Qt::ControlModifier) {
                text.remove(QChar::fromLatin1('&'));
            }

            // kWarning()<<KGlobal::activeComponent().catalogName()<<text;
            if (copyWidgetTextCommand.isEmpty()) {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(text);
            } else {
                const QString textCmd = copyWidgetTextCommand.arg(text, QFile::decodeName(KLocalizedString::applicationDomain()));
                const QString exec = QStandardPaths::findExecutable(textCmd);
                if (exec.isEmpty()) {
                    qWarning() << "Could not find executable:" << textCmd;
                    return false;
                }

                QProcess *script = new QProcess(this);
                script->start(exec, QStringList{});
                connect(script, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), script, &QObject::deleteLater);
            }
            e->accept();
            return true;

            // kWarning()<<"MouseButtonDblClick"<<static_cast<QWidget*>(obj)->childAt(static_cast<QMouseEvent*>(e)->globalPos());
        }
        return false;
    case QEvent::Timer:
    case QEvent::MouseMove:
    case QEvent::Paint:
        return false;
    default:
        // qCDebug(DEBUG_KXMLGUI) << "KCheckAccelerators::eventFilter " << e->type()
        // << " " << autoCheck;
        break;
    }
    return false;
}

void KCheckAccelerators::autoCheckSlot()
{
    if (block) {
        autoCheckTimer.setSingleShot(true);
        autoCheckTimer.start(20);
        return;
    }
    block = true;
    checkAccelerators(!alwaysShow);
    block = false;
}

void KCheckAccelerators::createDialog(QWidget *actWin, bool automatic)
{
    if (drklash) {
        return;
    }

    drklash = new QDialog(actWin);
    drklash->setAttribute(Qt::WA_DeleteOnClose);
    drklash->setObjectName(QStringLiteral("kapp_accel_check_dlg"));
    drklash->setWindowTitle(i18nc("@title:window", "Dr. Klash' Accelerator Diagnosis"));
    drklash->resize(500, 460);
    QVBoxLayout *layout = new QVBoxLayout(drklash);
    drklash_view = new QTextBrowser(drklash);
    layout->addWidget(drklash_view);
    QCheckBox *disableAutoCheck = nullptr;
    if (automatic) {
        disableAutoCheck = new QCheckBox(i18nc("@option:check", "Disable automatic checking"), drklash);
        connect(disableAutoCheck, &QCheckBox::toggled, this, &KCheckAccelerators::slotDisableCheck);
        layout->addWidget(disableAutoCheck);
    }
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, drklash);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::rejected, drklash, &QDialog::close);
    if (disableAutoCheck) {
        disableAutoCheck->setFocus();
    } else {
        drklash_view->setFocus();
    }
}

void KCheckAccelerators::slotDisableCheck(bool on)
{
    autoCheck = !on;
    if (!on) {
        autoCheckSlot();
    }
}

void KCheckAccelerators::checkAccelerators(bool automatic)
{
    QWidget *actWin = qApp->activeWindow();
    if (!actWin) {
        return;
    }

    KAcceleratorManager::manage(actWin);
    QString a;
    QString c;
    QString r;
    KAcceleratorManager::last_manage(a, c, r);

    if (automatic) { // for now we only show dialogs on F12 checks
        return;
    }

    if (c.isEmpty() && r.isEmpty() && (automatic || a.isEmpty())) {
        return;
    }

    QString s;

    if (!c.isEmpty()) {
        s += i18n("<h2>Accelerators changed</h2>")
            + QLatin1String(
                  "<table "
                  "border><tr><th><b>%1</b></th><th><b>%2</b></th></tr>%3</table>")
                  .arg(i18n("Old Text"), i18n("New Text"), c);
    }

    if (!r.isEmpty()) {
        s += i18n("<h2>Accelerators removed</h2>") + QLatin1String("<table border><tr><th><b>%1</b></th></tr>%2</table>").arg(i18n("Old Text"), r);
    }

    if (!a.isEmpty()) {
        s += i18n("<h2>Accelerators added (just for your info)</h2>")
            + QLatin1String("<table border><tr><th><b>%1</b></th></tr>%2</table>").arg(i18n("New Text"), a);
    }

    createDialog(actWin, automatic);
    drklash_view->setHtml(s);
    drklash->show();
    drklash->raise();

    // dlg will be destroyed before returning
}

#include "kcheckaccelerators.moc"
