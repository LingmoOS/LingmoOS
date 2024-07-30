/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "types.h"

#include <QScreen>

namespace KScreen
{
class Output;
class QScreenOutput;
class QScreenScreen;

class QScreenConfig : public QObject
{
    Q_OBJECT

public:
    explicit QScreenConfig(QObject *parent = nullptr);
    ~QScreenConfig() override;

    KScreen::ConfigPtr toKScreenConfig() const;
    void updateKScreenConfig(KScreen::ConfigPtr &config) const;

    QMap<int, QScreenOutput *> outputMap() const;
    int outputId(const QScreen *qscreen);

private Q_SLOTS:
    void screenAdded(const QScreen *qscreen);
    void screenRemoved(QScreen *qscreen);

Q_SIGNALS:
    void configChanged(const KScreen::ConfigPtr &config);

private:
    QMap<int, QScreenOutput *> m_outputMap;
    QScreenScreen *m_screen;
    int m_lastOutputId = -1;
    bool m_blockSignals;
};

} // namespace
