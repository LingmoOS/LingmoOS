/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemmessagingpropertiestest.h"

#include "dbus/fakedbus.h"
#include "generictypes.h"
#include "manager.h"
#include "modem.h"
#include "modemmessaging.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void ModemMessagingPropertiesTest::initTestCase()
{
    fakeModem = new FakeModem();

    modem = new Modem();
    modem->setAccessTechnologies(16);
    modem->SetCurrentBands({0});
    modem->SetCurrentCapabilities(4);
    modem->SetCurrentModes({MM_MODEM_MODE_ANY, MM_MODEM_MODE_NONE});
    modem->setDevice(QLatin1String("/sys/devices/pci0000:00/0000:00:1d.0/usb4/4-1/4-1.2"));
    modem->setDeviceIdentifier(QLatin1String("1c435eb6d74494b5f78d7221e2c5ae9ec526a981"));
    modem->setDrivers({QLatin1String("option1")});
    modem->setEquipmentIdentifier(QLatin1String("353475021085110"));
    modem->setManufacturer(QLatin1String("huawei"));
    modem->setMaxActiveBearers(1);
    modem->setMaxBearers(1);
    modem->setModel(QLatin1String("K2540"));
    // modem->setOwnNumbers();
    modem->setPlugin(QLatin1String("Huawei"));
    modem->setPorts({{QLatin1String("ttyUSB0"), MM_MODEM_PORT_TYPE_AT},
                     {QLatin1String("ttyUSB1"), MM_MODEM_PORT_TYPE_QCDM},
                     {QLatin1String("ttyUSB2"), MM_MODEM_PORT_TYPE_AT}});
    modem->SetPowerState(3);
    modem->setPrimaryPort(QLatin1String("ttyUSB2"));
    modem->setRevision(QLatin1String("11.001.05.00.11"));
    modem->setSignalQuality({93, true});
    modem->setSim(QDBusObjectPath("/org/kde/fakemodem/SIM/1"));
    modem->setState(8);
    modem->setStateFailedReason(0);
    modem->setSupportedBands({0});
    modem->setSupportedCapabilities({4});
    modem->setSupportedIpFamilies(3);
    ModemManager::SupportedModesType supportedModes;
    ModemManager::CurrentModesType supportedMode1 = {MM_MODEM_MODE_4G & MM_MODEM_MODE_3G & MM_MODEM_MODE_2G, MM_MODEM_MODE_NONE};
    ModemManager::CurrentModesType supportedMode2 = {MM_MODEM_MODE_4G & MM_MODEM_MODE_3G & MM_MODEM_MODE_2G, MM_MODEM_MODE_2G};
    ModemManager::CurrentModesType supportedMode3 = {MM_MODEM_MODE_4G & MM_MODEM_MODE_3G & MM_MODEM_MODE_2G, MM_MODEM_MODE_3G};
    supportedModes << supportedMode1 << supportedMode2 << supportedMode3;
    modem->setSupportedModes(supportedModes);
    modem->setUnlockRequired(1);
    modem->setUnlockRetries({{MM_MODEM_LOCK_SIM_PIN, 3}, {MM_MODEM_LOCK_SIM_PIN2, 3}, {MM_MODEM_LOCK_SIM_PUK, 10}, {MM_MODEM_LOCK_SIM_PUK2, 10}});

    QSignalSpy addModemSpy(ModemManager::notifier(), SIGNAL(modemAdded(QString)));
    fakeModem->addModem(modem);
    QVERIFY(addModemSpy.wait());

    modemMessaging = new ModemMessaging(modem);
    modemMessaging->setModemPath(modem->modemPath());
    modemMessaging->setSupportedStorages({MM_SMS_STORAGE_SM, MM_SMS_STORAGE_ME});
    modemMessaging->setDefaultStorage(MM_SMS_STORAGE_SM);

    sms = new Sms();
    sms->setData(QByteArray("data"));
    sms->setDeliveryReportRequest(true);
    sms->setDeliveryState(MM_SMS_DELIVERY_STATE_COMPLETED_RECEIVED);
    sms->setDischargeTimestamp(QLatin1String("2015-03-30T16:31:53+00:00"));
    sms->setMessageReference(1);
    sms->setNumber(QLatin1String("+420777888999"));
    sms->setPduType(MM_SMS_PDU_TYPE_DELIVER);
    sms->setSMSC(QLatin1String("+420777666555"));
    sms->setSmsClass(1);
    sms->setState(MM_SMS_STATE_STORED);
    sms->setStorage(MM_SMS_STORAGE_SM);
#if MM_CHECK_VERSION(1, 2, 0)
    sms->setServiceCategory(MM_SMS_CDMA_SERVICE_CATEGORY_EMERGENCY_BROADCAST);
    sms->setTeleserviceId(MM_SMS_CDMA_TELESERVICE_ID_UNKNOWN);
#endif
    sms->setText(QLatin1String("hello"));
    sms->setTimestamp(QLatin1String("2015-03-30T16:31:53+00:00"));
    sms->setValidity({MM_SMS_VALIDITY_TYPE_RELATIVE, 0});

    ModemManager::MMVariantMapMap interfaces;
    interfaces.insert(QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MESSAGING), modemMessaging->toMap());
    fakeModem->addInterfaces(QDBusObjectPath(modem->modemPath()), interfaces);
    modemMessaging->setEnableNotifications(true);
}

void ModemMessagingPropertiesTest::testModemMessagingProperties()
{
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::modemDevices().first();
    QVERIFY(modemDevice);
    QVERIFY(modemDevice->hasInterface(ModemManager::ModemDevice::MessagingInterface));
    ModemManager::ModemMessaging::Ptr modemMessagingInterface =
        modemDevice->interface(ModemManager::ModemDevice::MessagingInterface).objectCast<ModemManager::ModemMessaging>();
    QVERIFY(modemMessagingInterface);
    QSignalSpy smsAddedSpy(modemMessagingInterface.data(), SIGNAL(messageAdded(QString, bool)));
    modemMessaging->addMessage(sms);
    QVERIFY(smsAddedSpy.wait());
    QCOMPARE(modemMessagingInterface->messages().count(), 1);

    ModemManager::Sms::Ptr modemSms = modemMessagingInterface->messages().first();
    QVERIFY(modemSms);

    sms->setData(QByteArray("data2"));
    QSignalSpy dataChangedSpy(modemSms.data(), SIGNAL(dataChanged(QByteArray)));
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(modemSms->data(), dataChangedSpy.at(0).at(0).toByteArray());

    sms->setDeliveryReportRequest(false);
    QSignalSpy deliveryReportRequestChangedSpy(modemSms.data(), SIGNAL(deliveryReportRequestChanged(bool)));
    QVERIFY(deliveryReportRequestChangedSpy.wait());
    QCOMPARE(modemSms->deliveryReportRequest(), deliveryReportRequestChangedSpy.at(0).at(0).toBool());

    sms->setDeliveryState(MM_SMS_DELIVERY_STATE_TEMPORARY_ERROR_SME_BUSY);
    QSignalSpy deliveryStateChangedSpy(modemSms.data(), SIGNAL(deliveryStateChanged(MMSmsDeliveryState)));
    QVERIFY(deliveryStateChangedSpy.wait());
    QCOMPARE(modemSms->deliveryState(), deliveryStateChangedSpy.at(0).at(0).value<MMSmsDeliveryState>());

    sms->setDischargeTimestamp(QLatin1String("2014-03-30T16:31:53+00:00"));
    QSignalSpy dischargeTimestampChangedSpy(modemSms.data(), SIGNAL(dischargeTimestampChanged(QDateTime)));
    QVERIFY(dischargeTimestampChangedSpy.wait());
    QCOMPARE(modemSms->dischargeTimestamp(), dischargeTimestampChangedSpy.at(0).at(0).toDateTime());

    sms->setMessageReference(2);
    QSignalSpy messageReferenceChangedSpy(modemSms.data(), SIGNAL(messageReferenceChanged(uint)));
    QVERIFY(messageReferenceChangedSpy.wait());
    QCOMPARE(modemSms->messageReference(), messageReferenceChangedSpy.at(0).at(0).toUInt());

    sms->setNumber(QLatin1String("+420777111222"));
    QSignalSpy numberChangedSpy(modemSms.data(), SIGNAL(numberChanged(QString)));
    QVERIFY(numberChangedSpy.wait());
    QCOMPARE(modemSms->number(), numberChangedSpy.at(0).at(0).toString());

    sms->setPduType(MM_SMS_PDU_TYPE_SUBMIT);
    QSignalSpy pduTypeChangedSpy(modemSms.data(), SIGNAL(pduTypeChanged(MMSmsPduType)));
    QVERIFY(pduTypeChangedSpy.wait());
    QCOMPARE(modemSms->pduType(), pduTypeChangedSpy.at(0).at(0).value<MMSmsPduType>());

    sms->setSMSC(QLatin1String("+420777878978"));
    QSignalSpy SMSCChangedSpy(modemSms.data(), SIGNAL(SMSCChanged(QString)));
    QVERIFY(SMSCChangedSpy.wait());
    QCOMPARE(modemSms->SMSC(), SMSCChangedSpy.at(0).at(0).toString());

    sms->setSmsClass(2);
    QSignalSpy smsClassChangedSpy(modemSms.data(), SIGNAL(smsClassChanged(int)));
    QVERIFY(smsClassChangedSpy.wait());
    QCOMPARE(modemSms->smsClass(), smsClassChangedSpy.at(0).at(0).toInt());

    sms->setState(MM_SMS_STATE_SENT);
    QSignalSpy stateChangedSpy(modemSms.data(), SIGNAL(stateChanged(MMSmsState)));
    QVERIFY(stateChangedSpy.wait());
    QCOMPARE(modemSms->state(), stateChangedSpy.at(0).at(0).value<MMSmsState>());

    sms->setStorage(MM_SMS_STORAGE_MT);
    QSignalSpy storageChangedSpy(modemSms.data(), SIGNAL(storageChanged(MMSmsStorage)));
    QVERIFY(storageChangedSpy.wait());
    QCOMPARE(modemSms->storage(), storageChangedSpy.at(0).at(0).value<MMSmsStorage>());

#if MM_CHECK_VERSION(1, 2, 0)
    sms->setServiceCategory(MM_SMS_CDMA_SERVICE_CATEGORY_UNKNOWN);
    QSignalSpy serviceCategoryChangedSpy(modemSms.data(), SIGNAL(serviceCategoryChanged(MMSmsCdmaServiceCategory)));
    QVERIFY(serviceCategoryChangedSpy.wait());
    QCOMPARE(modemSms->serviceCategory(), serviceCategoryChangedSpy.at(0).at(0).value<MMSmsCdmaServiceCategory>());

    sms->setTeleserviceId(MM_SMS_CDMA_TELESERVICE_ID_CMT91);
    QSignalSpy teleserviceIdChangedSpy(modemSms.data(), SIGNAL(teleserviceIdChanged(MMSmsCdmaTeleserviceId)));
    QVERIFY(teleserviceIdChangedSpy.wait());
    QCOMPARE(modemSms->teleserviceId(), teleserviceIdChangedSpy.at(0).at(0).value<MMSmsCdmaTeleserviceId>());
#endif

    sms->setText(QLatin1String("hello2"));
    QSignalSpy textChangedSpy(modemSms.data(), SIGNAL(textChanged(QString)));
    QVERIFY(textChangedSpy.wait());
    QCOMPARE(modemSms->text(), textChangedSpy.at(0).at(0).toString());

    sms->setTimestamp(QLatin1String("2014-03-30T16:31:53+00:00"));
    QSignalSpy timestampChangedSpy(modemSms.data(), SIGNAL(timestampChanged(QDateTime)));
    QVERIFY(timestampChangedSpy.wait());
    QCOMPARE(modemSms->timestamp(), timestampChangedSpy.at(0).at(0).toDateTime());

    sms->setValidity({MM_SMS_VALIDITY_TYPE_ABSOLUTE, 1});
    QSignalSpy validityChangedSpy(modemSms.data(), SIGNAL(validityChanged(ModemManager::ValidityPair)));
    QVERIFY(validityChangedSpy.wait());
    QCOMPARE(modemSms->validity().validity, validityChangedSpy.at(0).at(0).value<ModemManager::ValidityPair>().validity);
    QCOMPARE(modemSms->validity().value, validityChangedSpy.at(0).at(0).value<ModemManager::ValidityPair>().value);

    QSignalSpy smsDeletedSpy(modemMessagingInterface.data(), SIGNAL(messageDeleted(QString)));
    modemMessaging->Delete(QDBusObjectPath(sms->smsPath()));
    QVERIFY(smsDeletedSpy.wait());
    QCOMPARE(modemMessagingInterface->messages().count(), 0);
}

QTEST_MAIN(ModemMessagingPropertiesTest)

#include "moc_modemmessagingpropertiestest.cpp"
