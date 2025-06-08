/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <Plasma/Applet>

#include <KScreen/Types>

#include "../common/osdaction.h"

class KScreenApplet : public Plasma::Applet
{
    Q_OBJECT

    /**
     * The number of currently connected (not necessarily enabled) outputs
     */
    Q_PROPERTY(int connectedOutputCount READ connectedOutputCount NOTIFY connectedOutputCountChanged)

public:
    explicit KScreenApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~KScreenApplet() override;

    enum Action {
        SwitchToExternal,
        SwitchToInternal,
        Clone,
        ExtendLeft,
        ExtendRight,
    };
    Q_ENUM(Action)

    void init() override;

    int connectedOutputCount() const;

    Q_INVOKABLE void applyLayoutPreset(KScreenApplet::Action action);

    Q_INVOKABLE QVariant availableActions();

Q_SIGNALS:
    void connectedOutputCountChanged();

private:
    void checkOutputs();

    KScreen::ConfigPtr m_screenConfiguration;
    int m_connectedOutputCount = 0;
};
