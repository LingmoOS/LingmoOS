// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <ddbusextendedabstractinterface.h>
#include "fakedbus/fakedbusservice.h"

#include <QTest>
#include <QSignalSpy>

DCORE_USE_NAMESPACE

class DBusExtendedInterfaceFoo : public DDBusExtendedAbstractInterface {
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
        { return "org.deepin.FakeDBusService"; }

    explicit DBusExtendedInterfaceFoo(QObject *parent = nullptr)
        : DDBusExtendedAbstractInterface(FakeDBusService::get_service(),
                                         FakeDBusService::get_path(),
                                         staticInterfaceName(),
                                         QDBusConnection::sessionBus(),
                                         parent) {
        connect(this, &DBusExtendedInterfaceFoo::propertyChanged, this,
                [this](const QString &propName, const QVariant &value){
            if (propName == QStringLiteral("strProperty"))
               {
                   const QString &strProperty = qvariant_cast<QString>(value);
                   if (m_strProp != strProperty)
                   {
                       m_strProp = strProperty;
                       Q_EMIT StrPropertyChanged(m_strProp);
                   }
                   return;
               }

               qWarning() << "property not handle: " << propName;
               return;
        });
    }
    ~DBusExtendedInterfaceFoo(){

    }

    Q_PROPERTY(QString strProperty READ strProperty WRITE setStrProperty NOTIFY StrPropertyChanged)
    QString strProperty() {
        return qvariant_cast<QString>(internalPropGet("strProperty", &m_strProp));
    }
    void setStrProperty(const QString &value) {
        internalPropSet("strProperty", QVariant::fromValue(value), &m_strProp);
    }
Q_SIGNALS: // SIGNALS
    void StrPropertyChanged(const QString & value) const;
private:
    QString m_strProp;
};

class ut_DDBusExtendedAbstractInterface : public testing::Test
{
public:
    void SetUp() override
    {
        m_fakeService = new FakeDBusService;
        m_dbusExtend = new DBusExtendedInterfaceFoo;
    }
    void TearDown() override
    {
        delete m_fakeService;
        delete m_dbusExtend;
    }
    DBusExtendedInterfaceFoo *m_dbusExtend = nullptr;
    FakeDBusService *m_fakeService = nullptr;
};

TEST_F(ut_DDBusExtendedAbstractInterface, sync)
{
    EXPECT_TRUE(m_dbusExtend->sync());
    m_dbusExtend->setSync(false, false);
    EXPECT_FALSE(m_dbusExtend->sync());
}

TEST_F(ut_DDBusExtendedAbstractInterface, useCache)
{
    EXPECT_FALSE(m_dbusExtend->useCache());
    m_dbusExtend->setUseCache(true);
    EXPECT_TRUE(m_dbusExtend->useCache());
}

TEST_F(ut_DDBusExtendedAbstractInterface, startServiceProcess)
{
    EXPECT_TRUE(m_dbusExtend->isValid());
    m_dbusExtend->startServiceProcess();
}

TEST_F(ut_DDBusExtendedAbstractInterface, getAllProperties)
{
    QSignalSpy spy(m_dbusExtend,  &DDBusExtendedAbstractInterface::asyncGetAllPropertiesFinished);
    m_dbusExtend->setSync(false);

    m_dbusExtend->getAllProperties();

    EXPECT_TRUE(QTest::qWaitFor([&spy, this]() {
        return spy.count() >= 1 && !m_dbusExtend->lastExtendedError().isValid();
    }, 2000));
}

TEST_F(ut_DDBusExtendedAbstractInterface, internalPropGet)
{
    QSignalSpy asyncPropertyFinishedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::asyncPropertyFinished);
    QSignalSpy propertyChangedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged);
    m_dbusExtend->setSync(false);

    // internalPropGet
    m_dbusExtend->strProperty();

    QObject::connect(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged,
                     m_dbusExtend, [](const QString &propertyName, const QVariant &value){
        if (propertyName == "strProp")
            EXPECT_EQ(value.toString(), "testDBusService");

        qInfo() << "propertyChanged" << propertyName << value;
    });

    EXPECT_TRUE(QTest::qWaitFor([&]() {
        return asyncPropertyFinishedSpy.count() >= 1 &&
                propertyChangedSpy.count() >= 1 &&
                !m_dbusExtend->lastExtendedError().isValid();
    }, 2000));
}

TEST_F(ut_DDBusExtendedAbstractInterface, internalPropSet)
{
    QSignalSpy asyncPropertyFinishedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::asyncSetPropertyFinished);
    QSignalSpy propertyChangedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged);
    m_dbusExtend->setSync(false);

    // internalPropSet
    m_dbusExtend->setStrProperty("MyProp");

    QObject::connect(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged,
                     m_dbusExtend, [](const QString &propertyName, const QVariant &value){
        if (propertyName == "strProp")
            EXPECT_EQ(value.toString(), "MyProp");

        qInfo() << "propertyChanged" << propertyName << value;
    });

    EXPECT_TRUE(QTest::qWaitFor([&]() {
        return asyncPropertyFinishedSpy.count() >= 1 &&
                propertyChangedSpy.count() >= 1 &&
                !m_dbusExtend->lastExtendedError().isValid();
    }, 2000));
}

#include "ut_ddbusextendedabstractinterface.moc"
