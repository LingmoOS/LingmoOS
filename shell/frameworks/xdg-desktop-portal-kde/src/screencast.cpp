/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 */

#include "screencast.h"
#include "notificationinhibition.h"
#include "request.h"
#include "restoredata.h"
#include "screencast_debug.h"
#include "screenchooserdialog.h"
#include "session.h"
#include "utils.h"
#include "waylandintegration.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWayland/Client/plasmawindowmodel.h>

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDataStream>
#include <QGuiApplication>
#include <QIODevice>

ScreenCastPortal::ScreenCastPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<RestoreData>();
}

ScreenCastPortal::~ScreenCastPortal()
{
}

bool ScreenCastPortal::inhibitionsEnabled() const
{
    if (!WaylandIntegration::isStreamingAvailable()) {
        return false;
    }

    auto cfg = KSharedConfig::openConfig(QStringLiteral("plasmanotifyrc"));

    KConfigGroup grp(cfg, "DoNotDisturb");

    return grp.readEntry("WhenScreenSharing", true);
}

uint ScreenCastPortal::CreateSession(const QDBusObjectPath &handle,
                                     const QDBusObjectPath &session_handle,
                                     const QString &app_id,
                                     const QVariantMap &options,
                                     QVariantMap &results)
{
    Q_UNUSED(results)

    qCDebug(XdgDesktopPortalKdeScreenCast) << "CreateSession called with parameters:";
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    options: " << options;

    Session *session = Session::createSession(this, Session::ScreenCast, app_id, session_handle.path());

    if (!session) {
        return 2;
    }

    if (!WaylandIntegration::isStreamingAvailable()) {
        qCWarning(XdgDesktopPortalKdeScreenCast) << "zkde_screencast_unstable_v1 does not seem to be available";
        return 2;
    }

    connect(session, &Session::closed, [session] {
        auto screencastSession = qobject_cast<ScreenCastSession *>(session);
        const auto streams = screencastSession->streams();
        for (const WaylandIntegration::Stream &stream : streams) {
            WaylandIntegration::stopStreaming(stream.nodeId);
        }
    });
    return 0;
}

uint ScreenCastPortal::SelectSources(const QDBusObjectPath &handle,
                                     const QDBusObjectPath &session_handle,
                                     const QString &app_id,
                                     const QVariantMap &options,
                                     QVariantMap &results)
{
    Q_UNUSED(results)

    qCDebug(XdgDesktopPortalKdeScreenCast) << "SelectSource called with parameters:";
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    options: " << options;

    ScreenCastSession *session = qobject_cast<ScreenCastSession *>(Session::getSession(session_handle.path()));

    if (!session) {
        qCWarning(XdgDesktopPortalKdeScreenCast) << "Tried to select sources on non-existing session " << session_handle.path();
        return 2;
    }

    session->setOptions(options);
    // Might be also a RemoteDesktopSession
    if (session->type() == Session::RemoteDesktop) {
        RemoteDesktopSession *remoteDesktopSession = qobject_cast<RemoteDesktopSession *>(session);
        if (remoteDesktopSession) {
            remoteDesktopSession->setScreenSharingEnabled(true);
        }
    } else {
        session->setPersistMode(ScreenCastPortal::PersistMode(options.value(QStringLiteral("persist_mode")).toUInt()));
        session->setRestoreData(options.value(QStringLiteral("restore_data")));
    }

    return 0;
}

uint ScreenCastPortal::Start(const QDBusObjectPath &handle,
                             const QDBusObjectPath &session_handle,
                             const QString &app_id,
                             const QString &parent_window,
                             const QVariantMap &options,
                             QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeScreenCast) << "Start called with parameters:";
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeScreenCast) << "    options: " << options;

    QPointer<ScreenCastSession> session = qobject_cast<ScreenCastSession *>(Session::getSession(session_handle.path()));

    if (!session) {
        qCWarning(XdgDesktopPortalKdeScreenCast) << "Tried to call start on non-existing session " << session_handle.path();
        return 2;
    }

    if (QGuiApplication::screens().isEmpty()) {
        qCWarning(XdgDesktopPortalKdeScreenCast) << "Failed to show dialog as there is no screen to select";
        return 2;
    }

    const PersistMode persist = session->persistMode();
    bool valid = false;
    QList<Output> selectedOutputs;
    QList<QMap<int, QVariant>> selectedWindows;
    QRect selectedRegion;
    if (persist != NoPersist && session->restoreData().isValid()) {
        const RestoreData restoreData = qdbus_cast<RestoreData>(session->restoreData().value<QDBusArgument>());
        if (restoreData.session == QLatin1String("KDE") && restoreData.version == RestoreData::currentRestoreDataVersion()) {
            const QVariantMap restoreDataPayload = restoreData.payload;
            const QVariantList restoreOutputs = restoreDataPayload[QStringLiteral("outputs")].toList();
            if (!restoreOutputs.isEmpty()) {
                OutputsModel model(OutputsModel::WorkspaceIncluded, this);
                for (const auto &outputUniqueId : restoreOutputs) {
                    for (int i = 0, c = model.rowCount(); i < c; ++i) {
                        const Output &iOutput = model.outputAt(i);
                        if (iOutput.uniqueId() == outputUniqueId) {
                            selectedOutputs << iOutput;
                        }
                    }
                }
                valid = selectedOutputs.count() == restoreOutputs.count();
            }

            const QStringList restoreWindows = restoreDataPayload[QStringLiteral("windows")].toStringList();
            if (!restoreWindows.isEmpty()) {
                const KWayland::Client::PlasmaWindowModel model(WaylandIntegration::plasmaWindowManagement());
                for (const QString &windowUuid : restoreWindows) {
                    for (int i = 0, c = model.rowCount(); i < c; ++i) {
                        const QModelIndex index = model.index(i, 0);

                        if (model.data(index, KWayland::Client::PlasmaWindowModel::Uuid).toString() == windowUuid) {
                            selectedWindows << model.itemData(index);
                        }
                    }
                }
                valid = selectedWindows.count() == restoreWindows.count();
            }
        }
    }

    bool allowRestore = valid;
    if (!valid) {
        QScopedPointer<ScreenChooserDialog, QScopedPointerDeleteLater> screenDialog(
            new ScreenChooserDialog(app_id, session->multipleSources(), SourceTypes(session->types())));
        connect(session, &Session::closed, screenDialog.data(), &ScreenChooserDialog::reject);
        Utils::setParentWindow(screenDialog->windowHandle(), parent_window);
        Request::makeClosableDialogRequest(handle, screenDialog.get());
        valid = screenDialog->exec();
        if (valid) {
            allowRestore = screenDialog->allowRestore();
            selectedOutputs = screenDialog->selectedOutputs();
            selectedWindows = screenDialog->selectedWindows();
            selectedRegion = screenDialog->selectedRegion();
        }
    }

    if (valid && session) {
        QVariantList outputs;
        QStringList windows;
        WaylandIntegration::Streams streams;
        Screencasting::CursorMode cursorMode = Screencasting::CursorMode(session->cursorMode());
        for (const auto &output : std::as_const(selectedOutputs)) {
            WaylandIntegration::Stream stream;
            switch (output.outputType()) {
            case Output::Region:
                stream = WaylandIntegration::startStreamingRegion(selectedRegion, cursorMode);
                break;
            case Output::Workspace:
                stream = WaylandIntegration::startStreamingWorkspace(cursorMode);
                break;
            case Output::Virtual:
                stream = WaylandIntegration::startStreamingVirtual(output.uniqueId(), {1920, 1080}, cursorMode);
                break;
            default:
                stream = WaylandIntegration::startStreamingOutput(output.screen(), cursorMode);
                break;
            }

            if (!stream.isValid()) {
                qCWarning(XdgDesktopPortalKdeScreenCast) << "Invalid screen!" << output.outputType() << output.uniqueId();
                return 2;
            }

            if (allowRestore) {
                outputs += output.uniqueId();
            }
            streams << stream;
        }
        for (const auto &win : std::as_const(selectedWindows)) {
            WaylandIntegration::Stream stream = WaylandIntegration::startStreamingWindow(win, cursorMode);
            if (!stream.isValid()) {
                qCWarning(XdgDesktopPortalKdeScreenCast) << "Invalid window!" << win;
                return 2;
            }

            if (allowRestore) {
                windows += win[KWayland::Client::PlasmaWindowModel::Uuid].toString();
            }
            streams << stream;
        }

        if (streams.isEmpty()) {
            qCWarning(XdgDesktopPortalKdeScreenCast) << "Pipewire stream is not ready to be streamed";
            return 2;
        }

        session->setStreams(streams);
        results.insert(QStringLiteral("streams"), QVariant::fromValue<WaylandIntegration::Streams>(streams));
        if (allowRestore) {
            results.insert("persist_mode", quint32(persist));
            if (persist != NoPersist) {
                const RestoreData restoreData = {"KDE",
                                                 RestoreData::currentRestoreDataVersion(),
                                                 QVariantMap{
                                                     {"outputs", outputs},
                                                     {"windows", windows},
                                                 }};
                results.insert("restore_data", QVariant::fromValue<RestoreData>(restoreData));
            }
        }

        if (inhibitionsEnabled()) {
            new NotificationInhibition(app_id, i18nc("Do not disturb mode is enabled because...", "Screen sharing in progress"), session);
        }
        qCDebug(XdgDesktopPortalKdeScreenCast) << "Screencast started successfully";
        return 0;
    }

    qCWarning(XdgDesktopPortalKdeScreenCast) << "Aborting screencast";
    return 1;
}
