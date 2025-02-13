#ifndef VPN_H
#define VPN_H

#include <QObject>

class VPN : public QObject
{
    Q_OBJECT

public:
    explicit VPN(QObject *parent = nullptr);

    Q_INVOKABLE void activateVPNConnection(const QString &connection, const QString &device, const QString &specificParameter);
};

#endif // VPN_H
