// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddeindicatortrayprotocol.h"
#include "abstracttrayprotocol.h"

#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QPointer>
#include <QPainter>
#include <QDBusReply>
#include <QPaintEvent>
#include <QJsonObject>
#include <DDBusSender>
#include <QApplication>
#include <QDBusMessage>
#include <QJsonDocument>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QLoggingCategory>

#include <cmath>
#include <thread>

namespace tray {

Q_LOGGING_CATEGORY(indicatorLog, "dde.shell.tray.indicator")
static QString indicatorPfrefix = QStringLiteral("Indicator:");

DDEindicatorProtocol::DDEindicatorProtocol(QObject *parent)
    : AbstractTrayProtocol(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    // TODO: use cmake defined path
    m_watcher->addPath("/etc/dde-dock/indicator/");

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &DDEindicatorProtocol::registedItemChanged);

    QMetaObject::invokeMethod(this, &DDEindicatorProtocol::registedItemChanged, Qt::QueuedConnection);
}

DDEindicatorProtocol::~DDEindicatorProtocol()
{
}

void DDEindicatorProtocol::registedItemChanged()
{
    QStringList indicatorPaths;

    QString path;
    Q_FOREACH(path, m_watcher->directories()) {
        QDir indicatorConfDir(path);
        for (const QFileInfo &fileInfo : indicatorConfDir.entryInfoList({"*.json"}, QDir::Files | QDir::NoDotAndDotDot)) {
            indicatorPaths << fileInfo.canonicalFilePath();
        }
    }

    for (auto currentIndicator : indicatorPaths) {
        if (!m_registedItem.contains(currentIndicator)) {
            auto trayHandler = QSharedPointer<DDEindicatorProtocolHandler>(new DDEindicatorProtocolHandler(currentIndicator));
            m_registedItem.insert(currentIndicator, trayHandler);
            Q_EMIT AbstractTrayProtocol::trayCreated(trayHandler.get());
        }
    }

    for (auto alreadyRegistedItem : m_registedItem.keys()) {
        if (!indicatorPaths.contains(alreadyRegistedItem)) {
            if (auto value = m_registedItem.value(alreadyRegistedItem, nullptr)) {
                m_registedItem.remove(alreadyRegistedItem);
            }
        }
    }
}

class DDEindicatorProtocolHandlerPrivate
{
public:
    explicit DDEindicatorProtocolHandlerPrivate(DDEindicatorProtocolHandler *parent) : q_ptr(parent) {}

    void init();
    void initDBus();
    void updateContent();

    template<typename Func>
    void featData(const QString &key,
                  const QJsonObject &data,
                  const char *propertyChangedSlot,
                  Func const &callback);
    
    template<typename Func>
    void propertyChanged(const QString &key, const QDBusMessage &msg, Func const &callback);

    bool                    enabled;
    QString                 indicatorName;
    QString                 indicatorPath;
    QMap<QString, QString>  propertyNames;
    QMap<QString, QString>  propertyInterfaceNames;

    QByteArray m_pixmapData;
    QString m_text;

    DDEindicatorProtocolHandler* q_ptr;
    Q_DECLARE_PUBLIC(DDEindicatorProtocolHandler)
};


void DDEindicatorProtocolHandlerPrivate::init()
{
    initDBus();
    updateContent();
}

void DDEindicatorProtocolHandlerPrivate::initDBus()
{
    Q_Q(DDEindicatorProtocolHandler);

    QFile confFile(indicatorPath);
    if (!confFile.open(QIODevice::ReadOnly)) {
        qCWarning(indicatorLog()) << "read indicator config Error";
    }

    QJsonDocument doc = QJsonDocument::fromJson(confFile.readAll());
    confFile.close();
    auto config = doc.object();
    auto delay = config.value("delay").toInt(0);

    qDebug(indicatorLog()) << "delay load" << delay << indicatorPath << q;

    QTimer::singleShot(delay, [ = ]() {
        auto data = config.value("data").toObject();

        if (data.contains("text")) {
            featData("text", data, SLOT(textPropertyChanged(QDBusMessage)), [ = ](QVariant v) {
                auto res = v.toByteArray();
                q->setEnabled(!res.isEmpty());
                m_text = v.toString();
                updateContent();
            });
        }

        if (data.contains("icon")) {
            featData("icon", data, SLOT(iconPropertyChanged(QDBusMessage)), [ = ](QVariant v) {
                auto res = v.toByteArray();
                q->setEnabled(!res.isEmpty());
                m_pixmapData = res;
                updateContent();
            });
        }

        const QJsonObject action = config.value("action").toObject();
        if (!action.isEmpty())
            q->connect(q, &DDEindicatorProtocolHandler::clicked, q, [ = ](uint8_t button_index, int x, int y) {
                std::thread t([ = ] ()-> void {
                    auto triggerConfig = action.value("trigger").toObject();
                    auto dbusService = triggerConfig.value("dbus_service").toString();
                    auto dbusPath = triggerConfig.value("dbus_path").toString();
                    auto dbusInterface = triggerConfig.value("dbus_interface").toString();
                    auto methodName = triggerConfig.value("dbus_method").toString();
                    auto isSystemBus = triggerConfig.value("system_dbus").toBool(false);
                    auto bus = isSystemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();

                    QDBusInterface interface(dbusService, dbusPath, dbusInterface, bus);
                    QDBusReply<void> reply = interface.call(methodName, button_index, x, y);
                    if (!reply.isValid()) {
                        interface.call(methodName);
                    }
                });
                t.detach();
            });
    });
}

void DDEindicatorProtocolHandlerPrivate::updateContent()
{
    Q_Q(DDEindicatorProtocolHandler);
    if (!enabled) return;

    auto widget = static_cast<QWidget*>(q->parent());

    if (widget) {
        widget->update();
    }
}

template<typename Func>
void DDEindicatorProtocolHandlerPrivate::featData(const QString &key,
            const QJsonObject &data,
            const char *propertyChangedSlot,
            Func const &callback)
{
    Q_Q(DDEindicatorProtocolHandler);
    auto dataConfig = data.value(key).toObject();
    auto dbusService = dataConfig.value("dbus_service").toString();
    auto dbusPath = dataConfig.value("dbus_path").toString();
    auto dbusInterface = dataConfig.value("dbus_interface").toString();
    auto isSystemBus = dataConfig.value("system_dbus").toBool(false);
    auto bus = isSystemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();

    QDBusInterface interface(dbusService, dbusPath, dbusInterface, bus, q);

    if (dataConfig.contains("dbus_method")) {
        QString methodName = dataConfig.value("dbus_method").toString();
        auto ratio = qApp->devicePixelRatio();
        QDBusReply<QByteArray> reply = interface.call(methodName.toStdString().c_str(), ratio);
        callback(reply.value());
    }

    if (dataConfig.contains("dbus_properties")) {
        auto propertyName = dataConfig.value("dbus_properties").toString();
        auto propertyNameCStr = propertyName.toStdString();
        propertyInterfaceNames.insert(key, dbusInterface);
        propertyNames.insert(key, QString::fromStdString(propertyNameCStr));
        QDBusConnection::sessionBus().connect(dbusService,
                                                dbusPath,
                                                "org.freedesktop.DBus.Properties",
                                                "PropertiesChanged",
                                                "sa{sv}as",
                                                q,
                                                propertyChangedSlot);

        QDBusConnection::sessionBus().connect(dbusService,
                                                dbusPath,
                                                dbusInterface,
                                                QString("%1Changed").arg(propertyName),
                                                "s",
                                                q,
                                                propertyChangedSlot);

        callback(interface.property(propertyNameCStr.c_str()));
    }
}

template<typename Func>
void DDEindicatorProtocolHandlerPrivate::propertyChanged(const QString &key, const QDBusMessage &msg, Func const &callback)
{
    QList<QVariant> arguments = msg.arguments();
    if (1 == arguments.count()) {
        const QString &v = msg.arguments().at(0).toString();
        callback(v);
        return;
    } else if (3 != arguments.count()) {
        qDebug(indicatorLog()) << "arguments count must be 3";
        return;
    }

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != propertyInterfaceNames.value(key)) {
        qDebug(indicatorLog()) << "interfaceName mismatch" << interfaceName << propertyInterfaceNames.value(key) << key;
        return;
    }
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    if (changedProps.contains(propertyNames.value(key))) {
        callback(changedProps.value(propertyNames.value(key)));
    }
}

DDEindicatorProtocolHandler::DDEindicatorProtocolHandler(QString indicatorFilePath, QObject *parent)
    : AbstractTrayProtocolHandler(parent)
    , d_ptr(new DDEindicatorProtocolHandlerPrivate(this))
{
    Q_D(DDEindicatorProtocolHandler);
    d->indicatorPath = indicatorFilePath;
    d->indicatorName = indicatorFilePath.split(QDir::separator()).last();
    d->enabled = false;
    d->init();
}

DDEindicatorProtocolHandler::~DDEindicatorProtocolHandler()
{

}

uint32_t DDEindicatorProtocolHandler::windowId() const
{
    return 0;
}

QString DDEindicatorProtocolHandler::id() const
{
    return indicatorPfrefix + d_ptr->indicatorName;
}
    
QString DDEindicatorProtocolHandler::title() const
{
    return d_ptr->indicatorName;
}

QString DDEindicatorProtocolHandler::status() const
{
    return d_ptr->enabled ? "Active" : "DeActive";
}

QString DDEindicatorProtocolHandler::category() const
{
    return "Indicator";
}

QIcon DDEindicatorProtocolHandler::overlayIcon() const
{
    return QIcon();
}

QIcon DDEindicatorProtocolHandler::attentionIcon() const
{
    return QIcon();
}

QIcon DDEindicatorProtocolHandler::icon() const
{
    return QIcon();
}

bool DDEindicatorProtocolHandler::enabled() const
{
    return d_ptr->enabled;
}

void DDEindicatorProtocolHandler::setEnabled(bool enabled)
{
    Q_D(DDEindicatorProtocolHandler);
    if (enabled == d->enabled) {
        return;
    }

    d->enabled = enabled;
    Q_EMIT enabledChanged();
}

bool DDEindicatorProtocolHandler::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DDEindicatorProtocolHandler);

    if (watched == parent()) {
        if (event->type() == QEvent::Paint) {
            auto widget = static_cast<QWidget*>(parent());
            QPainter painter(widget);
            QFontMetrics qfm = QFontMetrics(widget->font());
            QRect tightTextRect = qfm.tightBoundingRect(d->m_text);
            QRect textRect = qfm.boundingRect(d->m_text);
            QPoint topLeft = textRect.topLeft() - tightTextRect.topLeft();
            QPoint bottom = textRect.bottomRight() - tightTextRect.bottomRight();
            QPoint center = QPoint(std::floor((widget->rect().width() - textRect.width()) / 2), std::floor((widget->rect().height() - textRect.height()) / 2)) // this make textRect in center
                                + (topLeft + bottom) / 2; // this adjust make tightTextRect in center
            painter.drawText(QRect(center.x(), center.y(), textRect.width() + 1, textRect.height() + 1), (d->m_text));

            if (d->m_pixmapData != nullptr) {
                auto rawPixmap = QPixmap::fromImage(QImage::fromData(d->m_pixmapData));
                rawPixmap.setDevicePixelRatio(qApp->devicePixelRatio());
                painter.drawPixmap(widget->rect(), rawPixmap);
            }
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            // TODO: point map2global
            Q_EMIT clicked(mouseEvent->button(), 200, 200);
        }
    }
    return false;
}

void DDEindicatorProtocolHandler::textPropertyChanged(const QDBusMessage &message)
{
    Q_D(DDEindicatorProtocolHandler);

    d->propertyChanged("text", message, [ = ] (const QVariant &value) {
        auto res = value.toByteArray();
        setEnabled(!res.isEmpty());

        d->m_text = QString::fromLocal8Bit(value.toByteArray());
        d->updateContent();
    });
}

void DDEindicatorProtocolHandler::iconPropertyChanged(const QDBusMessage &message)
{
    Q_D(DDEindicatorProtocolHandler);

    d->propertyChanged("icon", message, [ = ] (const QVariant &value) {
        auto res = value.toByteArray();
        setEnabled(!res.isEmpty());
        d->m_pixmapData= value.toByteArray();
        d->updateContent();
    });
}
}
