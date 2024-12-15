// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEXPORTEDINTERFACE_H
#define DEXPORTEDINTERFACE_H

#include <dtkcore_global.h>
#include <dobject.h>

#include <QObject>

#include <functional>

DCORE_BEGIN_NAMESPACE

namespace DUtil {
class DExportedInterfacePrivate;
class LIBDTKCORESHARED_EXPORT DExportedInterface : public QObject, public DObject
{
    Q_OBJECT
public:
    explicit DExportedInterface(QObject *parent = nullptr);
    ~DExportedInterface();

    void registerAction(const QString &action, const QString &description, const std::function<QVariant(QString)> handler = nullptr);
    virtual QVariant invoke(const QString &action, const QString &parameters) const;
private:
    D_DECLARE_PRIVATE(DExportedInterface)
};
}

DCORE_END_NAMESPACE

#endif
