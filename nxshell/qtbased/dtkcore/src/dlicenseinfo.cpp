// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dlicenseinfo.h"

#include <DObjectPrivate>

#include <QFile>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

class DLicenseInfo::DComponentInfoPrivate : public DObjectPrivate
{
public:
    QString name;
    QString version;
    QString copyRight;
    QString licenseName;

protected:
    explicit DComponentInfoPrivate(DLicenseInfo::DComponentInfo *qq)
        : DObjectPrivate(qq)
    {
    }

private:
    Q_DECLARE_PUBLIC(DLicenseInfo::DComponentInfo)
    friend class DLicenseInfoPrivate;
};

DLicenseInfo::DComponentInfo::DComponentInfo(DObject * parent)
    : DObject(*new DLicenseInfo::DComponentInfoPrivate(this), parent)
{

}

DLicenseInfo::DComponentInfo::~DComponentInfo()
{
}

QString DLicenseInfo::DComponentInfo::name() const
{
    return d_func()->name;
}

QString DLicenseInfo::DComponentInfo::version() const
{
    return d_func()->version;
}

QString DLicenseInfo::DComponentInfo::copyRight() const
{
    return d_func()->copyRight;
}

QString DLicenseInfo::DComponentInfo::licenseName() const
{
    return d_func()->licenseName;
}

class Q_DECL_HIDDEN DLicenseInfoPrivate : public DObjectPrivate
{
public:
    explicit DLicenseInfoPrivate(DLicenseInfo *qq);
    ~DLicenseInfoPrivate() override;

    bool loadFile(const QString &file);
    bool loadContent(const QByteArray &content);
    QByteArray licenseContent(const QString &licenseName);
    void clear();

    QString licenseSearchPath;
    DLicenseInfo::DComponentInfos componentInfos;
};

DLicenseInfoPrivate::DLicenseInfoPrivate(DLicenseInfo *qq)
    : DObjectPrivate(qq)
{
}

DLicenseInfoPrivate::~DLicenseInfoPrivate()
{
    clear();
}

bool DLicenseInfoPrivate::loadFile(const QString &file)
{
    QFile jsonFile(file);
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qWarning() << QString("Failed on open file: \"%1\", error message: \"%2\"").arg(
            qPrintable(jsonFile.fileName()), qPrintable(jsonFile.errorString()));
        return false;
    }
    return loadContent(jsonFile.readAll());
}

bool DLicenseInfoPrivate::loadContent(const QByteArray &content)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(content, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "When loading the license, parseJson failed:" << qPrintable(error.errorString());
        return false;
    }
    if (!jsonDoc.isArray()) {
        qWarning() << "When loading the license, parseJson failed: it is not a JSON array";
        return false;
    }

    clear();
    QJsonArray array = jsonDoc.array();
    for (const QJsonValue &value : array) {
        if (!value.isObject()) {
            qWarning() << "When loading the license, parseJson failed: it is not a JSON object!";
            return false;
        }
        DLicenseInfo::DComponentInfo *componentInfo = new DLicenseInfo::DComponentInfo;
        QJsonObject obj = value.toObject();
        QJsonValue name = obj.value("name");
        QJsonValue version = obj.value("version");
        QJsonValue copyright = obj.value("copyright");
        QJsonValue license = obj.value("license");
        if (!name.isString() || !version.isString()
            || !copyright.isString() || !license.isString()) {
            qWarning() << "When loading the license, parseJson failed: it is not a string!";
            return false;
        }
        componentInfo->d_func()->name = name.toString();
        componentInfo->d_func()->version = version.toString();
        componentInfo->d_func()->copyRight = copyright.toString();
        componentInfo->d_func()->licenseName = license.toString();
        componentInfos.append(componentInfo);
    }
    return true;
}

QByteArray DLicenseInfoPrivate::licenseContent(const QString &licenseName)
{
    QByteArray content;
    QStringList dirs{"/usr/share/spdx-license"};
    if (!licenseSearchPath.isEmpty())
        dirs.prepend(licenseSearchPath);
    for (const QString &dir : dirs) {
        QFile file(QString("%1/%2.txt").arg(dir).arg(licenseName));
        if (!file.exists())
            continue;
        if (file.open(QIODevice::ReadOnly)) {
            content = file.readAll();
            file.close();
            break;
        }
    }
    if (content.isEmpty()) {
        qWarning() << QString("License content is empty when getting license content!");
    }
    return content;
}

void DLicenseInfoPrivate::clear()
{
    qDeleteAll(componentInfos);
    componentInfos.clear();
}

DLicenseInfo::DLicenseInfo(DObject *parent)
    : DObject(*new DLicenseInfoPrivate(this), parent)
{
}

bool DLicenseInfo::loadContent(const QByteArray &content)
{
    D_D(DLicenseInfo);
    return d->loadContent(content);
}

bool DLicenseInfo::loadFile(const QString &file)
{
    D_D(DLicenseInfo);
    return d->loadFile(file);
}

void DLicenseInfo::setLicenseSearchPath(const QString &path)
{
    D_D(DLicenseInfo);
    d->licenseSearchPath = path;
}

QByteArray DLicenseInfo::licenseContent(const QString &licenseName)
{
    D_D(DLicenseInfo);
    return d->licenseContent(licenseName);
}

DLicenseInfo::DComponentInfos DLicenseInfo::componentInfos() const
{
    D_DC(DLicenseInfo);
    return d->componentInfos;
}

DCORE_END_NAMESPACE
