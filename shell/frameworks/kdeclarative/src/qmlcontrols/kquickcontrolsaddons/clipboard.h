/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QClipboard>
#include <QVariant>
#include <qqmlregistration.h>

class ClipboardPrivate;

/**
 * @brief Wrapper for QClipboard
 *
 * Offers a simple wrapper to interact with QClipboard from QtQuick.
 *
 * ```
 * import QtQuick
 * import org.kde.kquickcontrolsaddons as KQuickControlsAddons
 * Text {
 *     text: "lorem ipsum"
 *
 *     KQuickControlsAddons.Clipboard { id: clipboard }
 *
 *     MouseArea {
 *         anchors.fill: parent
 *         acceptedButtons: Qt.LeftButton | Qt.RightButton
 *         onClicked: clipboard.content = parent.text
 *     }
 * }
 * ```
 */
class Clipboard : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    /**
     * Controls the state this object will be monitoring and extracting its contents from.
     */
    Q_PROPERTY(QClipboard::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

    /**
     * Provides the contents currently in the clipboard and lets modify them.
     */
    Q_PROPERTY(QVariant content READ content WRITE setContent NOTIFY contentChanged)

    /**
     * Figure out the nature of the contents in the clipboard as mimetype strings.
     */
    Q_PROPERTY(QStringList formats READ formats NOTIFY contentChanged)

public:
    explicit Clipboard(QObject *parent = nullptr);

    QClipboard::Mode mode() const;
    void setMode(QClipboard::Mode mode);

    /**
     * @param format mimetype string
     * @return Output based on the mimetype. This may be a list of URLs, text, image data, or use QMimeData::data
     */
    Q_SCRIPTABLE QVariant contentFormat(const QString &format) const;
    QVariant content() const;
    void setContent(const QVariant &content);

    QStringList formats() const;

    /** @see QClipboard::clear() */
    Q_SCRIPTABLE void clear();

Q_SIGNALS:
    void modeChanged(QClipboard::Mode mode);
    void contentChanged();

private Q_SLOTS:
    void clipboardChanged(QClipboard::Mode m);

private:
    QClipboard *m_clipboard;
    QClipboard::Mode m_mode;
};

#endif
