// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGER_H
#define CONFIGER_H

#include "userpreference.h"

#include <QString>

#define ConfigerIns  GrandSearch::Configer::instance()

namespace GrandSearch {

class ConfigerPrivate;
class Configer : public QObject
{
    Q_OBJECT
    friend class ConfigerPrivate;
public:
    static Configer *instance();
    bool init();
    UserPreferencePointer group(const QString &name) const;
protected:
    void initDefault();
protected slots:
    void onFileChanged(const QString &path);
    void onLoadConfig();
protected:
    explicit Configer(QObject *parent = nullptr);
    ~Configer();
private:
    ConfigerPrivate *d;
};

}

#endif // CONFIGER_H
