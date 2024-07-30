/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAYLANDCLIPBOARD_H
#define WAYLANDCLIPBOARD_H

#include "ksystemclipboard.h"

#include <memory>

class DataControlDevice;
class DataControlDeviceManager;
class KeyboardFocusWatcher;

class WaylandClipboard : public KSystemClipboard
{
public:
    WaylandClipboard(QObject *parent);
    ~WaylandClipboard();
    void setMimeData(QMimeData *mime, QClipboard::Mode mode) override;
    void clear(QClipboard::Mode mode) override;
    const QMimeData *mimeData(QClipboard::Mode mode) const override;

    bool isValid();

private:
    void gainedFocus();
    std::unique_ptr<KeyboardFocusWatcher> m_keyboardFocusWatcher;
    std::unique_ptr<DataControlDeviceManager> m_manager;
    std::unique_ptr<DataControlDevice> m_device;
};

#endif
