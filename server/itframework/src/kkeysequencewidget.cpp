/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-xmlgui.h"

#include "kkeysequencewidget.h"

#include "debug.h"
#include "kactioncollection.h"

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QHash>
#include <QToolButton>

#include <KLocalizedString>
#include <KMessageBox>
#include <KeySequenceRecorder>
#if HAVE_GLOBALACCEL
#include <KGlobalAccel>
#endif

static bool shortcutsConflictWith(const QList<QKeySequence> &shortcuts, const QKeySequence &needle)
{
    if (needle.isEmpty()) {
        return false;
    }

    for (const QKeySequence &sequence : shortcuts) {
        if (sequence.isEmpty()) {
            continue;
        }

        if (sequence.matches(needle) != QKeySequence::NoMatch //
            || needle.matches(sequence) != QKeySequence::NoMatch) {
            return true;
        }
    }

    return false;
}

class KKeySequenceWidgetPrivate
{
public:
    KKeySequenceWidgetPrivate(KKeySequenceWidget *qq);

    void init();

    void updateShortcutDisplay();
    void startRecording();

    // Conflicts the key sequence @p seq with a current standard shortcut?
    bool conflictWithStandardShortcuts(const QKeySequence &seq);
    // Conflicts the key sequence @p seq with a current local shortcut?
    bool conflictWithLocalShortcuts(const QKeySequence &seq);
    // Conflicts the key sequence @p seq with a current global shortcut?
    bool conflictWithGlobalShortcuts(const QKeySequence &seq);

    bool promptStealLocalShortcut(const QList<QAction *> &actions, const QKeySequence &seq);
    bool promptstealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq);

#if HAVE_GLOBALACCEL
    struct KeyConflictInfo {
        QKeySequence key;
        QList<KGlobalShortcutInfo> shortcutInfo;
    };
    bool promptStealGlobalShortcut(const std::vector<KeyConflictInfo> &shortcuts, const QKeySequence &sequence);
#endif
    void wontStealShortcut(QAction *item, const QKeySequence &seq);

    bool checkAgainstStandardShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::StandardShortcuts;
    }

    bool checkAgainstGlobalShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::GlobalShortcuts;
    }

    bool checkAgainstLocalShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::LocalShortcuts;
    }

    // private slot
    void doneRecording();

    // members
    KKeySequenceWidget *const q;
    KeySequenceRecorder *recorder;
    QHBoxLayout *layout;
    QPushButton *keyButton;
    QToolButton *clearButton;

    QKeySequence keySequence;
    QKeySequence oldKeySequence;
    QString componentName;

    //! Check the key sequence against KStandardShortcut::find()
    KKeySequenceWidget::ShortcutTypes checkAgainstShortcutTypes;

    /**
     * The list of action to check against for conflict shortcut
     */
    QList<QAction *> checkList; // deprecated

    /**
     * The list of action collections to check against for conflict shortcut
     */
    QList<KActionCollection *> checkActionCollections;

    /**
     * The action to steal the shortcut from.
     */
    QList<QAction *> stealActions;
};

KKeySequenceWidgetPrivate::KKeySequenceWidgetPrivate(KKeySequenceWidget *qq)
    : q(qq)
    , layout(nullptr)
    , keyButton(nullptr)
    , clearButton(nullptr)
    , componentName()
    , checkAgainstShortcutTypes(KKeySequenceWidget::LocalShortcuts | KKeySequenceWidget::GlobalShortcuts)
    , stealActions()
{
}

void KKeySequenceWidgetPrivate::init()
{
    layout = new QHBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    keyButton = new QPushButton(q);
    keyButton->setFocusPolicy(Qt::StrongFocus);
    keyButton->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    keyButton->setToolTip(
        i18nc("@info:tooltip",
              "Click on the button, then enter the shortcut like you would in the program.\nExample for Ctrl+A: hold the Ctrl key and press A."));
    layout->addWidget(keyButton);

    clearButton = new QToolButton(q);
    layout->addWidget(clearButton);

    if (qApp->isLeftToRight()) {
        clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-rtl")));
    } else {
        clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-ltr")));
    }

    recorder = new KeySequenceRecorder(q->window()->windowHandle(), q);
    recorder->setModifierlessAllowed(false);
    recorder->setMultiKeyShortcutsAllowed(true);

    updateShortcutDisplay();
}

bool KKeySequenceWidgetPrivate::promptStealLocalShortcut(const QList<QAction *> &actions, const QKeySequence &seq)
{
    const int listSize = actions.size();

    QString title = i18ncp("%1 is the number of conflicts", "Shortcut Conflict", "Shortcut Conflicts", listSize);

    QString conflictingShortcuts;
    for (const QAction *action : actions) {
        conflictingShortcuts += i18n("Shortcut '%1' for action '%2'\n",
                                     action->shortcut().toString(QKeySequence::NativeText),
                                     KLocalizedString::removeAcceleratorMarker(action->text()));
    }
    QString message = i18ncp("%1 is the number of ambiguous shortcut clashes (hidden)",
                             "The \"%2\" shortcut is ambiguous with the following shortcut.\n"
                             "Do you want to assign an empty shortcut to this action?\n"
                             "%3",
                             "The \"%2\" shortcut is ambiguous with the following shortcuts.\n"
                             "Do you want to assign an empty shortcut to these actions?\n"
                             "%3",
                             listSize,
                             seq.toString(QKeySequence::NativeText),
                             conflictingShortcuts);

    return KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18nc("@action:button", "Reassign"))) == KMessageBox::Continue;
}

void KKeySequenceWidgetPrivate::wontStealShortcut(QAction *item, const QKeySequence &seq)
{
    QString title(i18nc("@title:window", "Shortcut conflict"));
    QString msg(
        i18n("<qt>The '%1' key combination is already used by the <b>%2</b> action.<br>"
             "Please select a different one.</qt>",
             seq.toString(QKeySequence::NativeText),
             KLocalizedString::removeAcceleratorMarker(item->text())));
    KMessageBox::error(q, msg, title);
}

bool KKeySequenceWidgetPrivate::conflictWithLocalShortcuts(const QKeySequence &keySequence)
{
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::LocalShortcuts)) {
        return false;
    }

    // We have actions both in the deprecated checkList and the
    // checkActionCollections list. Add all the actions to a single list to
    // be able to process them in a single loop below.
    // Note that this can't be done in setCheckActionCollections(), because we
    // keep pointers to the action collections, and between the call to
    // setCheckActionCollections() and this function some actions might already be
    // removed from the collection again.
    QList<QAction *> allActions;
    allActions += checkList;
    for (KActionCollection *collection : std::as_const(checkActionCollections)) {
        allActions += collection->actions();
    }

    // Because of multikey shortcuts we can have clashes with many shortcuts.
    //
    // Example 1:
    //
    // Application currently uses 'CTRL-X,a', 'CTRL-X,f' and 'CTRL-X,CTRL-F'
    // and the user wants to use 'CTRL-X'. 'CTRL-X' will only trigger as
    // 'activatedAmbiguously()' for obvious reasons.
    //
    // Example 2:
    //
    // Application currently uses 'CTRL-X'. User wants to use 'CTRL-X,CTRL-F'.
    // This will shadow 'CTRL-X' for the same reason as above.
    //
    // Example 3:
    //
    // Some weird combination of Example 1 and 2 with three shortcuts using
    // 1/2/3 key shortcuts. I think you can imagine.
    QList<QAction *> conflictingActions;

    // find conflicting shortcuts with existing actions
    for (QAction *qaction : std::as_const(allActions)) {
        if (shortcutsConflictWith(qaction->shortcuts(), keySequence)) {
            // A conflict with a KAction. If that action is configurable
            // ask the user what to do. If not reject this keySequence.
            if (checkActionCollections.first()->isShortcutsConfigurable(qaction)) {
                conflictingActions.append(qaction);
            } else {
                wontStealShortcut(qaction, keySequence);
                return true;
            }
        }
    }

    if (conflictingActions.isEmpty()) {
        // No conflicting shortcuts found.
        return false;
    }

    if (promptStealLocalShortcut(conflictingActions, keySequence)) {
        stealActions = conflictingActions;
        // Announce that the user agreed
        for (QAction *stealAction : std::as_const(stealActions)) {
            Q_EMIT q->stealShortcut(keySequence, stealAction);
        }
        return false;
    }
    return true;
}

#if HAVE_GLOBALACCEL
bool KKeySequenceWidgetPrivate::promptStealGlobalShortcut(const std::vector<KeyConflictInfo> &clashing, const QKeySequence &sequence)
{
    QString clashingKeys;
    for (const auto &[key, shortcutInfo] : clashing) {
        const QString seqAsString = key.toString();
        for (const KGlobalShortcutInfo &info : shortcutInfo) {
            clashingKeys += i18n("Shortcut '%1' in Application '%2' for action '%3'\n", //
                                 seqAsString,
                                 info.componentFriendlyName(),
                                 info.friendlyName());
        }
    }
    const int hashSize = clashing.size();

    QString message = i18ncp("%1 is the number of conflicts (hidden), %2 is the key sequence of the shortcut that is problematic",
                             "The shortcut '%2' conflicts with the following key combination:\n",
                             "The shortcut '%2' conflicts with the following key combinations:\n",
                             hashSize,
                             sequence.toString());
    message += clashingKeys;

    QString title = i18ncp("%1 is the number of shortcuts with which there is a conflict",
                           "Conflict with Registered Global Shortcut",
                           "Conflict with Registered Global Shortcuts",
                           hashSize);

    return KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18nc("@action:button", "Reassign"))) == KMessageBox::Continue;
}
#endif

bool KKeySequenceWidgetPrivate::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
#ifdef Q_OS_WIN
    // on windows F12 is reserved by the debugger at all times, so we can't use it for a global shortcut
    if (KKeySequenceWidget::GlobalShortcuts && keySequence.toString().contains(QLatin1String("F12"))) {
        QString title = i18n("Reserved Shortcut");
        QString message = i18n(
            "The F12 key is reserved on Windows, so cannot be used for a global shortcut.\n"
            "Please choose another one.");

        KMessageBox::error(q, message, title);
        return false;
    }
#endif
#if HAVE_GLOBALACCEL
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::GlobalShortcuts)) {
        return false;
    }
    // Global shortcuts are on key+modifier shortcuts. They can clash with
    // each of the keys of a multi key shortcut.
    std::vector<KeyConflictInfo> clashing;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence keys(keySequence[i]);
        if (!KGlobalAccel::isGlobalShortcutAvailable(keySequence, componentName)) {
            clashing.push_back({keySequence, KGlobalAccel::globalShortcutsByKey(keys)});
        }
    }
    if (clashing.empty()) {
        return false;
    }

    if (!promptStealGlobalShortcut(clashing, keySequence)) {
        return true;
    }
    // The user approved stealing the shortcut. We have to steal
    // it immediately because KAction::setGlobalShortcut() refuses
    // to set a global shortcut that is already used. There is no
    // error it just silently fails. So be nice because this is
    // most likely the first action that is done in the slot
    // listening to keySequenceChanged().
    KGlobalAccel::stealShortcutSystemwide(keySequence);
    return false;
#else
    Q_UNUSED(keySequence);
    return false;
#endif
}

bool KKeySequenceWidgetPrivate::promptstealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
    QString title = i18nc("@title:window", "Conflict with Standard Application Shortcut");
    QString message = i18n(
        "The '%1' key combination is also used for the standard action "
        "\"%2\" that some applications use.\n"
        "Do you really want to use it as a global shortcut as well?",
        seq.toString(QKeySequence::NativeText),
        KStandardShortcut::label(std));

    return KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18nc("@action:button", "Reassign"))) == KMessageBox::Continue;
}

bool KKeySequenceWidgetPrivate::conflictWithStandardShortcuts(const QKeySequence &seq)
{
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::StandardShortcuts)) {
        return false;
    }
    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(seq);
    if (ssc != KStandardShortcut::AccelNone && !promptstealStandardShortcut(ssc, seq)) {
        return true;
    }
    return false;
}

void KKeySequenceWidgetPrivate::startRecording()
{
    keyButton->setDown(true);
    recorder->startRecording();
    updateShortcutDisplay();
}

void KKeySequenceWidgetPrivate::doneRecording()
{
    keyButton->setDown(false);
    stealActions.clear();
    keyButton->setText(keyButton->text().chopped(strlen(" ...")));
    q->setKeySequence(recorder->currentKeySequence(), KKeySequenceWidget::Validate);
    updateShortcutDisplay();
}

void KKeySequenceWidgetPrivate::updateShortcutDisplay()
{
    QString s;
    QKeySequence sequence = recorder->isRecording() ? recorder->currentKeySequence() : keySequence;
    if (!sequence.isEmpty()) {
        s = sequence.toString(QKeySequence::NativeText);
    } else if (recorder->isRecording()) {
        s = i18nc("What the user inputs now will be taken as the new shortcut", "Input");
    } else {
        s = i18nc("No shortcut defined", "None");
    }

    if (recorder->isRecording()) {
        // make it clear that input is still going on
        s.append(QLatin1String(" ..."));
    }

    s = QLatin1Char(' ') + s + QLatin1Char(' ');
    keyButton->setText(s);
}

KKeySequenceWidget::KKeySequenceWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KKeySequenceWidgetPrivate(this))
{
    d->init();
    setFocusProxy(d->keyButton);
    connect(d->keyButton, &QPushButton::clicked, this, &KKeySequenceWidget::captureKeySequence);
    connect(d->clearButton, &QToolButton::clicked, this, &KKeySequenceWidget::clearKeySequence);

    connect(d->recorder, &KeySequenceRecorder::currentKeySequenceChanged, this, [this] {
        d->updateShortcutDisplay();
    });
    connect(d->recorder, &KeySequenceRecorder::recordingChanged, this, [this] {
        if (!d->recorder->isRecording()) {
            d->doneRecording();
        }
    });
}

KKeySequenceWidget::~KKeySequenceWidget()
{
    delete d;
}

KKeySequenceWidget::ShortcutTypes KKeySequenceWidget::checkForConflictsAgainst() const
{
    return d->checkAgainstShortcutTypes;
}

void KKeySequenceWidget::setComponentName(const QString &componentName)
{
    d->componentName = componentName;
}

bool KKeySequenceWidget::multiKeyShortcutsAllowed() const
{
    return d->recorder->multiKeyShortcutsAllowed();
}

void KKeySequenceWidget::setMultiKeyShortcutsAllowed(bool allowed)
{
    d->recorder->setMultiKeyShortcutsAllowed(allowed);
}

void KKeySequenceWidget::setCheckForConflictsAgainst(ShortcutTypes types)
{
    d->checkAgainstShortcutTypes = types;
}

void KKeySequenceWidget::setModifierlessAllowed(bool allow)
{
    d->recorder->setModifierlessAllowed(allow);
}

bool KKeySequenceWidget::isKeySequenceAvailable(const QKeySequence &keySequence) const
{
    if (keySequence.isEmpty()) {
        return true;
    }
    return !(d->conflictWithLocalShortcuts(keySequence) //
             || d->conflictWithGlobalShortcuts(keySequence) //
             || d->conflictWithStandardShortcuts(keySequence));
}

bool KKeySequenceWidget::isModifierlessAllowed()
{
    return d->recorder->modifierlessAllowed();
}

void KKeySequenceWidget::setClearButtonShown(bool show)
{
    d->clearButton->setVisible(show);
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(4, 1)
void KKeySequenceWidget::setCheckActionList(const QList<QAction *> &checkList) // deprecated
{
    d->checkList = checkList;
    Q_ASSERT(d->checkActionCollections.isEmpty()); // don't call this method if you call setCheckActionCollections!
}
#endif

void KKeySequenceWidget::setCheckActionCollections(const QList<KActionCollection *> &actionCollections)
{
    d->checkActionCollections = actionCollections;
}

// slot
void KKeySequenceWidget::captureKeySequence()
{
    d->recorder->setWindow(window()->windowHandle());
    d->recorder->startRecording();
}

QKeySequence KKeySequenceWidget::keySequence() const
{
    return d->keySequence;
}

// slot
void KKeySequenceWidget::setKeySequence(const QKeySequence &seq, Validation validate)
{
    if (d->keySequence == seq) {
        return;
    }
    if (validate == Validate && !isKeySequenceAvailable(seq)) {
        return;
    }
    d->keySequence = seq;
    d->updateShortcutDisplay();
    Q_EMIT keySequenceChanged(seq);
}

// slot
void KKeySequenceWidget::clearKeySequence()
{
    setKeySequence(QKeySequence());
}

// slot
void KKeySequenceWidget::applyStealShortcut()
{
    QSet<KActionCollection *> changedCollections;

    for (QAction *stealAction : std::as_const(d->stealActions)) {
        // Stealing a shortcut means setting it to an empty one.
        stealAction->setShortcuts(QList<QKeySequence>());

        // The following code will find the action we are about to
        // steal from and save it's actioncollection.
        KActionCollection *parentCollection = nullptr;
        for (KActionCollection *collection : std::as_const(d->checkActionCollections)) {
            if (collection->actions().contains(stealAction)) {
                parentCollection = collection;
                break;
            }
        }

        // Remember the changed collection
        if (parentCollection) {
            changedCollections.insert(parentCollection);
        }
    }

    for (KActionCollection *col : std::as_const(changedCollections)) {
        col->writeSettings();
    }

    d->stealActions.clear();
}

#include "moc_kkeysequencewidget.cpp"
