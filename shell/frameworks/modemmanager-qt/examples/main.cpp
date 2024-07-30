/*
  SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <ModemManagerQt/Interface>
#include <ModemManagerQt/Manager>
#include <ModemManagerQt/Modem>
#include <ModemManagerQt/ModemDevice>
#include <ModemManagerQt/ModemMessaging>
#include <ModemManagerQt/Sim>
#include <ModemManagerQt/Sms>

#include <QDBusReply>

#include <QDebug>

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    qDebug() << "List of Modem Devices:";
    Q_FOREACH (ModemManager::ModemDevice::Ptr modemdevice, ModemManager::modemDevices()) {
        ModemManager::Modem::Ptr modemInterface = modemdevice->modemInterface();
        if (modemInterface) {
            qDebug() << "Modem: " << modemInterface->manufacturer() << modemInterface->model();
        } else {
            qDebug() << "Modem: " << modemdevice->uni();
        }

        QStringList interfaces;
        if (modemInterface) {
            interfaces << QStringLiteral("modem");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::GsmInterface)) {
            interfaces << QStringLiteral("gsm");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::GsmUssdInterface)) {
            interfaces << QStringLiteral("gsm ussd");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::CdmaInterface)) {
            interfaces << QStringLiteral("cdma");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::MessagingInterface)) {
            interfaces << QStringLiteral("messaging");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::LocationInterface)) {
            interfaces << QStringLiteral("location");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::TimeInterface)) {
            interfaces << QStringLiteral("time");
        }
        if (modemdevice->hasInterface(ModemManager::ModemDevice::VoiceInterface)) {
            interfaces << QStringLiteral("voice");
        }

        qDebug() << "Interfaces: " << interfaces;
        qDebug() << "SIM Card: ";
        if (!modemdevice->sim()) {
            qDebug() << "   no SIM card";
        } else {
            qDebug() << "   " << modemdevice->sim()->uni() << "Operator: " << modemdevice->sim()->operatorName();
        }

        qDebug() << "Messages: ";
        ModemManager::ModemMessaging::Ptr messaging = modemdevice->messagingInterface();
        if (messaging && !messaging->messages().isEmpty()) {
            ModemManager::Sms::List messages = messaging->messages();
            Q_FOREACH (ModemManager::Sms::Ptr sms, messages) {
                qDebug() << "   " << sms->number() << sms->text();
            }
        } else {
            qDebug() << "   no messaging interface or message";
        }

        if (messaging) {
            ModemManager::ModemMessaging::Message msg;
            msg.number = QStringLiteral("number");
            msg.text = QStringLiteral("message text");
            QDBusReply<QString> sms = messaging->createMessage(msg);
            if (sms.isValid() && !sms.value().isEmpty()) {
                qDebug() << "Message created";
                ModemManager::Sms::Ptr tmp = messaging->findMessage(sms.value());
                if (tmp) {
                    qDebug() << "Message text:";
                    qDebug() << tmp->text();
                    QDBusPendingReply<> reply = tmp->send();
                    reply.waitForFinished();
                    if (reply.isError()) {
                        qDebug() << reply.error().message();
                    }
                }
            }
        }
    }
}
