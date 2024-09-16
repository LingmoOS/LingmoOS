// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "constants.h"

#include <QObject>
#include <DConfig>
#include <QScopedPointer>

DCORE_USE_NAMESPACE


namespace dock {

class DockSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint dockSize READ dockSize WRITE setDockSize NOTIFY dockSizeChanged FINAL)
    Q_PROPERTY(HideMode hidemode READ hideMode WRITE setHideMode NOTIFY hideModeChanged FINAL)
    Q_PROPERTY(Position position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(ItemAlignment itemAlignment READ itemAlignment WRITE setItemAlignment NOTIFY itemAlignmentChanged FINAL)
    Q_PROPERTY(IndicatorStyle indicatorStyle READ indicatorStyle WRITE setIndicatorStyle NOTIFY indicatorStyleChanged FINAL)
    Q_PROPERTY(QVariantMap pluginsVisible READ pluginsVisible WRITE setPluginsVisible NOTIFY pluginsVisibleChanged FINAL)
    Q_PROPERTY(bool showInPrimary READ showInPrimary WRITE setShowInPrimary NOTIFY showInPrimaryChanged FINAL)

public:
    static DockSettings* instance();

    uint dockSize();
    HideMode hideMode();
    Position position();
    ItemAlignment itemAlignment();
    IndicatorStyle indicatorStyle();
    QVariantMap pluginsVisible();
    bool showInPrimary() const;

    void setDockSize(const uint& size);
    void setHideMode(const HideMode& mode);
    void setPosition(const Position& position);
    void setItemAlignment(const ItemAlignment& alignment);
    void setIndicatorStyle(const IndicatorStyle& style);
    void setPluginsVisible(const QVariantMap & pluginsVisible);
    void setShowInPrimary(bool newShowInPrimary);

private:
    enum WriteJob {
        dockSizeJob = 0,
        hideModeJob = 1,
        positionJob = 2,
        itemAlignmentJob = 3,
        indicatorStyleJob = 4,
    };

    explicit DockSettings(QObject *parent = nullptr);
    void init();

    void addWriteJob(WriteJob job);
    inline void checkWriteJob();

Q_SIGNALS:
    void dockSizeChanged(uint size);
    void hideModeChanged(HideMode mode);
    void positionChanged(Position position);
    void itemAlignmentChanged(ItemAlignment alignment);
    void indicatorStyleChanged(IndicatorStyle style);
    void pluginsVisibleChanged(const QVariantMap &pluginsVisible);

    void showInPrimaryChanged(bool showInPrimary);

private:
    QScopedPointer<DConfig> m_dockConfig;
    QTimer* m_writeTimer;
    QList<WriteJob> m_writeJob;

    uint m_dockSize;
    HideMode m_hideMode;
    Position m_dockPosition;
    ItemAlignment m_alignment;
    IndicatorStyle m_style;
    QVariantMap m_pluginsVisible;
    bool m_showInPrimary;
};
}
