/*
 *   SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kcmusb.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QList>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QTreeWidget>

#include <KLocalizedString>
#include <KPluginFactory>

#include "usbdevices.h"

K_PLUGIN_CLASS_WITH_JSON(USBViewer, "kcmusb.json")

USBViewer::USBViewer(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(widget());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QSplitter *splitter = new QSplitter(widget());
    splitter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    mainLayout->addWidget(splitter);

    _devices = new QTreeWidget(splitter);

    QStringList headers{i18n("Device")};
    _devices->setHeaderLabels(headers);
    _devices->setRootIsDecorated(true);
    _devices->header()->hide();
    //_devices->setColumnWidthMode(0, Q3ListView::Maximum);

    QList<int> sizes;
    sizes.append(200);
    splitter->setSizes(sizes);

    _details = new QTextEdit(splitter);
    _details->setReadOnly(true);

    QTimer *refreshTimer = new QTimer(this);
    // 1 sec seems to be a good compromise between latency and polling load.
    refreshTimer->start(1000);

    connect(refreshTimer, &QTimer::timeout, this, &USBViewer::refresh);
    connect(_devices, &QTreeWidget::currentItemChanged, this, &USBViewer::selectionChanged);
}

USBViewer::~USBViewer()
{
    USBDevice::clear();
}

void USBViewer::load()
{
    _items.clear();
    _devices->clear();

    refresh();
}

static quint32 key(USBDevice &dev)
{
    return dev.bus() * 256 + dev.device();
}

static quint32 key_parent(USBDevice &dev)
{
    return dev.bus() * 256 + dev.parent();
}

static void delete_recursive(QTreeWidgetItem *item, const QMap<int, QTreeWidgetItem *> &new_items)
{
    if (!item)
        return;

    QTreeWidgetItemIterator it(item, QTreeWidgetItemIterator::All);
    while (*it != nullptr) {
        QTreeWidgetItem *currentItem = *it;
        if (new_items.contains(currentItem->text(1).toUInt()) == false) {
            delete_recursive(currentItem->child(0), new_items);
            delete currentItem;
        }
        ++it;
    }
}

void USBViewer::refresh()
{
    QMap<int, QTreeWidgetItem *> new_items;

    if (!USBDevice::load())
        return;

    int level = 0;
    bool found = true;

    while (found) {
        found = false;

        const auto devices = USBDevice::devices();
        for (USBDevice *usbDevice : devices) {
            if (usbDevice->level() == level) {
                quint32 k = key(*usbDevice);
                if (level == 0) {
                    QTreeWidgetItem *item = _items.value(k);
                    if (!item) {
                        QStringList itemContent;
                        itemContent << usbDevice->product() << QString::number(k);
                        item = new QTreeWidgetItem(_devices, itemContent);
                    }
                    new_items.insert(k, item);
                    found = true;
                } else {
                    QTreeWidgetItem *parent = new_items.value(key_parent(*usbDevice));
                    if (parent) {
                        QTreeWidgetItem *item = _items.value(k);

                        if (!item) {
                            QStringList itemContent;
                            itemContent << usbDevice->product() << QString::number(k);
                            item = new QTreeWidgetItem(parent, itemContent);
                        }
                        new_items.insert(k, item);
                        parent->setExpanded(true);
                        found = true;
                    }
                }
            }
        }

        ++level;
    }

    // recursive delete all items not in new_items
    delete_recursive(_devices->topLevelItem(0), new_items);

    _items = new_items;

    if (_devices->selectedItems().isEmpty() == true)
        selectionChanged(_devices->topLevelItem(0));
}

void USBViewer::selectionChanged(QTreeWidgetItem *item)
{
    if (item) {
        quint32 busdev = item->text(1).toUInt();
        USBDevice *dev = USBDevice::find(busdev >> 8, busdev & 255);
        if (dev) {
            _details->setHtml(dev->dump());
            return;
        }
    }
    _details->clear();
}

#include "kcmusb.moc"

#include "moc_kcmusb.cpp"
