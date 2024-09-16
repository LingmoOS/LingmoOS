// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <DObject>
#include <QQmlEngine>

DS_BEGIN_NAMESPACE

class DApplet;
class DQmlEnginePrivate;
/**
 * @brief UI插件实例项
 */
class DS_SHARE DQmlEngine : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DQmlEngine)
public:
    explicit DQmlEngine(QObject *parent = nullptr);
    explicit DQmlEngine(DApplet *applet, QObject *parent = nullptr);
    virtual ~DQmlEngine() override;

    QObject *rootObject() const;

    QQmlEngine *engine();

    static QObject *createObject(const QUrl &url);
    static QObject *createObject(const QUrl &url, const QVariantMap &initialProperties);

    bool create();
    void completeCreate();

Q_SIGNALS:
    void finished();
    void createFinished();
};

DS_END_NAMESPACE
