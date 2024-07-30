/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../decorationdefines.h"
#include <QFont>
#include <QFontMetricsF>
#include <QList>
#include <kdecoration2/private/kdecoration2_private_export.h>
#include <memory>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration2 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace KDecoration2
{
class DecorationSettings;

class KDECORATIONS_PRIVATE_EXPORT DecorationSettingsPrivate
{
public:
    virtual ~DecorationSettingsPrivate();
    virtual bool isOnAllDesktopsAvailable() const = 0;
    virtual bool isAlphaChannelSupported() const = 0;
    virtual bool isCloseOnDoubleClickOnMenu() const = 0;
    virtual QList<DecorationButtonType> decorationButtonsLeft() const = 0;
    virtual QList<DecorationButtonType> decorationButtonsRight() const = 0;
    virtual BorderSize borderSize() const = 0;
    virtual QFont font() const;
    virtual QFontMetricsF fontMetrics() const;

    DecorationSettings *decorationSettings();
    const DecorationSettings *decorationSettings() const;

    int gridUnit() const;
    int smallSpacing() const;
    int largeSpacing() const;
    void setGridUnit(int unit);
    void setLargeSpacing(int spacing);
    void setSmallSpacing(int spacing);

protected:
    explicit DecorationSettingsPrivate(DecorationSettings *parent);

private:
    class Private;
    const std::unique_ptr<Private> d;
};
}
