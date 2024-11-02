//
// Created by zpf on 2023/6/12.
//

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include "chat.h"
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    auto chat = new Chat;
    QObject::connect(chat, &Chat::readyToChat, [chat](){
        qDebug() << chat->isValid();
        if(chat->isValid()) {
            chat->talk("hello");
        }
    });
    QObject::connect(chat, &Chat::response, [&](const QString &content){
        qDebug() << content;
    });
    chat->newChat();
    return QCoreApplication::exec();
}