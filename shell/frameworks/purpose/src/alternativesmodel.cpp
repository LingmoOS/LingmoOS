/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "alternativesmodel.h"

#if HAVE_QTDBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#include <QDebug>
#include <QDirIterator>
#include <QIcon>
#include <QJsonArray>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KJsonUtils>
#include <KPluginMetaData>
#include <KSharedConfig>

#include "configuration.h"
#include "helper.h"
#include "purpose_external_process_debug.h"

using namespace Purpose;

static const QStringList s_defaultDisabledPlugins = {QStringLiteral("saveasplugin")};

typedef bool (*matchFunction)(const QString &constraint, const QJsonValue &value);

static bool defaultMatch(const QString &constraint, const QJsonValue &value)
{
    return value == QJsonValue(constraint);
}

static bool mimeTypeMatch(const QString &constraint, const QJsonValue &value)
{
    if (value.isArray()) {
        const auto array = value.toArray();
        for (const QJsonValue &val : array) {
            if (mimeTypeMatch(constraint, val)) {
                return true;
            }
        }
        return false;
    } else if (value.isObject()) {
        for (const QJsonValue &val : value.toObject()) {
            if (mimeTypeMatch(constraint, val)) {
                return true;
            }
        }
        return false;
    } else if (constraint.contains(QLatin1Char('*'))) {
        const QRegularExpression re(QRegularExpression::wildcardToRegularExpression(constraint), QRegularExpression::CaseInsensitiveOption);
        return re.match(value.toString()).hasMatch();
    } else {
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForName(value.toString());
        return mime.inherits(constraint);
    }
}

static bool dbusMatch(const QString &constraint, const QJsonValue &value)
{
    Q_UNUSED(value)
#if HAVE_QTDBUS
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(constraint);
#else
    Q_UNUSED(constraint)
    return false;
#endif
}

static bool executablePresent(const QString &constraint, const QJsonValue &value)
{
    Q_UNUSED(value)
    return !QStandardPaths::findExecutable(constraint).isEmpty();
}

static bool desktopFilePresent(const QString &constraint, const QJsonValue &value)
{
    Q_UNUSED(value)
    return !QStandardPaths::locate(QStandardPaths::ApplicationsLocation, constraint).isEmpty();
}

static QMap<QString, matchFunction> s_matchFunctions = {
    {QStringLiteral("mimeType"), mimeTypeMatch},
    {QStringLiteral("dbus"), dbusMatch},
    {QStringLiteral("application"), desktopFilePresent},
    {QStringLiteral("exec"), executablePresent},
};

class Purpose::AlternativesModelPrivate
{
public:
    QList<KPluginMetaData> m_plugins;
    QJsonObject m_inputData;
    QString m_pluginType;
    QStringList m_disabledPlugins = s_defaultDisabledPlugins;
    QJsonObject m_pluginTypeData;
    const QRegularExpression constraintRx{QStringLiteral("(\\w+):(.*)")};

    bool isPluginAcceptable(const KPluginMetaData &meta, const QStringList &disabledPlugins) const
    {
        const QJsonObject obj = meta.rawData();
        if (!obj.value(QLatin1String("X-Purpose-PluginTypes")).toArray().contains(m_pluginType)) {
            // qCDebug(PURPOSE_EXTERNAL_PROCESS_LOG) << "discarding" << meta.name() << KPluginMetaData::readStringList(meta.rawData(),
            // QStringLiteral("X-Purpose-PluginTypes"));
            return false;
        }

        if (disabledPlugins.contains(meta.pluginId()) || m_disabledPlugins.contains(meta.pluginId())) {
            // qCDebug(PURPOSE_EXTERNAL_PROCESS_LOG) << "disabled plugin" << meta.name() << meta.pluginId();
            return false;
        }

        // All constraints must match
        const QJsonArray constraints = obj.value(QLatin1String("X-Purpose-Constraints")).toArray();
        for (const QJsonValue &constraint : constraints) {
            if (!constraintMatches(meta, constraint)) {
                return false;
            }
        }
        return true;
    }

    bool constraintMatches(const KPluginMetaData &meta, const QJsonValue &constraint) const
    {
        // Treat an array as an OR
        if (constraint.isArray()) {
            const QJsonArray options = constraint.toArray();
            for (const auto &option : options) {
                if (constraintMatches(meta, option)) {
                    return true;
                }
            }
            return false;
        }
        Q_ASSERT(constraintRx.isValid());
        QRegularExpressionMatch match = constraintRx.match(constraint.toString());
        if (!match.isValid() || !match.hasMatch()) {
            qCWarning(PURPOSE_EXTERNAL_PROCESS_LOG) << "wrong constraint" << constraint.toString();
            return false;
        }
        const QString propertyName = match.captured(1);
        const QString constrainedValue = match.captured(2);
        const bool acceptable = s_matchFunctions.value(propertyName, defaultMatch)(constrainedValue, m_inputData.value(propertyName));
        if (!acceptable) {
            // qCDebug(PURPOSE_EXTERNAL_PROCESS_LOG) << "not accepted" << meta.name() << propertyName << constrainedValue << m_inputData[propertyName];
        }
        return acceptable;
    }
};

AlternativesModel::AlternativesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new AlternativesModelPrivate)
{
}

AlternativesModel::~AlternativesModel()
{
    Q_D(AlternativesModel);
    delete d;
}

QHash<int, QByteArray> AlternativesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(IconNameRole, QByteArrayLiteral("iconName"));
    roles.insert(PluginIdRole, QByteArrayLiteral("pluginId"));
    roles.insert(ActionDisplayRole, QByteArrayLiteral("actionDisplay"));
    return roles;
}

void AlternativesModel::setInputData(const QJsonObject &input)
{
    Q_D(AlternativesModel);
    if (input == d->m_inputData) {
        return;
    }

    d->m_inputData = input;
    initializeModel();

    Q_EMIT inputDataChanged();
}

void AlternativesModel::setPluginType(const QString &pluginType)
{
    Q_D(AlternativesModel);
    if (pluginType == d->m_pluginType) {
        return;
    }

    d->m_pluginTypeData = Purpose::readPluginType(pluginType);
    d->m_pluginType = pluginType;
    Q_ASSERT(d->m_pluginTypeData.isEmpty() == d->m_pluginType.isEmpty());

    initializeModel();

    Q_EMIT pluginTypeChanged();
}

QStringList AlternativesModel::disabledPlugins() const
{
    Q_D(const AlternativesModel);
    return d->m_disabledPlugins;
}

void AlternativesModel::setDisabledPlugins(const QStringList &pluginIds)
{
    Q_D(AlternativesModel);
    if (pluginIds == d->m_disabledPlugins) {
        return;
    }

    d->m_disabledPlugins = pluginIds;

    initializeModel();

    Q_EMIT disabledPluginsChanged();
}

QString AlternativesModel::pluginType() const
{
    Q_D(const AlternativesModel);
    return d->m_pluginType;
}

QJsonObject AlternativesModel::inputData() const
{
    Q_D(const AlternativesModel);
    return d->m_inputData;
}

Purpose::Configuration *AlternativesModel::configureJob(int row)
{
    Q_D(AlternativesModel);
    const KPluginMetaData pluginData = d->m_plugins.at(row);
    return new Configuration(d->m_inputData, d->m_pluginType, d->m_pluginTypeData, pluginData, this);
}

int AlternativesModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const AlternativesModel);
    return parent.isValid() ? 0 : d->m_plugins.count();
}

QVariant AlternativesModel::data(const QModelIndex &index, int role) const
{
    Q_D(const AlternativesModel);
    if (!index.isValid() || index.row() > d->m_plugins.count()) {
        return QVariant();
    }

    KPluginMetaData data = d->m_plugins[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return data.name();
    case Qt::ToolTipRole:
        return data.description();
    case IconNameRole:
        return data.iconName();
    case Qt::DecorationRole:
        return QIcon::fromTheme(data.iconName());
    case PluginIdRole:
        return data.pluginId();
    case ActionDisplayRole: {
        const QJsonObject pluginData = data.rawData().value(QLatin1String("KPlugin")).toObject();
        const QString action = KJsonUtils::readTranslatedString(pluginData, QStringLiteral("X-Purpose-ActionDisplay"));
        return action.isEmpty() ? data.name() : action;
    }
    }
    return QVariant();
}

static QList<KPluginMetaData> findScriptedPackages(std::function<bool(const KPluginMetaData &)> filter)
{
    QList<KPluginMetaData> ret;
    QSet<QString> addedPlugins;
    const QStringList dirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kpackage/Purpose"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        QDirIterator dirIt(dir, QDir::Dirs | QDir::NoDotAndDotDot);

        while (dirIt.hasNext()) {
            QDir dir(dirIt.next());
            Q_ASSERT(dir.exists());
            if (!dir.exists(QStringLiteral("metadata.json"))) {
                continue;
            }

            const KPluginMetaData info = Purpose::createMetaData(dir.absoluteFilePath(QStringLiteral("metadata.json")));
            if (!addedPlugins.contains(info.pluginId()) && filter(info)) {
                addedPlugins << info.pluginId();
                ret += info;
            }
        }
    }

    return ret;
}

void AlternativesModel::initializeModel()
{
    Q_D(AlternativesModel);
    if (d->m_pluginType.isEmpty()) {
        return;
    }

    const QJsonArray inbound = d->m_pluginTypeData.value(QLatin1String("X-Purpose-InboundArguments")).toArray();
    for (const QJsonValue &arg : inbound) {
        const auto key = arg.toString();
        if (!d->m_inputData.contains(key)) {
            qCWarning(PURPOSE_EXTERNAL_PROCESS_LOG).nospace()
                << "Cannot initialize model for plugin type " << d->m_pluginType << " with data " << d->m_inputData << ": missing key " << key;
            return;
        }
    }

    const auto config = KSharedConfig::openConfig(QStringLiteral("purposerc"));
    const auto group = config->group(QStringLiteral("plugins"));
    const QStringList disabledPlugins = group.readEntry("disabled", QStringList());
    auto pluginAcceptable = [d, disabledPlugins](const KPluginMetaData &meta) {
        return d->isPluginAcceptable(meta, disabledPlugins);
    };

    beginResetModel();
    d->m_plugins.clear();
    d->m_plugins << KPluginMetaData::findPlugins(QStringLiteral("kf6/purpose"), pluginAcceptable);
    d->m_plugins += findScriptedPackages(pluginAcceptable);
    endResetModel();
}

#include "moc_alternativesmodel.cpp"
