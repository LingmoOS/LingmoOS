// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "containment.h"

#include <QQuickWindow>

DS_BEGIN_NAMESPACE

/**
 * @brief 插件集
 */
class DPanelPrivate;
class DS_SHARE DPanel : public DContainment
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DPanel)
    Q_PROPERTY(QQuickWindow *popupWindow READ popupWindow NOTIFY popupWindowChanged)
    Q_PROPERTY(QQuickWindow *toolTipWindow READ toolTipWindow NOTIFY toolTipWindowChanged)
    Q_PROPERTY(QQuickWindow *menuWindow READ menuWindow NOTIFY menuWindowChanged)
public:
    explicit DPanel(QObject *parent = nullptr);
    virtual ~DPanel() override;

    QQuickWindow *window() const;

    QQuickWindow *popupWindow() const;
    QQuickWindow *toolTipWindow() const;
    QQuickWindow *menuWindow() const;

    // 加载插件
    virtual bool load() override;
    // 初始化
    virtual bool init() override;

    static DPanel *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void popupWindowChanged();
    void toolTipWindowChanged();
    void menuWindowChanged();
};

DS_END_NAMESPACE

QML_DECLARE_TYPEINFO(DS_NAMESPACE::DPanel, QML_HAS_ATTACHED_PROPERTIES)
