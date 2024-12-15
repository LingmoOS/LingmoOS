// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuiscreen.h"

#include "private/oceanuiscreen_p.h"
#include "private/displayworker.h"

#include <QGuiApplication>

namespace oceanuiV25 {
OceanUIScreen *OceanUIScreenPrivate::New(QList<Monitor *> monitors, DisplayWorker *worker, QObject *parent)
{
    OceanUIScreen *screen = new OceanUIScreen(parent);
    screen->d_ptrOceanUIScreen->m_worker = worker;
    screen->d_ptrOceanUIScreen->setMonitors(monitors);
    return screen;
}

OceanUIScreenPrivate *OceanUIScreenPrivate::Private(OceanUIScreen *screen)
{
    return screen->d_ptrOceanUIScreen.get();
}

OceanUIScreenPrivate::OceanUIScreenPrivate(OceanUIScreen *screen)
    : q_ptr(screen)
    , m_screen(nullptr)
{
}

OceanUIScreenPrivate::~OceanUIScreenPrivate() { }

void OceanUIScreenPrivate::setMonitors(QList<Monitor *> monitors)
{
    m_monitors = monitors;
    std::sort(m_monitors.begin(), m_monitors.end(), [](const Monitor *monitor1, const Monitor *monitor2) {
        return monitor1->name() < monitor2->name();
    });
    QStringList name;
    for (auto monitor : m_monitors) {
        name << monitor->name();
    }
    m_name = name.join(" = ");
    updateResolutionList();
    updateRateList();
    updateScreen();
    q_ptr->connect(monitor(), &Monitor::modelListChanged, q_ptr, [this]() {
        updateResolutionList();
        updateRateList();
    });
    q_ptr->connect(monitor(), &Monitor::currentModeChanged, q_ptr, [this]() {
        updateResolutionList();
        updateRateList();
        q_ptr->currentResolutionChanged();
        q_ptr->currentRateChanged();
    });
    q_ptr->connect(monitor(), &Monitor::availableFillModesChanged, q_ptr, &OceanUIScreen::availableFillModesChanged);
    q_ptr->connect(monitor(), &Monitor::currentFillModeChanged, q_ptr, &OceanUIScreen::currentFillModeChanged);
    q_ptr->connect(monitor(), &Monitor::enableChanged, q_ptr, &OceanUIScreen::enableChanged);
    auto updateScreenFun = [this]() {
        updateScreen();
    };
    q_ptr->connect(qApp, &QGuiApplication::screenAoceand, q_ptr, updateScreenFun);
    q_ptr->connect(qApp, &QGuiApplication::screenRemoved, q_ptr, updateScreenFun);
}

Monitor *OceanUIScreenPrivate::monitor()
{
    return m_monitors.first();
}

QList<Monitor *> OceanUIScreenPrivate::monitors()
{
    return m_monitors;
}

void OceanUIScreenPrivate::setMode(QSize resolution, double rate)
{
    m_worker->backupConfig();
    for (auto monitor : m_monitors) {
        quint32 id = 0;
        for (auto mode : monitor->modeList()) {
            if (mode.width() == resolution.width() && mode.height() == resolution.height()) {
                if (mode.rate() == rate) {
                    id = mode.id();
                    break;
                } else if (id == 0) {
                    id = mode.id();
                }
            }
        }
        if (id != 0) {
            m_worker->setMonitorResolution(monitor, id);
        } else {
            m_worker->setMonitorResolutionBySize(monitor, resolution.width(), resolution.height());
        }
    }
    m_worker->applyChanges();
}

void OceanUIScreenPrivate::setRotate(uint rotate)
{
    m_worker->backupConfig();
    m_worker->setMonitorRotate(monitor(), rotate);
    m_worker->applyChanges();
}

void OceanUIScreenPrivate::setFillMode(const QString &fileMode)
{
    m_worker->backupConfig();
    for (auto monitor : m_monitors) {
        m_worker->setCurrentFillMode(monitor, fileMode);
    }
}

void OceanUIScreenPrivate::updateResolutionList()
{
    QList<QSize> resolutionList;
    for (auto monitor = m_monitors.cbegin(); monitor != m_monitors.cend(); monitor++) {
        QList<QSize> tmpResolutionList;
        for (auto mode : (*monitor)->modeList()) {
            QSize tmpMode(mode.width(), mode.height());
            if (!tmpResolutionList.contains(tmpMode)) {
                tmpResolutionList.append(tmpMode);
            }
        }
        if (monitor == m_monitors.cbegin()) {
            resolutionList = tmpResolutionList;
        } else {
            for (auto it = resolutionList.begin(); it != resolutionList.end();) {
                if (tmpResolutionList.contains(*it)) {
                    it++;
                } else {
                    it = resolutionList.erase(it);
                }
            }
        }
    }
    m_resolutionList = resolutionList;
    if (m_resolutionList != resolutionList) {
        m_resolutionList = resolutionList;
        Q_EMIT q_ptr->resolutionListChanged();
    }
}

void OceanUIScreenPrivate::updateRateList()
{
    QList<double> rateList;
    Resolution currentMode = monitor()->currentMode();
    for (auto mode : monitor()->modeList()) {
        if (mode.width() == currentMode.width() && mode.height() == currentMode.height()) {
            rateList.append(mode.rate());
        }
    }
    if (m_rateList != rateList) {
        m_rateList = rateList;
        Q_EMIT q_ptr->rateListChanged();
    }
}

void OceanUIScreenPrivate::updateScreen()
{
    QString name = monitor()->name();
    QRect rect = monitor()->rect();
    QScreen *tmpScreen = nullptr;
    for (auto screen : QGuiApplication::screens()) {
        if (rect == screen->geometry()) {
            if (name == screen->name()) {
                tmpScreen = screen;
            }
            if (!tmpScreen) {
                tmpScreen = screen;
            }
        }
    }
    if (tmpScreen != m_screen) {
        m_screen = tmpScreen;
        Q_EMIT q_ptr->screenChanged();
    }
}

OceanUIScreen::OceanUIScreen(QObject *parent)
    : QObject(parent)
    , d_ptrOceanUIScreen(new OceanUIScreenPrivate(this))
{
}

OceanUIScreen::~OceanUIScreen() { }

QString OceanUIScreen::name() const
{
    return d_ptrOceanUIScreen->m_name;
}

bool OceanUIScreen::enable() const
{
    return d_ptrOceanUIScreen->monitor()->enable();
}

int OceanUIScreen::x() const
{
    return d_ptrOceanUIScreen->monitor()->x();
}

int OceanUIScreen::y() const
{
    return d_ptrOceanUIScreen->monitor()->y();
}

QSize OceanUIScreen::bestResolution() const
{
    Resolution resolution = d_ptrOceanUIScreen->monitor()->bestMode();
    return QSize(resolution.width(), resolution.height());
}

QSize OceanUIScreen::currentResolution() const
{
    auto monitor = d_ptrOceanUIScreen->monitor();
    return QSize(monitor->w(), monitor->h());
}

void OceanUIScreen::setCurrentResolution(const QSize &resolution)
{
    d_ptrOceanUIScreen->setMode(resolution, currentRate());
}

double OceanUIScreen::bestRate() const
{
    return d_ptrOceanUIScreen->monitor()->bestMode().rate();
}

double OceanUIScreen::currentRate() const
{
    return d_ptrOceanUIScreen->monitor()->currentMode().rate();
}

void OceanUIScreen::setCurrentRate(const double &rate)
{
    d_ptrOceanUIScreen->setMode(currentResolution(), rate);
}

QList<QSize> OceanUIScreen::resolutionList() const
{
    return d_ptrOceanUIScreen->m_resolutionList;
}

QList<double> OceanUIScreen::rateList() const
{
    return d_ptrOceanUIScreen->m_rateList;
}

uint OceanUIScreen::rotate() const
{
    return d_ptrOceanUIScreen->monitor()->rotate();
}

void OceanUIScreen::setRotate(uint rotate)
{
    d_ptrOceanUIScreen->setRotate(rotate);
}

double OceanUIScreen::brightness() const
{
    return d_ptrOceanUIScreen->monitor()->brightness();
}

QString OceanUIScreen::currentFillMode() const
{
    QString fillMode = d_ptrOceanUIScreen->monitor()->currentFillMode();
    return fillMode.isEmpty() ? "None" : fillMode;
}

void OceanUIScreen::setCurrentFillMode(const QString &fill)
{
    d_ptrOceanUIScreen->setFillMode(fill);
}

QStringList OceanUIScreen::availableFillModes() const
{
    return d_ptrOceanUIScreen->monitor()->availableFillModes();
}

QScreen *OceanUIScreen::screen() const
{
    return d_ptrOceanUIScreen->m_screen;
}

} // namespace oceanuiV25
