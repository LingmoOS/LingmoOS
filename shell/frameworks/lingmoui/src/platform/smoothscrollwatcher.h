/*
 * SPDX-FileCopyrightText: 2024 Nathan Misner <nathan@infochunk.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LINGMOUI_SMOOTHSCROLLWATCHER_H
#define LINGMOUI_SMOOTHSCROLLWATCHER_H

#include <QObject>

#include "lingmouiplatform_export.h"

namespace LingmoUI
{
namespace Platform
{
/**
 * @class SmoothScrollWatcher smoothscrollwatcher.h <LingmoUI/SmoothScrollWatcher>
 *
 * This class reports on the status of the SmoothScroll DBus interface,
 * which sends a message when the smooth scroll setting gets changed.
 */
class LINGMOUIPLATFORM_EXPORT SmoothScrollWatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged FINAL)

public:
    SmoothScrollWatcher(QObject *parent = nullptr);
    ~SmoothScrollWatcher();

    bool enabled() const;

    static SmoothScrollWatcher *self();

Q_SIGNALS:
    void enabledChanged(bool value);

private:
    bool m_enabled;

private Q_SLOTS:
    void setEnabled(bool value);
};

}
}

#endif // LINGMOUI_SMOOTHSCROLLWATCHER_H
