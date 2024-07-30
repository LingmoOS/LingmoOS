/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 * SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
 */

#include "inputcapture.h"

#include "inputcapture_debug.h"
#include "inputcapturebarrier.h"
#include "inputcapturedialog.h"
#include "request.h"
#include "session.h"
#include "utils.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QGuiApplication>

using namespace Qt::StringLiterals;

static QString kwinService()
{
    return u"org.kde.KWin"_s;
}

static QString kwinInputCapturePath()
{
    return u"/org/kde/KWin/EIS/InputCapture"_s;
}

static QString kwinInputCaptureManagerInterface()
{
    return u"org.kde.KWin.EIS.InputCaptureManager"_s;
}

QDBusArgument &operator<<(QDBusArgument &argument, const InputCapturePortal::zone &zone)
{
    argument.beginStructure();
    argument << zone.width << zone.height << zone.x_offset << zone.y_offset;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, InputCapturePortal::zone &zone)
{
    argument.beginStructure();
    argument >> zone.width >> zone.height >> zone.x_offset >> zone.y_offset;
    argument.endStructure();
    return argument;
}

InputCapturePortal::InputCapturePortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<zone>();
    qDBusRegisterMetaType<QList<zone>>();
    qDBusRegisterMetaType<QList<QMap<QString, QVariant>>>();
    qDBusRegisterMetaType<QPair<QPoint, QPoint>>();
    qDBusRegisterMetaType<QList<QPair<QPoint, QPoint>>>();
}

uint InputCapturePortal::CreateSession(const QDBusObjectPath &handle,
                                       const QDBusObjectPath &session_handle,
                                       const QString &app_id,
                                       const QString &parent_window,
                                       const QVariantMap &options,
                                       QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeInputCapture) << "CreateSession called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;

    auto *session = static_cast<InputCaptureSession *>(Session::createSession(this, Session::InputCapture, app_id, session_handle.path()));

    if (!session) {
        return 2;
    }

    const auto requestedCapabilities = options.value("capabilities").toUInt();
    if (requestedCapabilities == 0) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "No capabilities requested";
        return 2;
    }

    InputCaptureDialog dialog(app_id, Capabilities::fromInt(requestedCapabilities), this);
    Utils::setParentWindow(dialog.windowHandle(), parent_window);
    Request::makeClosableDialogRequest(handle, &dialog);

    if (!dialog.exec()) {
        return 1;
    }

    auto msg = QDBusMessage::createMethodCall(kwinService(), kwinInputCapturePath(), kwinInputCaptureManagerInterface(), u"addInputCapture"_s);
    msg << static_cast<int>(requestedCapabilities);
    QDBusReply<QDBusObjectPath> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Failed to create KWin input capture:" << reply.error();
        return 2;
    }
    session->connect(reply.value());

    connect(session, &Session::closed, session, [session] {
        auto msg = QDBusMessage::createMethodCall(kwinService(), kwinInputCapturePath(), kwinInputCaptureManagerInterface(), u"removeInputCapture"_s);
        msg << session->kwinInputCapture();
        QDBusConnection::sessionBus().send(msg);
    });
    connect(session, &InputCaptureSession::disabled, this, [this, session] {
        session->state = State::Disabled;
        qCDebug(XdgDesktopPortalKdeInputCapture) << "Disabled session" << session->handle();
        Q_EMIT Disabled(QDBusObjectPath(session->handle()), {});
    });
    connect(session, &InputCaptureSession::deactivated, this, [this, session](uint activationId) {
        session->state = State::Deactivated;
        qCDebug(XdgDesktopPortalKdeInputCapture) << "Deactivated session" << session->handle() << "acitvation_id" << activationId;
        Q_EMIT Deactivated(QDBusObjectPath(session->handle()), {{u"activation_id"_s, activationId}});
    });
    connect(session, &InputCaptureSession::activated, this, [this, session](uint activationId, const QPointF &cursorPosition) {
        session->state = State::Activated;
        qCDebug(XdgDesktopPortalKdeInputCapture) << "Activated session" << session->handle() << "acitvation_id" << activationId << "cursor_position"
                                                 << cursorPosition;
        Q_EMIT Activated(QDBusObjectPath(session->handle()), {{u"activation_id"_s, activationId}, {u"cursor_position"_s, cursorPosition}});
    });

    results.insert(u"capabilities"_s, requestedCapabilities);
    return 0;
}

uint InputCapturePortal::GetZones(const QDBusObjectPath &handle,
                                  const QDBusObjectPath &session_handle,
                                  const QString &app_id,
                                  const QVariantMap &options,
                                  QVariantMap &results)
{
    Q_UNUSED(results);
    qCDebug(XdgDesktopPortalKdeInputCapture) << "GetZones called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;

    auto *session = qobject_cast<InputCaptureSession *>(Session::getSession(session_handle.path()));

    if (!session) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to get zones on non-existing session " << session_handle.path();
        return 2;
    }

    auto handleZoneChange = [this, session] {
        if (session->state != State::Disabled) {
            if (!QDBusReply(session->disable()).isValid()) {
                qCWarning(XdgDesktopPortalKdeInputCapture()) << "Error disabling capture on zone change";
                session->close();
                return;
            }
        }
        session->clearBarriers();
        qCDebug(XdgDesktopPortalKdeInputCapture) << "Sending  zones changed" << session->handle();
        ++m_zoneId;
        Q_EMIT ZonesChanged(QDBusObjectPath(session->handle()), {{u"zone_set"_s, m_zoneId}});
    };

    results.insert("zone_set", m_zoneId);
    QList<zone> zones;
    for (const auto screen : qGuiApp->screens()) {
        zones.push_back(zone{
            .width = static_cast<uint>(screen->geometry().width()),
            .height = static_cast<uint>(screen->geometry().height()),
            .x_offset = screen->geometry().x(),
            .y_offset = screen->geometry().y(),
        });
        connect(screen, &QScreen::geometryChanged, this, handleZoneChange);
    }

    connect(qGuiApp, &QGuiApplication::screenAdded, session, handleZoneChange);
    connect(qGuiApp, &QGuiApplication::screenRemoved, session, handleZoneChange);

    results.insert("zones", QVariant::fromValue(zones));
    return 0;
}

uint InputCapturePortal::SetPointerBarriers(const QDBusObjectPath &handle,
                                            const QDBusObjectPath &session_handle,
                                            const QString &app_id,
                                            const QVariantMap &options,
                                            const QList<QVariantMap> &barriers,
                                            uint zone_set,
                                            QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeInputCapture) << "SetPointerBarriers called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    zone_set: " << zone_set;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    barriers: ";

    auto *session = qobject_cast<InputCaptureSession *>(Session::getSession(session_handle.path()));

    if (!session) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to set barriers non-existing session " << session_handle.path();
        return 2;
    }

    if (zone_set != m_zoneId) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Invalid zone_set " << session_handle.path();
        return 2;
    }

    if (session->state != State::Disabled) {
        if (auto reply = QDBusReply(session->disable()); !reply.isValid()) {
            qCWarning(XdgDesktopPortalKdeInputCapture) << "Error disabling input capture:" << reply.error();
            return 2;
        }
    }
    session->clearBarriers();

    QList<QRect> screenGeometries;
    std::ranges::transform(qGuiApp->screens(), std::back_inserter(screenGeometries), &QScreen::geometry);

    QList<uint> failedBarriers;

    for (const auto &barrier : barriers) {
        const auto id = barrier.value(u"barrier_id"_s).toUInt();
        int x1;
        int y1;
        int x2;
        int y2;
        const auto position = barrier.value(u"position"_s).value<QDBusArgument>();
        position.beginStructure();
        // (iiii)
        position >> x1 >> y1 >> x2 >> y2;
        position.endStructure();
        qCDebug(XdgDesktopPortalKdeInputCapture) << "        " << id << x1 << y1 << x2 << y2;

        if (id == 0) {
            qCWarning(XdgDesktopPortalKdeInputCapture) << "Invalid barrier id " << id;
            failedBarriers.append(id);
            continue;
        }

        const auto barrierOrFailure = checkAndMakeBarrier(x1, y1, x2, y2, screenGeometries);
        if (auto reason = std::get_if<BarrierFailureReason>(&barrierOrFailure)) {
            switch (*reason) {
            case BarrierFailureReason::Diagonal:
                qCWarning(XdgDesktopPortalKdeInputCapture) << "Disallowed Diagonal barrier " << id;
                break;
            case BarrierFailureReason::NotOnEdge:
                qCWarning(XdgDesktopPortalKdeInputCapture) << "Barrier" << id << "not on any screen edge";
                break;
            case BarrierFailureReason::BetweenScreensOrDoesNotFill:
                qCWarning(XdgDesktopPortalKdeInputCapture) << "Barrier" << id << "doesnt fill or on edge to another screen";
                break;
            }
            failedBarriers.append(id);
        } else {
            session->addBarrier(std::get<1>(barrierOrFailure));
        }
    }
    results.insert(u"failed_barriers"_s, QVariant::fromValue(failedBarriers));
    return 0;
}

QDBusUnixFileDescriptor
InputCapturePortal::ConnectToEIS(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, const QDBusMessage &message)
{
    qCDebug(XdgDesktopPortalKdeInputCapture) << "ConnectToEIS called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;

    auto *session = qobject_cast<InputCaptureSession *>(Session::getSession(session_handle.path()));
    if (!session) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to call ConnectToEis on non-existing session " << session_handle.path();
        return QDBusUnixFileDescriptor();
    }

    if (session->state != State::Disabled) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to call ConnectToEis on enabled session " << session_handle.path();
        auto error = message.createErrorReply(QDBusError::Failed, u"Session is enabled"_s);
        QDBusConnection::sessionBus().send(error);
        return QDBusUnixFileDescriptor();
    }

    QDBusReply<QDBusUnixFileDescriptor> reply = session->connectToEIS();
    if (!reply.isValid()) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Failed to connect to eis" << reply.error();
        auto error = message.createErrorReply(QDBusError::Failed, u"Failed to connect to eis"_s);
        QDBusConnection::sessionBus().send(error);
        return QDBusUnixFileDescriptor();
    }

    return reply.value();
}

uint InputCapturePortal::Enable(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results)
{
    Q_UNUSED(results);
    qCDebug(XdgDesktopPortalKdeInputCapture) << "Enable called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;

    auto *session = qobject_cast<InputCaptureSession *>(Session::getSession(session_handle.path()));
    if (!session) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to call Enable on non-existing session " << session_handle.path();
        return 2;
    }

    if (session->state != State::Disabled) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Session is already enabled" << session_handle.path();
        return 2;
    }

    QDBusReply reply = session->enable();
    if (!reply.isValid()) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Failed to enable session" << reply.error();
        return 2;
    }

    session->state = State::Deactivated;
    return 0;
}

uint InputCapturePortal::Disable(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results)
{
    Q_UNUSED(results);
    qCDebug(XdgDesktopPortalKdeInputCapture) << "Disable called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;

    auto *session = qobject_cast<InputCaptureSession *>(Session::getSession(session_handle.path()));
    if (!session) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to call Enable on non-existing session " << session_handle.path();
        return 2;
    }

    if (session->state == State::Disabled) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Session is not enabled" << session_handle.path();
        return 2;
    }

    QDBusReply reply = session->enable();
    if (!reply.isValid()) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Failed to disable session" << reply.error();
        return 2;
    }

    return 0;
}

uint InputCapturePortal::Release(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results)
{
    Q_UNUSED(results);
    qCDebug(XdgDesktopPortalKdeInputCapture) << "Release called with parameters:";
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    session_handle: " << session_handle.path();
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInputCapture) << "    options: " << options;

    auto *session = qobject_cast<InputCaptureSession *>(Session::getSession(session_handle.path()));
    if (!session) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Tried to call Enable on non-existing session " << session_handle.path();
        return 2;
    }

    if (session->state != State::Activated) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Session is not activated" << session_handle.path();
        return 2;
    }

    auto it = options.find("cursor_position");
    bool positionSpecified = it != options.end();
    QPointF cursorPosition = positionSpecified ? qdbus_cast<QPointF>(it->value<QDBusArgument>()) : QPointF(); // (dd)

    if (positionSpecified) {
        qCDebug(XdgDesktopPortalKdeInputCapture) << "cursor_position" << cursorPosition;
    } else {
        qCDebug(XdgDesktopPortalKdeInputCapture) << "no cursor position hinted";
    }

    QDBusReply reply = session->release(cursorPosition, positionSpecified);
    if (!reply.isValid()) {
        qCWarning(XdgDesktopPortalKdeInputCapture) << "Failed to release session" << reply.error();
        return 2;
    }

    return 0;
}
