#include "accessibility.h"
#include <QSettings>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDebug>

Accessibility::Accessibility(QObject *parent)
    : QObject(parent)
    , m_kwinSettings(new QSettings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kwinrc",
                               QSettings::IniFormat))
    , m_wobblyWindows(false)
    , m_thumbnailaside(false)
    , m_touchpoints(false)
    , m_snaphelper(false)
    , m_diminactive(false)
{
    m_kwinSettings->beginGroup("Plugins");
    m_wobblyWindows = m_kwinSettings->value("wobblywindowsEnabled").toBool();
    m_thumbnailaside = m_kwinSettings->value("thumbnailasideEnabled").toBool();
    m_touchpoints = m_kwinSettings->value("touchpointsEnabled").toBool();
    m_snaphelper = m_kwinSettings->value("snaphelperEnabled").toBool();
    m_diminactive = m_kwinSettings->value("diminactiveEnabled").toBool();
    m_kwinSettings->endGroup();
}

bool Accessibility::wobblyWindows() const
{
    return m_wobblyWindows;
}

void Accessibility::setWobblywindows(bool enabled)
{
    if (m_wobblyWindows != enabled) {
        m_wobblyWindows = enabled;
        m_kwinSettings->beginGroup("Plugins");
        m_kwinSettings->setValue("wobblywindowsEnabled", enabled);
        m_kwinSettings->endGroup();
        m_kwinSettings->sync();
        QDBusInterface("org.kde.KWin", "/KWin").call("reconfigure");
        emit wobblyWindowsChanged();
    }
}

bool Accessibility::thumbnailaside() const
{
    return m_thumbnailaside;
}

void Accessibility::setThumbnailaside(bool enabled)
{
    if (m_thumbnailaside != enabled) {
        m_thumbnailaside = enabled;
        m_kwinSettings->beginGroup("Plugins");
        m_kwinSettings->setValue("thumbnailasideEnabled", enabled);
        m_kwinSettings->endGroup();
        m_kwinSettings->sync();
        QDBusInterface("org.kde.KWin", "/KWin").call("reconfigure");
        emit thumbnailasideChanged();
    }
}

bool Accessibility::touchpoints() const
{
    return m_touchpoints;
}

void Accessibility::setTouchpoints(bool enabled)
{
    if (m_touchpoints != enabled) {
        m_touchpoints = enabled;
        m_kwinSettings->beginGroup("Plugins");
        m_kwinSettings->setValue("touchpointsEnabled", enabled);
        m_kwinSettings->endGroup();
        m_kwinSettings->sync();
        QDBusInterface("org.kde.KWin", "/KWin").call("reconfigure");
        emit touchpointsChanged();
    }
}

bool Accessibility::snaphelper() const
{
    return m_snaphelper;
}

void Accessibility::setSnaphelper(bool enabled)
{
    if (m_snaphelper != enabled) {
        m_snaphelper = enabled;
        m_kwinSettings->beginGroup("Plugins");
        m_kwinSettings->setValue("snaphelperEnabled", enabled);
        m_kwinSettings->endGroup();
        m_kwinSettings->sync();
        QDBusInterface("org.kde.KWin", "/KWin").call("reconfigure");
        emit snaphelperChanged();
    }
}

bool Accessibility::diminactive() const
{
    return m_diminactive;
}

void Accessibility::setDiminactive(bool enabled)
{
    if (m_diminactive != enabled) {
        m_diminactive = enabled;
        m_kwinSettings->beginGroup("Plugins");
        m_kwinSettings->setValue("diminactiveEnabled", enabled);
        m_kwinSettings->endGroup();
        m_kwinSettings->beginGroup("Effect-DimInactive");
        m_kwinSettings->setValue("DimDesktop", true);
        m_kwinSettings->setValue("Strength", 40);
        m_kwinSettings->endGroup();
        m_kwinSettings->sync();
        QDBusInterface("org.kde.KWin", "/KWin").call("reconfigure");
        emit diminactiveChanged();
    }
}
