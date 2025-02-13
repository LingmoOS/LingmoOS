#ifndef NETWORKPROXY_H
#define NETWORKPROXY_H

#include <QObject>
#include <QSettings>

class NetworkProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int flag READ flag WRITE setFlag NOTIFY flagChanged)
    Q_PROPERTY(bool useSameProxy READ useSameProxy WRITE setUseSameProxy NOTIFY useSameProxyChanged)
    Q_PROPERTY(QString scriptProxy READ scriptProxy WRITE setScriptProxy NOTIFY scriptProxyChanged)
    Q_PROPERTY(QString httpProxy READ httpProxy WRITE setHttpProxy NOTIFY httpProxyChanged)
    Q_PROPERTY(QString ftpProxy READ ftpProxy WRITE setFtpProxy NOTIFY ftpProxyChanged)
    Q_PROPERTY(QString socksProxy READ socksProxy WRITE setSocksProxy NOTIFY socksProxyChanged)
    Q_PROPERTY(QString httpProxyPort READ httpProxyPort WRITE setHttpProxyPort NOTIFY httpProxyPortChanged)
    Q_PROPERTY(QString ftpProxyPort READ ftpProxyPort WRITE setFtpProxyPort NOTIFY ftpProxyPortChanged)
    Q_PROPERTY(QString socksProxyPort READ socksProxyPort WRITE setSocksProxyPort NOTIFY socksProxyPortChanged)

public:
    explicit NetworkProxy(QObject *parent = nullptr);

    int flag() const;
    void setFlag(int flag);

    bool useSameProxy() const;
    void setUseSameProxy(bool enabled);

    QString scriptProxy() const;
    void setScriptProxy(const QString &path);

    QString httpProxy() const;
    void setHttpProxy(const QString &path);

    QString ftpProxy() const;
    void setFtpProxy(const QString &path);

    QString socksProxy() const;
    void setSocksProxy(const QString &path);

    QString httpProxyPort() const;
    void setHttpProxyPort(const QString &port);

    QString ftpProxyPort() const;
    void setFtpProxyPort(const QString &port);

    QString socksProxyPort() const;
    void setSocksProxyPort(const QString &port);

    Q_INVOKABLE void updateProxy();
    Q_INVOKABLE void delayUpdateProxy();

signals:
    void flagChanged();
    void useSameProxyChanged();
    void scriptProxyChanged();
    void httpProxyChanged();
    void ftpProxyChanged();
    void socksProxyChanged();
    void httpProxyPortChanged();
    void ftpProxyPortChanged();
    void socksProxyPortChanged();

private:
    QSettings m_settings;
    int m_flag;
    bool m_useSameProxy;

    QString m_scriptProxy;
    QString m_httpProxy;
    QString m_ftpProxy;
    QString m_socksProxy;

    QString m_httpProxyPort;
    QString m_ftpProxyPort;
    QString m_socksProxyPort;
};

#endif // NETWORKPROXY_H
