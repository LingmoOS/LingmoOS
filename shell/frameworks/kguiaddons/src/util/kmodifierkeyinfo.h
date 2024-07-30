/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KMODIFIERKEYINFO_H
#define KMODIFIERKEYINFO_H

#include <kguiaddons_export.h>

#include <QExplicitlySharedDataPointer>
#include <QObject>

class KModifierKeyInfoProvider;

/**
 * @class KModifierKeyInfo kmodifierkeyinfo.h KModifierKeyInfo
 *
 * Get information about the state of the keyboard's modifier keys.
 *
 * This class provides cross-platform information about the state of the
 * keyboard's modifier keys and the mouse buttons and allows to change the
 * state as well.
 *
 * It recognizes two states a key can be in:
 * @li @em locked: eg. caps-locked (a.k.a. toggled)
 * @li @em latched the key is temporarily locked but will be unlocked upon
 *                          the next keypress.
 *
 * An application can either query the states synchronously (isKeyLatched(),
 * isKeyLocked()) or connect to KModifierKeyInfo's signals to be notified about
 * changes (keyLatched(), keyLocked()).
 */
class KGUIADDONS_EXPORT KModifierKeyInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * Default constructor
     */
    explicit KModifierKeyInfo(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~KModifierKeyInfo() override;

    /**
     * Check if a key is known by the underlying window system and can be queried.
     *
     * @param key The key to check
     * @return true if the key is available, false if it is unknown
     */
    bool knowsKey(Qt::Key key) const;

    /**
     * Get a list of known keys.
     *
     * @return A list of known keys of which states will be reported.
     */
    const QList<Qt::Key> knownKeys() const;

    /**
     * Synchronously check if a key is pressed.
     *
     * @param key The key to check
     * @return true if the key is pressed, false if the key is not pressed or unknown.
     * @see isKeyLatched, @see isKeyLocked, @see keyPressed
     */
    bool isKeyPressed(Qt::Key key) const;

    /**
     * Synchronously check if a key is latched.
     *
     * @param key The key to check
     * @return true if the key is latched, false if the key is not latched or unknown.
     * @see isKeyPressed, @see isKeyLocked, @see keyLatched
     */
    bool isKeyLatched(Qt::Key key) const;

    /**
     * Set the latched state of a key.
     *
     * @param key The key to latch
     * @param latched true to latch the key, false to unlatch it.
     * @return false if the key is unknown. true doesn't guarantee you the
     *         operation worked.
     */
    bool setKeyLatched(Qt::Key key, bool latched);

    /**
     * Synchronously check if a key is locked.
     *
     * @param key The key to check
     * @return true if the key is locked, false if the key is not locked or unknown.
     * @see isKeyPressed, @see isKeyLatched, @see keyLocked
     */
    bool isKeyLocked(Qt::Key key) const;

    /**
     * Set the locked state of a key.
     *
     * @param key The key to lock
     * @param latched true to lock the key, false to unlock it.
     * @return false if the key is unknown. true doesn't guarantee you the
     *         operation worked.
     */
    bool setKeyLocked(Qt::Key key, bool locked);

    /**
     * Synchronously check if a mouse button is pressed.
     *
     * @param button The mouse button to check
     * @return true if the mouse button is pressed, false if the mouse button
     *         is not pressed or its state is unknown.
     */
    bool isButtonPressed(Qt::MouseButton button) const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the pressed state of a key changes
     * (key press or key release).
     *
     * @param key The key that changed state
     * @param pressed true if the key is now pressed, false if is released.
     */
    void keyPressed(Qt::Key key, bool pressed);

    /**
     * This signal is emitted whenever the latched state of a key changes.
     *
     * @param key The key that changed state
     * @param latched true if the key is now latched, false if it isn't
     */
    void keyLatched(Qt::Key key, bool latched);

    /**
     * This signal is emitted whenever the locked state of a key changes.
     *
     * @param key The key that changed state
     * @param locked true if the key is now locked, false if it isn't
     */
    void keyLocked(Qt::Key key, bool locked);

    /**
     * This signal is emitted whenever the pressed state of a mouse button
     * changes (mouse button press or release).
     *
     * @param button The mouse button that changed state
     * @param pressed true if the mouse button is now pressed, false if
     *                is released.
     */
    void buttonPressed(Qt::MouseButton button, bool pressed);

    /**
     * This signal is emitted whenever a new modifier is found due to
     * the keyboard mapping changing.
     *
     * @param key The key that was discovered
     */
    void keyAdded(Qt::Key key);

    /**
     * This signal is emitted whenever a previously known modifier no
     * longer exists due to the keyboard mapping changing.
     *
     * @param key The key that vanished
     */
    void keyRemoved(Qt::Key key);

private:
    Q_DISABLE_COPY(KModifierKeyInfo)
    QExplicitlySharedDataPointer<KModifierKeyInfoProvider> const p; // krazy:exclude=dpointer
};

#endif
