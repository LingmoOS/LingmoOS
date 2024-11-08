#include "mouse.h"
#include <QDBusPendingCall>

Mouse::Mouse(QObject *parent)
    : QObject(parent)
    , m_interface("com.lingmo.Settings",
                  "/Mouse",
                  "com.lingmo.Mouse",
                  QDBusConnection::sessionBus())
{
    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(leftHandedChanged()), this, SIGNAL(leftHandedChanged()));
        connect(&m_interface, SIGNAL(naturalScrollChanged()), this, SIGNAL(naturalScrollChanged()));
        connect(&m_interface, SIGNAL(pointerAccelerationChanged()), this, SIGNAL(pointerAccelerationChanged()));
    }
}

Mouse::~Mouse()
{
}

bool Mouse::leftHanded() const
{
    return m_interface.property("leftHanded").toBool();
}

void Mouse::setLeftHanded(bool enabled)
{
    m_interface.asyncCall("setLeftHanded", enabled);
}

bool Mouse::acceleration() const
{
    return m_interface.property("acceleration").toBool();
}

void Mouse::setAcceleration(bool enabled)
{
    m_interface.asyncCall("setPointerAccelerationProfileFlat", enabled);
}

bool Mouse::naturalScroll() const
{
    return m_interface.property("naturalScroll").toBool();
}

void Mouse::setNaturalScroll(bool enabled)
{
    m_interface.asyncCall("setNaturalScroll", enabled);
}

qreal Mouse::pointerAcceleration() const
{
    return m_interface.property("pointerAcceleration").toReal();
}

void Mouse::setPointerAcceleration(qreal value)
{
    m_interface.asyncCall("setPointerAcceleration", value);
}
