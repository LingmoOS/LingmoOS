/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "qscreenconfig.h"
#include "types.h"

#include <QLoggingCategory>
#include <QScreen>
#include <QSize>

namespace KScreen
{
class Output;

class QScreenScreen : public QObject
{
    Q_OBJECT

public:
    explicit QScreenScreen(QScreenConfig *config);
    ~QScreenScreen() override;

    KScreen::ScreenPtr toKScreenScreen() const;
    void updateKScreenScreen(KScreen::ScreenPtr &screen) const;
};

} // namespace
