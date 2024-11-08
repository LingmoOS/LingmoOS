#ifndef DBUSHELPER_H
#define DBUSHELPER_H

#include <QtDBus/QtDBus>

const auto dcall=[](QDBusMessage msg){
    auto resp=QDBusConnection::sessionBus().call(msg);
    qDebug()<<"dcall::("<<msg<<")"<<resp;
    return resp.type()==QDBusMessage::ReplyMessage?resp.arguments():QList<QVariant>({QVariant()});
};


#endif // DBUSHELPER_H
