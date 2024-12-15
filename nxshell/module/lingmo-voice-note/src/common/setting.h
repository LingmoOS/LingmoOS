// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTING_H
#define SETTING_H

#include <qsettingbackend.h>

#include <DSettings>
#include <DSettingsOption>

#include <QSettings>
#include <QMutex>

DCORE_USE_NAMESPACE
//重写设置后端数据管理，兼容sp2参数读取设置
class CustemBackend : public DSettingsBackend
{
    Q_OBJECT
public:
    explicit CustemBackend(const QString &filepath, QObject *parent = nullptr);
    ~CustemBackend() override;
    //获取参数所有的key值
    QStringList keys() const override;
    //根据key获取内容
    QVariant getOption(const QString &key) const override;
    //同步数据
    void doSync() override;
    //根据key值设置内容
    void doSetOption(const QString &key, const QVariant &value) override;

private:
    QSettings *m_settings {nullptr};
    QMutex m_writeLock;
};

class setting : public QObject
{
    Q_OBJECT
public:
    //设置配置参数
    void setOption(const QString &key, const QVariant &value);
    //获取配置参数
    QVariant getOption(const QString &key);
    //获取参数管理对象
    DSettings *getSetting();
    //单例
    static setting *instance();

protected:
    setting();

private:
    CustemBackend *m_backend {nullptr};
    DSettings *m_setting {nullptr};
};

#endif // SETTING_H
