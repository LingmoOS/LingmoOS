/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "purpose/configuration.h"
#include "externalprocess/processjob.h"
#include <QFileInfo>

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <qregularexpression.h>

#include "helper.h"
#include "pluginbase.h"

using namespace Purpose;

class Purpose::ConfigurationPrivate
{
public:
    QJsonObject m_inputData;
    QString m_pluginTypeName;
    QJsonObject m_pluginType;
    const KPluginMetaData m_pluginData;
    bool m_useSeparateProcess;

    static void checkJobFinish(KJob *job)
    {
        const QStringList outputArgs = job->property("outputArgs").toStringList();
        const auto argsSet = QSet<QString>{outputArgs.cbegin(), outputArgs.cend()};

        const QStringList outputKeys = job->property("output").toJsonObject().keys();
        const auto keysSet = QSet<QString>{outputKeys.cbegin(), outputKeys.cend()};

        if (!keysSet.contains(argsSet) && job->error() == 0) {
            qWarning() << "missing output values for" << job->metaObject()->className() << ". Expected: " << outputArgs.join(QStringLiteral(", "))
                       << ". Got: " << outputKeys.join(QStringLiteral(", "));
        }
    }

    Purpose::Job *internalCreateJob(QObject *parent) const
    {
        if (m_useSeparateProcess) {
            return new ProcessJob(m_pluginData.fileName(), m_pluginTypeName, m_inputData, parent);
        } else {
            return createJob(parent);
        }
    }

    Purpose::Job *createJob(QObject *parent) const
    {
        if (m_pluginData.fileName().contains(QLatin1String("contents/code/main."))) {
            return new ProcessJob(m_pluginData.fileName(), m_pluginTypeName, m_inputData, parent);
        } else {
            auto pluginResult = KPluginFactory::instantiatePlugin<QObject>(m_pluginData, parent, QVariantList());

            if (!pluginResult) {
                qWarning() << "Couldn't load plugin:" << m_pluginData.fileName() << pluginResult.errorString;
                return nullptr;
            }

            Purpose::PluginBase *plugin = dynamic_cast<Purpose::PluginBase *>(pluginResult.plugin);
            return plugin->createJob();
        }
    }
};

Configuration::Configuration(const QJsonObject &inputData, const QString &pluginTypeName, const KPluginMetaData &pluginInformation, QObject *parent)
    : Configuration(inputData, pluginTypeName, QJsonObject(), pluginInformation, parent)
{
}

Configuration::Configuration(const QJsonObject &inputData,
                             const QString &pluginTypeName,
                             const QJsonObject &pluginType,
                             const KPluginMetaData &pluginInformation,
                             QObject *parent)
    : QObject(parent)
    , d_ptr(new ConfigurationPrivate{inputData, pluginTypeName, pluginType, pluginInformation, !qEnvironmentVariableIsSet("KDE_PURPOSE_LOCAL_JOBS")})
{
}

Configuration::~Configuration()
{
    delete d_ptr;
}

void Configuration::setData(const QJsonObject &data)
{
    Q_D(Configuration);

    //     qDebug() << "datachanged" << data;
    if (d->m_inputData != data) {
        d->m_inputData = data;
        Q_EMIT dataChanged();
    }
}

QJsonObject Configuration::data() const
{
    Q_D(const Configuration);
    return d->m_inputData;
}

bool Configuration::isReady() const
{
    Q_D(const Configuration);
    bool ok = true;
    const auto arguments = neededArguments();
    for (const QJsonValue &arg : arguments) {
        if (!d->m_inputData.contains(arg.toString())) {
            qDebug() << "missing mandatory argument" << arg.toString();
            ok = false;
        }
    }
    return ok;
}

QJsonArray Configuration::neededArguments() const
{
    Q_D(const Configuration);
    QJsonArray ret = d->m_pluginType.value(QLatin1String("X-Purpose-InboundArguments")).toArray();
    const QJsonArray arr = d->m_pluginData.rawData().value(QLatin1String("X-Purpose-Configuration")).toArray();
    for (const QJsonValue &val : arr) {
        ret += val;
    }
    return ret;
}

Purpose::Job *Configuration::createJob()
{
    if (!isReady()) {
        return nullptr;
    }

    Q_D(const Configuration);

    Purpose::Job *job = d->internalCreateJob(this);
    if (!job) {
        return job;
    }

    job->setData(d->m_inputData);
    job->setProperty("outputArgs", d->m_pluginType.value(QLatin1String("X-Purpose-OutboundArguments")));

    connect(job, &Purpose::Job::finished, &ConfigurationPrivate::checkJobFinish);
    return job;
}

QUrl Configuration::configSourceCode() const
{
    Q_D(const Configuration);
    if (d->m_pluginData.fileName().contains(QLatin1String("contents/code/main."))) {
        const QFileInfo fi(d->m_pluginData.fileName());
        QDir conentsDir = fi.dir();
        conentsDir.cdUp();
        return QUrl::fromLocalFile(conentsDir.filePath(QStringLiteral("config/config.qml")));
    } else {
        const QString configFile =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf6/purpose/%1_config.qml").arg(d->m_pluginData.pluginId()));
        if (configFile.isEmpty()) {
            return QUrl();
        }

        return QUrl::fromLocalFile(configFile);
    }
}

bool Configuration::useSeparateProcess() const
{
    Q_D(const Configuration);
    return d->m_useSeparateProcess;
}

void Configuration::setUseSeparateProcess(bool use)
{
    Q_D(Configuration);
    d->m_useSeparateProcess = use;
}

QString Configuration::pluginTypeName() const
{
    Q_D(const Configuration);
    KPluginMetaData md(d->m_pluginType, {});
    return md.name();
}

QString Configuration::pluginName() const
{
    Q_D(const Configuration);
    return d->m_pluginData.name();
}

#include "moc_configuration.cpp"
