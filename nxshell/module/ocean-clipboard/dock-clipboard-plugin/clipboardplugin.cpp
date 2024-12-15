// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clipboardplugin.h"

#include <QDBusConnection>

#define CLIPBOARD_KEY "clipboard-key"
DCORE_USE_NAMESPACE

ClipboardPlugin::ClipboardPlugin(QObject *parent)
    : QObject(parent)
    , m_quickPanelWidget(nullptr)
    , m_item(nullptr)
    , m_messageCallback(nullptr)
{
    QTranslator *translator = new QTranslator(this);
    if (translator->load(QString("/usr/share/ocean-clipboard/translations/ocean-clipboard_%1.qm").arg(QLocale::system().name()))) {
        QCoreApplication::installTranslator(translator);
    }
}

ClipboardPlugin::~ClipboardPlugin()
{
    if (m_quickPanelWidget) {
        delete m_quickPanelWidget;
        m_quickPanelWidget = nullptr;
    }

    if (m_item) {
        delete m_item;
        m_item = nullptr;
    }
}

const QString ClipboardPlugin::pluginName() const
{
    return "clipboard";
}

const QString ClipboardPlugin::pluginDisplayName() const
{
    return tr("Clipboard");
}

void ClipboardPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    if (m_item || m_quickPanelWidget) {
        return;
    }
    m_item = new ClipboardItem();
    m_quickPanelWidget = new QuickPanelWidget();

    m_proxyInter->itemAoceand(this, CLIPBOARD_KEY);
    m_quickPanelWidget->setDescription(pluginDisplayName());
    m_quickPanelWidget->setIcon(QIcon::fromTheme("clipboard"));

    connect(m_item, &ClipboardItem::requestHideApplet, this, [this] {
        m_proxyInter->requestSetAppletVisible(this, CLIPBOARD_KEY, false);
    });
    connect(m_quickPanelWidget, &QuickPanelWidget::requestHideApplet, this, [this] {
        m_proxyInter->requestSetAppletVisible(this, CLIPBOARD_KEY, false);
    });
    QDBusConnection::sessionBus().connect("org.lingmo.ocean.Clipboard1", "/org/lingmo/ocean/Clipboard1", "org.lingmo.ocean.Clipboard1", "clipboardVisibleChanged", this, SLOT(onClipboardVisibleChanged(bool)));
}

QWidget *ClipboardPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey == CLIPBOARD_KEY) {
        return m_item;
    }

    if (itemKey == Dock::QUICK_ITEM_KEY) {
        return m_quickPanelWidget;
    }

    return nullptr;
}

QWidget *ClipboardPlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey == CLIPBOARD_KEY) {
        return m_item->tipsWidget();
    }

    return nullptr;
}

QWidget *ClipboardPlugin::itemPopupApplet(const QString &itemKey)
{
    return nullptr;
}

const QString ClipboardPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey == CLIPBOARD_KEY) {
        return m_item->contextMenu();
    }

    return QString();
}

void ClipboardPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    if (itemKey == CLIPBOARD_KEY) {
        m_item->invokeMenuItem(menuId, checked);
    }
}

int ClipboardPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    return m_proxyInter->getValue(this, key, -1).toInt();
}

void ClipboardPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    m_proxyInter->saveValue(this, key, order);
}

void ClipboardPlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == CLIPBOARD_KEY) {
        m_item->refreshIcon();
    }
}

const QString ClipboardPlugin::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return QString("dbus-send --session --print-reply --dest=org.lingmo.ocean.Clipboard1 /org/lingmo/ocean/Clipboard1 org.lingmo.ocean.Clipboard1.Toggle");
}

void ClipboardPlugin::onClipboardVisibleChanged(bool visible)
{
    if (!m_messageCallback) {
        qWarning() << "Message callback function is nullptr";
        return;
    }
    QJsonObject msg;
    msg[Dock::MSG_TYPE] = Dock::MSG_ITEM_ACTIVE_STATE;
    msg[Dock::MSG_DATA] = visible;
    QJsonDocument doc;
    doc.setObject(msg);
    m_messageCallback(this, doc.toJson());
}
