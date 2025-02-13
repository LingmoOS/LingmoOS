#include "networkproxy.h"
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QTimer>

NetworkProxy::NetworkProxy(QObject *parent)
    : QObject(parent)
    , m_settings("lingmoos", "network")
{
    m_flag = m_settings.value("ProxyFlag", 0).toInt();
    m_useSameProxy = m_settings.value("UseSameProxy", false).toBool();
    m_scriptProxy = m_settings.value("ProxyScriptProxy", "").toString();
    m_httpProxy = m_settings.value("HttpProxy", "").toString();
    m_ftpProxy = m_settings.value("FtpProxy", "").toString();
    m_socksProxy = m_settings.value("SocksProxy", "").toString();
    m_httpProxyPort = m_settings.value("HttpProxyPort", "").toString();
    m_ftpProxyPort = m_settings.value("FtpProxyPort", "").toString();
    m_socksProxyPort = m_settings.value("SocksProxyPort", "").toString();
}

int NetworkProxy::flag() const
{
    return m_flag;
}

void NetworkProxy::setFlag(int flag)
{
    if (m_flag != flag) {
        m_flag = flag;
        m_settings.setValue("ProxyFlag", flag);
        delayUpdateProxy();
        emit flagChanged();
    }
}

bool NetworkProxy::useSameProxy() const
{
    return m_useSameProxy;
}

void NetworkProxy::setUseSameProxy(bool enabled)
{
    if (m_useSameProxy != enabled) {
        m_useSameProxy = enabled;
        m_settings.setValue("UseSameProxy", enabled);
        delayUpdateProxy();
        emit useSameProxyChanged();
    }
}

QString NetworkProxy::scriptProxy() const
{
    return m_scriptProxy;
}

void NetworkProxy::setScriptProxy(const QString &path)
{
    if (m_scriptProxy != path) {
        m_scriptProxy = path;
        m_settings.setValue("ProxyScriptProxy", path);
        delayUpdateProxy();
        emit scriptProxyChanged();
    }
}

QString NetworkProxy::httpProxy() const
{
    return m_httpProxy;
}

void NetworkProxy::setHttpProxy(const QString &path)
{
    if (m_httpProxy != path) {
        m_httpProxy = path;
        m_settings.setValue("HttpProxy", path);
        delayUpdateProxy();
        emit httpProxyChanged();
    }
}

QString NetworkProxy::ftpProxy() const
{
    return m_ftpProxy;
}

void NetworkProxy::setFtpProxy(const QString &path)
{
    if (m_ftpProxy != path) {
        m_ftpProxy = path;
        m_settings.setValue("FtpProxy", path);
        delayUpdateProxy();
        emit ftpProxyChanged();
    }
}

QString NetworkProxy::socksProxy() const
{
    return m_socksProxy;
}

void NetworkProxy::setSocksProxy(const QString &path)
{
    if (m_socksProxy != path) {
        m_socksProxy = path;
        m_settings.setValue("SocksProxy", path);
        delayUpdateProxy();
        emit socksProxyChanged();
    }
}

QString NetworkProxy::httpProxyPort() const
{
    return m_httpProxyPort;
}

void NetworkProxy::setHttpProxyPort(const QString &port)
{
    if (m_httpProxyPort != port) {
        m_httpProxyPort = port;
        m_settings.setValue("HttpProxyPort", port);
        delayUpdateProxy();
        emit httpProxyPortChanged();
    }
}

QString NetworkProxy::ftpProxyPort() const
{
    return m_ftpProxyPort;
}

void NetworkProxy::setFtpProxyPort(const QString &port)
{
    if (m_ftpProxyPort != port) {
        m_ftpProxyPort = port;
        m_settings.setValue("FtpProxyPort", port);
        delayUpdateProxy();
        emit ftpProxyPortChanged();
    }
}

QString NetworkProxy::socksProxyPort() const
{
    return m_socksProxyPort;
}

void NetworkProxy::setSocksProxyPort(const QString &port)
{
    if (m_socksProxyPort != port) {
        m_socksProxyPort = port;
        m_settings.setValue("SocksProxyPort", port);
        delayUpdateProxy();
        emit socksProxyPortChanged();
    }
}

void NetworkProxy::updateProxy()
{
    QDBusInterface iface("com.lingmo.Session", "/Session", "com.lingmo.Session");

    if (iface.isValid()) {
        iface.asyncCall("updateNetworkProxy");
    }
}

void NetworkProxy::delayUpdateProxy()
{
    QTimer::singleShot(50, this, &NetworkProxy::updateProxy);
}
