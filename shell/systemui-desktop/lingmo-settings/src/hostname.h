#include <QObject>

class HostNameChanger : public
QObject {
    Q_OBJECT
public:
    HostNameChanger(QObject *parent = nullptr) : QObject(parent) {}

};
