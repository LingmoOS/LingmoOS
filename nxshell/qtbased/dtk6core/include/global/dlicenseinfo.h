// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DLICENSEINFO_H
#define DLICENSEINFO_H

#include <QVector>

#include <dtkcore_global.h>
#include <dobject.h>

DCORE_BEGIN_NAMESPACE

class DLicenseInfoPrivate;
class LIBDTKCORESHARED_EXPORT DLicenseInfo : public DObject
{
public:
    explicit DLicenseInfo(DObject *parent = nullptr);

    bool loadContent(const QByteArray &content);
    bool loadFile(const QString &file);
    void setLicenseSearchPath(const QString &path);
    QByteArray licenseContent(const QString &licenseName);

    class DComponentInfoPrivate;
    class LIBDTKCORESHARED_EXPORT DComponentInfo : public DObject
    {
    public:
        explicit DComponentInfo(DObject *parent = nullptr);
        ~DComponentInfo() override;

        QString name() const;
        QString version() const;
        QString copyRight() const;
        QString licenseName() const;

    private:
        D_DECLARE_PRIVATE(DComponentInfo)
        friend class DLicenseInfoPrivate;
    };
    using DComponentInfos = QVector<DComponentInfo*>;
    DComponentInfos componentInfos() const;

private:
    D_DECLARE_PRIVATE(DLicenseInfo)
};

DCORE_END_NAMESPACE

#endif // DLICENSEINFO_H
