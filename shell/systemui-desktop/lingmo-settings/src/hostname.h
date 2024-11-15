#ifndef HOSTNAMECHANGER_H
#define HOSTNAMECHANGER_H

#include <QObject>
#include <QString>

class HostNameChanger : public QObject {
    Q_OBJECT

public:
    explicit HostNameChanger(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void onHostNameChanged(const QString &newHostName);
};

#endif // HOSTNAMECHANGER_H