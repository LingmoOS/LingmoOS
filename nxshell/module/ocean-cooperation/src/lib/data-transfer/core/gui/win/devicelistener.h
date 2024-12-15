#ifndef DEVICELISTENER_H
#define DEVICELISTENER_H

#include <QWidget>

class QVariant;
class QStorageInfo;
class DeviceListener : public QWidget
{
    Q_OBJECT
public:
    DeviceListener(QWidget *parent = nullptr);
    static DeviceListener *instance();
    ~DeviceListener();
signals:
    void updateDevice();
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    bool enroll{ false };
    QList<QStorageInfo> deviceList;
};

#endif // DEVICELISTENER_H
