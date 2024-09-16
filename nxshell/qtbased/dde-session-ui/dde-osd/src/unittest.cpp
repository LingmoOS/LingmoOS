// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unittest.h"
#include "notification/bubbletool.h"
#include "notification/bubble.h"
#include "notification/appicon.h"
#include "notification/persistence.h"
#include "notification-center/bubbleitem.h"
#include "notification-center/notifymodel.h"
#include "notification-center/notifylistview.h"

#include <QString>
#include <QThread>
#include <QStringList>
#include <DDBusSender>
#include <QJsonArray>

#include <QCheckBox>
#include <QWidget>

static const QString NotifyInterPath = "/org/deepin/dde/Notification1";
static const QString NotifyInterSrever = "org.deepin.dde.Notification1";

UnitTest::UnitTest()
    : m_notifyInter(new NotifyInter(NotifyInterSrever, NotifyInterPath,
                                    QDBusConnection::sessionBus(), this))
{

}

UnitTest::~UnitTest()
{

}

EntityPtr UnitTest::createEntity()
{
    QString appName = "deepin-screen-recorder";
    uint id = 0;
    QString appIcon = "deepin-screenshot";
    QString summary = "截图完成";
    QString body = "图片已保存到/home/chenwei/Desktop/截图录屏_20200709104935.png";
    QStringList actions ;
    actions << "_open" << "查看";
    QVariantMap hints;
    int expireTimeout = -1;

    EntityPtr notification = std::make_shared<NotificationEntity>(appName, QString(), appIcon,
                                                                  summary, body, actions, hints,
                                                                  QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                                  QString::number(id),
                                                                  QString::number(expireTimeout),
                                                                  this);
    return notification;
}

void UnitTest::test_NotifyInter()
{
    EntityPtr entity = createEntity();
    uint replaceId = m_notifyInter->Notify(entity->appName(), entity->replacesId().toUInt(),
                                           entity->appIcon(), entity->summary(), entity->body(),
                                           entity->actions(), entity->hints(), entity->timeout().toInt());
    QVERIFY(replaceId > 0);

    m_notifyInter->Toggle();

    QStringList argList;
    argList << "AudioUp" << "AudioDown" << "AudioMute";
    argList << "BrightnessUp" << "BrightnessDown";
    argList << "CapsLockOn" << "CapsLockOff";
    argList << "NumLockOn" << "NumLockOff";
    argList << "TouchpadOn" << "TouchpadOff"<< "TouchpadToggle";
    argList << "SwitchWM3D" << "SwitchWM2D"<< "SwitchWMError";
    argList << "AudioMicMuteOn" << "AudioMicMuteOff";
    argList << "WLANOn" << "WLANOff";

    foreach(QString arg , argList) {
        DDBusSender().service("org.deepin.dde.Osd1").interface("org.deepin.dde.Osd1").path("/")
                     .method(QString("ShowOSD")).arg(arg).call();
        QThread::msleep(500);
    }
}

void UnitTest::test_BubbleTool()
{
    QString key = "deepin-screen-recorder";
    QString appName = BubbleTool::getDeepinAppName(key);//获取应用名称

    QCOMPARE("截图录屏", appName);
}

void UnitTest::test_BubbleItem()
{
    EntityPtr entity = createEntity();
    QWidget widget;
    BubbleItem item(&widget, entity);

    // 可以连接到任何物体的任何信号并记录其发射。QSignalSpy本身就是一个QVariant列表的列表。
    // 信号的每次发射都会在列表中追加一项，其中包含信号的参数。
    QSignalSpy spy(&item, SIGNAL(havorStateChanged(bool)));

    // 触发一次该信号的事件，QEvent::Leave事件havorStateChanged信号携带的参数为false
    QEvent hoverEvent(QEvent::Leave);
    QApplication::sendEvent(&item, &hoverEvent);

    // 判断信号是否只发送了一次
    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    // 判断信号发送的数据类型
    QVERIFY(arguments.at(0).type() == QVariant::Bool);
    // 判断信号发送的结果
    QCOMPARE(arguments.at(0).toBool(), false);
}

void UnitTest::test_NotifyModel()
{
    // 测试model的方法，直接构造一个QAbstractItemModelTester，Qt会自动测试该model类
    NotifyListView view;
    Persistence database;
    NotifyModel model(this, &database);
    auto tester = new QAbstractItemModelTester(&model);
    Q_UNUSED(tester)
}

void UnitTest::test_Bubble()
{
    EntityPtr entity = createEntity();
    QWidget widget;
    Bubble bubble(&widget, entity);

    // 测试鼠标点击并松开后是否发出dismissed信号
    qRegisterMetaType<Bubble *>("Bubble *");
    QSignalSpy spy(&bubble, SIGNAL(dismissed(Bubble *)));
    QTest::mousePress(&bubble, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 0));
    QTest::mouseRelease(&bubble, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 0));

    QCOMPARE(spy.count(), 1);

    // 测试鼠标点击并松开后，如果松开与点击的位置不同，是否发出notProcessedYet信号与dismissed信号
    qRegisterMetaType<EntityPtr>("EntityPtr");
    QSignalSpy spy1(&bubble, SIGNAL(notProcessedYet(EntityPtr)));
    QSignalSpy spy2(&bubble, SIGNAL(dismissed(Bubble *)));
    QTest::mousePress(&bubble, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 0));
    QTest::mouseRelease(&bubble, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 5));

    QTRY_COMPARE_WITH_TIMEOUT(spy1.count(), 1, AnimationTime + 20);
    QCOMPARE(spy2.count(), 1);
}

void UnitTest::test_Persistence()
{
    Persistence database;
    EntityPtr entity = createEntity();

    database.addOne(entity);
    QString data = database.getById(QString("%1").arg(entity->id()));
    QJsonArray notify_array = QJsonDocument::fromJson(data.toLocal8Bit().data()).array();
    QJsonObject notify = notify_array.begin()->toObject();
    QCOMPARE(notify.value("id").toString(), QString("%1").arg(entity->id()));

    QList<EntityPtr> entityList= database.getAllNotify();
    QCOMPARE(entityList.size(), database.recordCount());

    database.removeAll();
    QCOMPARE(database.recordCount(), 0);
}


