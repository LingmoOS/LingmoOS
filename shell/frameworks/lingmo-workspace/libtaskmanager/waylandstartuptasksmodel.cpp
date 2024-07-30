/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "waylandstartuptasksmodel.h"
#include "libtaskmanager_debug.h"
#include "tasktools.h"

#include <KConfigGroup>
#include <KConfigWatcher>

#include <qwayland-lingmo-window-management.h>

#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QtWaylandClient/QWaylandClientExtensionTemplate>

using namespace Qt::StringLiterals;

namespace TaskManager
{

class LingmoActivation : public QObject, public QtWayland::org_kde_lingmo_activation
{
    Q_OBJECT
public:
    LingmoActivation(::org_kde_lingmo_activation *object)
        : QtWayland::org_kde_lingmo_activation(object)
    {
    }
    ~LingmoActivation() override
    {
        destroy();
    }
    void org_kde_lingmo_activation_app_id(const QString &app_id) override
    {
        Q_EMIT appId(app_id);
    }
    void org_kde_lingmo_activation_finished() override
    {
        Q_EMIT finished();
    }
Q_SIGNALS:
    void appId(const QString &appId);
    void finished();
};
class LingmoActivationFeedback : public QWaylandClientExtensionTemplate<LingmoActivationFeedback>, public QtWayland::org_kde_lingmo_activation_feedback
{
    Q_OBJECT
public:
    LingmoActivationFeedback()
        : QWaylandClientExtensionTemplate(1)
    {
        connect(this, &QWaylandClientExtension::activeChanged, this, [this] {
            if (!isActive()) {
                destroy();
            }
        });
    }
    ~LingmoActivationFeedback()
    {
        if (isActive()) {
            destroy();
        }
    }
Q_SIGNALS:
    void newActivation(LingmoActivation *activation);

protected:
    void org_kde_lingmo_activation_feedback_activation(::org_kde_lingmo_activation *id) override
    {
        Q_EMIT newActivation(new LingmoActivation(id));
    }
};

class Q_DECL_HIDDEN WaylandStartupTasksModel::Private
{
public:
    Private(WaylandStartupTasksModel *q);

    void addActivation(LingmoActivation *activation);
    void removeActivation(LingmoActivation *activation);

    void init();
    void loadConfig();

    struct Startup {
        QString name;
        QIcon icon;
        QString applicationId;
        QUrl launcherUrl;
        std::unique_ptr<LingmoActivation> activation;
    };

    WaylandStartupTasksModel *q;
    KConfigWatcher::Ptr configWatcher = nullptr;
    std::unique_ptr<LingmoActivationFeedback> feedback = nullptr;
    std::vector<Startup> startups;
    std::chrono::seconds startupTimeout = std::chrono::seconds::zero();
};

WaylandStartupTasksModel::Private::Private(WaylandStartupTasksModel *q)
    : q(q)
{
}

void WaylandStartupTasksModel::Private::init()
{
    configWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("klaunchrc"), KConfig::NoGlobals));
    QObject::connect(configWatcher.data(), &KConfigWatcher::configChanged, q, [this] {
        loadConfig();
    });

    loadConfig();
}

void WaylandStartupTasksModel::Private::loadConfig()
{
    KConfigGroup feedbackConfig(configWatcher->config(), u"FeedbackStyle"_s);

    if (!feedbackConfig.readEntry("TaskbarButton", true)) {
        q->beginResetModel();
        startups.clear();
        feedback.reset();
        q->endResetModel();
        return;
    }

    const KConfigGroup taskbarButtonConfig(configWatcher->config(), u"TaskbarButtonSettings"_s);
    startupTimeout = std::chrono::seconds(taskbarButtonConfig.readEntry("Timeout", 5));

    feedback = std::make_unique<LingmoActivationFeedback>();

    QObject::connect(feedback.get(), &LingmoActivationFeedback::activeChanged, q, [this] {
        if (!feedback->isActive()) {
            q->beginResetModel();
            startups.clear();
            q->endResetModel();
        }
    });

    QObject::connect(feedback.get(), &LingmoActivationFeedback::newActivation, q, [this](LingmoActivation *activation) {
        addActivation(activation);
    });
}

void WaylandStartupTasksModel::Private::addActivation(LingmoActivation *activation)
{
    QObject::connect(activation, &LingmoActivation::appId, q, [this, activation](const QString &appId) {
        // The application id is guaranteed to be the desktop filename without ".desktop"
        const QString desktopFileName = appId + QLatin1String(".desktop");
        const QString desktopFilePath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, desktopFileName);
        if (desktopFilePath.isEmpty()) {
            qCWarning(TASKMANAGER_DEBUG) << "Got invalid activation app_id:" << appId;
            return;
        }

        const QUrl launcherUrl(QString(u"applications:" + desktopFileName));
        const AppData appData = appDataFromUrl(QUrl::fromLocalFile(desktopFilePath));

        const int count = startups.size();
        q->beginInsertRows(QModelIndex(), count, count);
        startups.push_back(Startup{
            .name = appData.name,
            .icon = appData.icon,
            .applicationId = appId,
            .launcherUrl = launcherUrl,
            .activation = std::unique_ptr<LingmoActivation>(activation),
        });
        q->endInsertRows();

        // Remove the activation if it doesn't finish within certain time interval.
        QTimer *timeoutTimer = new QTimer(activation);
        QObject::connect(timeoutTimer, &QTimer::timeout, q, [this, activation]() {
            removeActivation(activation);
        });
        timeoutTimer->setSingleShot(true);
        timeoutTimer->start(startupTimeout);
    });

    QObject::connect(activation, &LingmoActivation::finished, q, [this, activation]() {
        removeActivation(activation);
    });
}

void WaylandStartupTasksModel::Private::removeActivation(LingmoActivation *activation)
{
    auto it = std::find_if(startups.begin(), startups.end(), [activation](const Startup &startup) {
        return startup.activation.get() == activation;
    });
    if (it == startups.end()) {
        return;
    }
    const int position = std::distance(startups.begin(), it);
    q->beginRemoveRows(QModelIndex(), position, position);
    startups.erase(it);
    q->endRemoveRows();
}

WaylandStartupTasksModel::WaylandStartupTasksModel(QObject *parent)
    : AbstractTasksModel(parent)
    , d(new Private(this))
{
    d->init();
}

WaylandStartupTasksModel::~WaylandStartupTasksModel()
{
}

QVariant WaylandStartupTasksModel::data(const QModelIndex &index, int role) const
{
    // Note: when index is valid, its row >= 0, so casting to unsigned is safe
    if (!index.isValid() || static_cast<size_t>(index.row()) >= d->startups.size()) {
        return QVariant();
    }

    const auto &data = d->startups[index.row()];
    if (role == Qt::DisplayRole) {
        return data.name;
    } else if (role == Qt::DecorationRole) {
        return data.icon;
    } else if (role == AppId) {
        return data.applicationId;
    } else if (role == AppName) {
        return data.name;
    } else if (role == LauncherUrl || role == LauncherUrlWithoutIcon) {
        return data.launcherUrl;
    } else if (role == IsStartup) {
        return true;
    } else if (role == CanLaunchNewInstance) {
        return false;
    } else if (role == IsOnAllVirtualDesktops) {
        return true;
    }

    return AbstractTasksModel::data(index, role);
}

int WaylandStartupTasksModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->startups.size();
}

} // namespace TaskManager

#include "waylandstartuptasksmodel.moc"
