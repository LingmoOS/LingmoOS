// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSETTER_H
#define CONFIGSETTER_H

#include <QObject>
#include <QSettings>
#include <QMutex>

class LibConfigSetter : public QObject
{
    Q_OBJECT
public:
    static LibConfigSetter *instance();
    void setValue(const QString &group, const QString &key,
                  const QVariant &value);
    QVariant value(const QString &group, const QString &key,
                   const QVariant &defaultValue = QVariant());

signals:
    void valueChanged(const QString &group, const QString &key,
                      const QVariant &value);

private:
    explicit LibConfigSetter(QObject *parent = nullptr);
    ~LibConfigSetter() override;

private:
    static LibConfigSetter *m_setter;
    QSettings *m_settings;
    QMutex m_mutex;
};

#endif // CONFIGSETTER_H
