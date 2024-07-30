/*
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KKEYSEQUENCERECORDER_H
#define KKEYSEQUENCERECORDER_H

#include <kguiaddons_export.h>

#include <QKeySequence>
#include <QObject>
#include <QWindow>

#include <memory>

class KKeySequenceRecorderPrivate;

/**
 * @class KKeySequenceRecorder kkeysequencerecorder.h KKeySequenceRecorder
 *
 * @short Record a QKeySequence by listening to key events in a window.
 *
 * After calling startRecording key events in the set window will be captured until a valid
 * QKeySequence has been recorded and gotKeySequence is emitted. See multiKeyShortcutsAllowed and
 * modifierlessAllowed for what constitutes a valid key sequence.
 *
 * During recording any shortcuts are inhibited and cannot be triggered. Either by using the
 * <a href="https://cgit.freedesktop.org/wayland/wayland-protocols/tree/unstable/keyboard-shortcuts-inhibit/keyboard-shortcuts-inhibit-unstable-v1.xml">
 * keyboard-shortcuts-inhibit protocol </a> on Wayland or grabbing the keyboard.
 *
 * For graphical elements that record key sequences and can optionally perform conflict checking
 * against existing shortcuts see KKeySequenceWidget and KeySequenceItem.
 *
 * Porting from KF5 to KF6:
 *
 * The class KeySequenceRecorder was renamed to KKeySequenceRecorder.
 *
 * @see KKeySequenceWidget, KeySequenceItem
 * @since 6.0
 */
class KGUIADDONS_EXPORT KKeySequenceRecorder : public QObject
{
    Q_OBJECT

    /**
     * Whether key events are currently recorded
     */
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingChanged)
    /**
     * The recorded key sequence.
     * After construction this is empty.
     *
     * During recording it is continuously updated with the newest user input.
     *
     * After recording it contains the last recorded QKeySequence
     */
    Q_PROPERTY(QKeySequence currentKeySequence READ currentKeySequence WRITE setCurrentKeySequence NOTIFY currentKeySequenceChanged)
    /**
     * The window in which the key events are happening that should be recorded
     */
    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    /**
     * If key presses of "plain" keys without a modifier are considered to be a valid finished
     * key combination.
     * Plain keys  include letter and symbol keys and text editing keys (Return, Space, Tab,
     * Backspace, Delete). Other keys like F1, Cursor keys, Insert, PageDown will always work.
     *
     * By default this is `false`.
     */
    Q_PROPERTY(bool modifierlessAllowed READ modifierlessAllowed WRITE setModifierlessAllowed NOTIFY modifierlessAllowedChanged)
    /** Controls the amount of key combinations that are captured until recording stops and gotKeySequence
     * is emitted.
     * By default  this is `true` and "Emacs-style" key sequences are recorded. Recording does not
     * stop until four valid key combination have been recorded. Afterwards `currentKeySequence().count()`
     * will be 4.
     *
     * Otherwise only one key combination is recorded before gotKeySequence is emitted with a
     * QKeySequence with a `count()` of  1.
     * @see QKeySequence
     */
    Q_PROPERTY(bool multiKeyShortcutsAllowed READ multiKeyShortcutsAllowed WRITE setMultiKeyShortcutsAllowed NOTIFY multiKeyShortcutsAllowedChanged)

    /**
     * It makes it acceptable for the key sequence to be just a modifier (e.g. Shift or Control)
     *
     * By default, if only a modifier is pressed and then released, the component will remain waiting for the sequence.
     * When enabled, it will take the modifier key as the key sequence.
     *
     * By default this is `false`.
     */
    Q_PROPERTY(bool modifierOnlyAllowed READ modifierOnlyAllowed WRITE setModifierOnlyAllowed NOTIFY modifierOnlyAllowedChanged)
public:
    /**
     * Constructor.
     *
     * @par window The window whose key events will be recorded.
     * @see window
     */
    explicit KKeySequenceRecorder(QWindow *window, QObject *parent = nullptr);
    ~KKeySequenceRecorder() override;

    /**
     * Start recording.
     * Calling startRecording when window() is `nullptr` has no effect.
     */
    Q_INVOKABLE void startRecording();

    bool isRecording() const;

    QKeySequence currentKeySequence() const;
    void setCurrentKeySequence(const QKeySequence &sequence);

    QWindow *window() const;
    void setWindow(QWindow *window);

    bool multiKeyShortcutsAllowed() const;
    void setMultiKeyShortcutsAllowed(bool allowed);

    void setModifierlessAllowed(bool allowed);
    bool modifierlessAllowed() const;

    void setModifierOnlyAllowed(bool allowed);
    bool modifierOnlyAllowed() const;

public Q_SLOTS:
    /**
     * Stops the recording session
     */
    void cancelRecording();

Q_SIGNALS:
    /**
     * This signal is emitted when a key sequence has been recorded.
     *
     * Compared to currentKeySequenceChanged and currentKeySequence this is signal is not emitted
     * continuously during recording but only after recording has finished.
     */
    void gotKeySequence(const QKeySequence &keySequence);

    void recordingChanged();
    void windowChanged();
    void currentKeySequenceChanged();
    void multiKeyShortcutsAllowedChanged();
    void modifierlessAllowedChanged();
    void modifierOnlyAllowedChanged();

private:
    friend class KKeySequenceRecorderPrivate;
    std::unique_ptr<KKeySequenceRecorderPrivate> const d;
};

#endif
