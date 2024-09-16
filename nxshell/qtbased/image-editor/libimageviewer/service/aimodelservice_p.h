// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AIMODELSERVICE_P_H
#define AIMODELSERVICE_P_H

#include "aimodelservice.h"

#include <QMap>
#include <QString>
#include <QSharedPointer>
#include <QFuture>
#include <QFutureWatcher>
#include <QTemporaryDir>
#include <QDBusInterface>
#include <QBasicTimer>

#include <DFloatingMessage>

DWIDGET_USE_NAMESPACE

// 模型信息
struct ModelInfo
{
    int modelID;      // 模型ID AIModelServiceData::EnhanceType
    QString model;    // 模型名称
    QString modelTr;  // 模型翻译名称
};

typedef QSharedPointer<ModelInfo> ModelPtr;

// 图像增强处理信息
struct EnhanceInfo
{
    const QString source;
    const QString output;
    const QString model;
    int index = 0;

    QAtomicInt state = AIModelService::None;  // 处理状态，可能有争用

    EnhanceInfo(const QString &s, const QString &o, const QString &m)
        : source(s)
        , output(o)
        , model(m)
    {
    }
};

typedef QSharedPointer<EnhanceInfo> EnhancePtr;

class AIModelServiceData
{
public:
    // 图像增强服务类型
    enum EnhanceType {
        Coloring,
        SuperResol,      // 超分辨率
        BackgroundBlur,  // 模糊背景
        BackgroundCut,   // 删除背景(人像抠图)
        Hand,
        Cartoon2D,
        Cartoon3D,
        Sketch,
        UserType = 1024,  // 用户类型，后续可能增强其它模型
    };

    // DBus接口反馈错误
    enum DBusError {
        DBusNoError = 0,
        DBusFailed = -1,      // 执行失败
        DBusNoPortrait = -2,  // 未检测人像
    };

    explicit AIModelServiceData(AIModelService *q);
    QList<QPair<int, QString>> initDBusModelList();
    ModelPtr createModelInfo(const QString &model);

    DFloatingMessage *createReloadMessage(const QString &output);
    static bool sendImageEnhance(const QString &source, const QString &output, const QString &model);

    void startDBusTimer();
    void stopDBusTimer();

    AIModelService *qptr = nullptr;
    int userModelCount = 0;  // 非默认的模型统计
    QMap<int, ModelPtr> mapModelInfo;
    QList<QPair<int, QString>> supportNameToModel;  // 缓存的支持模型列表<模型ID，名称>

    QString lastOutput;                       // 最近的图像增强输出文件
    QScopedPointer<QTemporaryDir> enhanceTemp;                // 图像增强文件临时目录
    QHash<QString, EnhancePtr> enhanceCache;  // 图像增强缓存信息（仅主线程访问）

    QMutex cacheMutex;
    QScopedPointer<QTemporaryDir> convertTemp;             // 图像类型转换文件临时目录
    QHash<QString, QString> convertCache;  // 缓存的信息，可能多个线程访问

    QFutureWatcher<EnhancePtr> enhanceWatcher;

    bool waitSave = false;  // 是否在等待保存操作结束
    QBasicTimer dbusTimer;  // DBus处理超时定时器
};

#endif  // AIMODELSERVICE_P_H
