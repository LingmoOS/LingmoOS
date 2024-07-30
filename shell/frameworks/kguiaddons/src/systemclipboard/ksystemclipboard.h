/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSYSTEMCLIPBOARD_H
#define KSYSTEMCLIPBOARD_H

#include <kguiaddons_export.h>

#include <QClipboard>
#include <QObject>

class QMimeData;

/**
 * This class mimics QClipboard but unlike QClipboard it will continue
 * to get updates even when our window does not have focus.
 *
 * This may require extra access permissions
 *
 * @since 5.89
 */
class KGUIADDONS_EXPORT KSystemClipboard : public QObject
{
    Q_OBJECT
public:
    /**
     * Returns a shared global SystemClipboard instance
     */
    static KSystemClipboard *instance();

    /**
     * Sets the clipboard to the new contents
     * The clipboard takes ownership of mime
     */
    virtual void setMimeData(QMimeData *mime, QClipboard::Mode mode) = 0;
    /**
     * Clears the current clipboard
     */
    virtual void clear(QClipboard::Mode mode) = 0;
    /**
     * Returns the current mime data received by the clipboard
     */
    virtual const QMimeData *mimeData(QClipboard::Mode mode) const = 0;
    /**
     * Returns the text content of the Clipboard
     *
     * Similar to QClipboard::text(QClipboard::Mode mode)
     */
    QString text(QClipboard::Mode mode);
Q_SIGNALS:
    /**
     * Emitted when the clipboard changes similar to QClipboard::changed
     */
    void changed(QClipboard::Mode mode);

protected:
    KSystemClipboard(QObject *parent);
};

#endif
