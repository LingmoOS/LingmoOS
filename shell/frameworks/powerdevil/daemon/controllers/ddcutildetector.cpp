/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2017 Dorian Vogel <dorianvogel@gmail.com>
 *    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#include "ddcutildetector.h"

#include <powerdevil_debug.h>

#include <QMutex>

#include <map>
#include <memory> // std::unique_ptr
#include <span>

#ifdef WITH_DDCUTIL
#include <ddcutil_c_api.h>

#include "ddcutildisplay.h"

#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
void ddcaCallback(DDCA_Display_Status_Event event);
#endif

class DDCutilPrivateSingleton : public QObject
{
    Q_OBJECT

#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    friend void ddcaCallback(DDCA_Display_Status_Event event);
#endif
    friend class DDCutilDetector;

public:
    static DDCutilPrivateSingleton &instance();

private:
    explicit DDCutilPrivateSingleton();
    ~DDCutilPrivateSingleton();

    void detect();
    const std::map<QString, std::unique_ptr<DDCutilDisplay>> &displays();

#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    void displayStatusChanged(DDCA_Display_Status_Event &event);
#endif

Q_SIGNALS:
    void displaysChanged();

    // emitted from the ddcutil callback from potentially outside the object's thread
    void displayAdded();
    void displayRemoved(const QString &id);
    void dpmsStateChanged(const QString &id, bool isSleeping);

private Q_SLOTS:
    void redetect();
    void removeDisplay(const QString &id);
    void setDpmsState(const QString &id, bool isSleeping);

private:
    std::map<QString, std::unique_ptr<DDCutilDisplay>> m_displays;
    // ddcutil has global state, let's avoid simultaneous access to its open display map
    QMutex m_openDisplayMutex;
    bool m_performedDetection = false;
    bool m_noDdcutil = false;
};

#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
void ddcaCallback(DDCA_Display_Status_Event event)
{
    DDCutilPrivateSingleton::instance().displayStatusChanged(event);
}
#endif

DDCutilPrivateSingleton &DDCutilPrivateSingleton::instance()
{
    static DDCutilPrivateSingleton singleton;
    return singleton;
}

DDCutilPrivateSingleton::DDCutilPrivateSingleton()
{
    m_noDdcutil = qEnvironmentVariableIntValue("POWERDEVIL_NO_DDCUTIL") > 0;
    if (m_noDdcutil) {
        return;
    }
#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 0, 0)
    qCDebug(POWERDEVIL) << "[DDCutilDetector]: Initializing ddcutil API (create ddcutil configuration file for tracing & more)...";
#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    DDCA_Status init_status = ddca_init2(nullptr, DDCA_SYSLOG_NOTICE, DDCA_INIT_OPTIONS_CLIENT_OPENED_SYSLOG, nullptr);
#else
    DDCA_Status init_status = ddca_init(nullptr, DDCA_SYSLOG_NOTICE, DDCA_INIT_OPTIONS_CLIENT_OPENED_SYSLOG);
#endif

    if (init_status < 0) {
        qCWarning(POWERDEVIL) << "[DDCutilDetector]: Could not initialize ddcutil API. Not using DDC for monitor brightness.";
        return;
    }
#endif
#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    if (ddca_register_display_status_callback(ddcaCallback)) {
        qCWarning(POWERDEVIL) << "[DDCutilDetector]: Failed to initialize callback";
        return;
    }

    connect(this, &DDCutilPrivateSingleton::displayAdded, this, &DDCutilPrivateSingleton::redetect);
    connect(this, &DDCutilPrivateSingleton::displayRemoved, this, &DDCutilPrivateSingleton::removeDisplay);
    connect(this, &DDCutilPrivateSingleton::dpmsStateChanged, this, &DDCutilPrivateSingleton::setDpmsState);

    ddca_start_watch_displays(DDCA_Display_Event_Class(DDCA_EVENT_CLASS_ALL));
#endif
}

DDCutilPrivateSingleton::~DDCutilPrivateSingleton()
{
    if (m_noDdcutil) {
        return;
    }
#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    ddca_stop_watch_displays(false);
    ddca_unregister_display_status_callback(ddcaCallback);
    disconnect(this, &DDCutilPrivateSingleton::displayAdded, this, &DDCutilPrivateSingleton::redetect);
    disconnect(this, &DDCutilPrivateSingleton::displayRemoved, this, &DDCutilPrivateSingleton::removeDisplay);
    disconnect(this, &DDCutilPrivateSingleton::dpmsStateChanged, this, &DDCutilPrivateSingleton::setDpmsState);
#endif
}

void DDCutilPrivateSingleton::detect()
{
    if (m_performedDetection || m_noDdcutil) {
        return;
    }
    m_performedDetection = true;

    qCDebug(POWERDEVIL) << "[DDCutilDetector]: Check for monitors using ddca_get_display_refs()...";
    // Inquire about detected monitors.
    DDCA_Display_Ref *displayRefs = nullptr;
    ddca_get_display_refs(false, &displayRefs);

    int displayCount = 0;
    while (displayRefs[displayCount] != nullptr) {
        ++displayCount;
    }
    qCInfo(POWERDEVIL) << "[DDCutilDetector]:" << displayCount << "display(s) were detected";

    for (int i = 0; i < displayCount; ++i) {
        auto display = std::make_unique<DDCutilDisplay>(displayRefs[i], &m_openDisplayMutex);

        QString id = DDCutilDisplay::generatePathId(display->ioPath());
        if (id.isEmpty()) {
            qCWarning(POWERDEVIL) << "[DDCutilDetector]: Cannot generate ID for display with model name:" << display->label() << "- ignoring";
            continue;
        }
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: Created ID:" << id << "for display:" << display->label();

        if (!display->supportsBrightness()) {
            qCInfo(POWERDEVIL) << "[DDCutilDetector]: Display" << display->label() << "does not seem to support brightness control - ignoring";
            continue;
        }
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: Display supports Brightness, adding handle to list";

        m_displays.emplace(std::move(id), std::move(display));
    }

    for (auto it = m_displays.cbegin(); it != m_displays.cend(); ++it) {
        DDCutilDisplay *display = it->second.get();
        connect(display, &DDCutilDisplay::supportsBrightnessChanged, this, [this, id = it->first](bool isSupported) {
            if (!isSupported) {
                removeDisplay(id);
            }
        });
    }
}

const std::map<QString, std::unique_ptr<DDCutilDisplay>> &DDCutilPrivateSingleton::displays()
{
    return m_displays;
}

void DDCutilPrivateSingleton::redetect()
{
#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    if (!m_performedDetection) {
        return;
    }
    qCDebug(POWERDEVIL) << "[DDCutilDetector]: Screen configuration changed. Redetecting displays";

    std::map<QString, std::unique_ptr<DDCutilDisplay>> invalidDisplays; // delete at end of function
    std::swap(m_displays, invalidDisplays); // clear m_displays, detect() will repopulate it

    if (ddca_redetect_displays() == DDCRC_OK) {
        m_performedDetection = false;
        detect();
    } else {
        qCCritical(POWERDEVIL) << "[DDCutilDetector]: Redetection failed";
    }

    if (!m_displays.empty() || !invalidDisplays.empty()) {
        Q_EMIT displaysChanged();
    }
#endif
}

#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
void DDCutilPrivateSingleton::displayStatusChanged(DDCA_Display_Status_Event &event)
{
    if (event.event_type == DDCA_EVENT_DISPLAY_CONNECTED) {
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: DDCA_EVENT_DISPLAY_CONNECTED signal arrived";
        Q_EMIT displayAdded();
    } else if (event.event_type == DDCA_EVENT_DISPLAY_DISCONNECTED) {
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: DDCA_EVENT_DISPLAY_DISCONNECTED signal arrived";
        Q_EMIT displayRemoved(DDCutilDisplay::generatePathId(event.io_path));
    } else if (event.event_type == DDCA_EVENT_DPMS_ASLEEP) {
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: DDCA_EVENT_DPMS_ASLEEP signal arrived";
        Q_EMIT dpmsStateChanged(DDCutilDisplay::generatePathId(event.io_path), true);
    } else if (event.event_type == DDCA_EVENT_DPMS_AWAKE) {
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: DDCA_EVENT_DPMS_AWAKE signal arrived";
        Q_EMIT dpmsStateChanged(DDCutilDisplay::generatePathId(event.io_path), false);
    }
}
#endif

void DDCutilPrivateSingleton::removeDisplay(const QString &id)
{
    if (auto deletedAfterEmit = m_displays.extract(id); !deletedAfterEmit.empty()) {
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: Removing display" << id;
        Q_EMIT displaysChanged();
    } else {
        qCDebug(POWERDEVIL) << "[DDCutilDetector]: Failed to remove display" << id;
    }
}

void DDCutilPrivateSingleton::setDpmsState(const QString &id, bool isSleeping)
{
#if DDCUTIL_VERSION >= QT_VERSION_CHECK(2, 1, 0)
    if (const auto &it = m_displays.find(id); it != m_displays.end()) {
        if (isSleeping) {
            qCDebug(POWERDEVIL) << "[DDCutilDetector]: Display" << id << "asleep - pause worker";
            it->second->pauseWorker();
        } else {
            qCDebug(POWERDEVIL) << "[DDCutilDetector]: Display" << id << "awake - resume worker";
            it->second->resumeWorker();
        }
    }
#endif
}
#endif

DDCutilDetector::DDCutilDetector(QObject *parent)
    : DisplayBrightnessDetector(parent)
{
}

DDCutilDetector::~DDCutilDetector()
{
}

void DDCutilDetector::detect()
{
#ifdef WITH_DDCUTIL
    bool isFirstDetectCall = connect(&DDCutilPrivateSingleton::instance(),
                                     &DDCutilPrivateSingleton::displaysChanged,
                                     this,
                                     &DisplayBrightnessDetector::displaysChanged,
                                     Qt::UniqueConnection);
    if (isFirstDetectCall) {
        DDCutilPrivateSingleton::instance().detect();
        if (!DDCutilPrivateSingleton::instance().displays().empty()) {
            Q_EMIT displaysChanged();
        }
    }
    Q_EMIT detectionFinished(!DDCutilPrivateSingleton::instance().displays().empty());
#else
    qCInfo(POWERDEVIL) << "[DDCutilDetector] compiled without DDC/CI support";
    Q_EMIT detectionFinished(false);
#endif
}

QList<DisplayBrightness *> DDCutilDetector::displays() const
{
    QList<DisplayBrightness *> result;
#ifdef WITH_DDCUTIL
    result.reserve(DDCutilPrivateSingleton::instance().displays().size());
    for (const auto &pair : DDCutilPrivateSingleton::instance().displays()) {
        result.append(pair.second.get());
    }
#endif
    return result;
}

#include "ddcutildetector.moc"
#include "moc_ddcutildetector.cpp"
