#include "cscreenoutput.h"

CScreenOutput::CScreenOutput(QObject *parent)
    : QObject(parent)
{

}

int CScreenOutput::id() const
{
    return m_id;
}

void CScreenOutput::setId(int id)
{
    if (id != m_id) {
        m_id = id;
        emit outputChanged();
    }
}

QString CScreenOutput::name() const
{
    return m_name;
}

void CScreenOutput::setName(QString name)
{
    if (name != m_name) {
        m_name = name;
        emit outputChanged();
    }
}

QString CScreenOutput::hash() const
{
    return QString();
}

CScreenOutput::Rotation CScreenOutput::rotation() const
{
    return m_rotation;
}

void CScreenOutput::setRotation(const Rotation &rotation)
{
    if (rotation != m_rotation) {
        m_rotation = rotation;
        emit rotationChanged();
    }
}

bool CScreenOutput::connected() const
{
    return m_connected;
}

void CScreenOutput::setConnected(bool connected)
{
    if (connected != m_connected) {
        m_connected = connected;
        emit connectedChanged();
    }
}

bool CScreenOutput::primary() const
{
    return m_primary;
}

void CScreenOutput::setPrimary(bool primary)
{
    if (primary != m_primary) {
        m_primary = primary;
        emit primaryChanged();
    }
}

bool CScreenOutput::enabled() const
{
    return m_enabled;
}

void CScreenOutput::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        emit enabledChanged();
    }
}

const QScreen *CScreenOutput::qscreen() const
{
    return m_qscreen;
}
