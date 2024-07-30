/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KEYSEQUENCEHELPER_H
#define KEYSEQUENCEHELPER_H

#include <KKeySequenceRecorder>

#include <QKeySequence>
#include <QQuickItem>

class KeySequenceHelperPrivate;
class QQuickWindow;

class KeySequenceHelper : public KKeySequenceRecorder
{
    Q_OBJECT

    Q_PROPERTY(
        ShortcutTypes checkAgainstShortcutTypes READ checkAgainstShortcutTypes WRITE setCheckAgainstShortcutTypes NOTIFY checkAgainstShortcutTypesChanged)

public:
    enum ShortcutType {
        None = 0x00, //!< No checking for conflicts
        StandardShortcuts = 0x01, //!< Check against standard shortcuts. @see KStandardShortcut
        GlobalShortcuts = 0x02, //!< Check against global shortcuts. @see KGlobalAccel
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)
    Q_FLAG(ShortcutTypes)

    /**
     * Constructor.
     */
    explicit KeySequenceHelper(QObject *parent = nullptr);

    /**
     * Destructs the widget.
     */
    ~KeySequenceHelper() override;

    Q_INVOKABLE void updateKeySequence(const QKeySequence &keySequence);

    Q_INVOKABLE void showQuestionDialog(const QString &title, const QString &text);
    Q_INVOKABLE void showErrorDialog(const QString &title, const QString &text);

    ShortcutTypes checkAgainstShortcutTypes();
    void setCheckAgainstShortcutTypes(ShortcutTypes types);

    Q_INVOKABLE static bool keySequenceIsEmpty(const QKeySequence &keySequence);
    Q_INVOKABLE static QString keySequenceNativeText(const QKeySequence &keySequence);
    Q_INVOKABLE static QWindow *renderWindow(QQuickWindow *quickWindow);

Q_SIGNALS:
    void checkAgainstShortcutTypesChanged();
    void questionDialogAccepted();
    void questionDialogRejected();

private:
    friend class KeySequenceHelperPrivate;
    KeySequenceHelperPrivate *const d;

    Q_DISABLE_COPY(KeySequenceHelper)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KeySequenceHelper::ShortcutTypes)

#endif // KEYSEQUENCEHELPER_H
