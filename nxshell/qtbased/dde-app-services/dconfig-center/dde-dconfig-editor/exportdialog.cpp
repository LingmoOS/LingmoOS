// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exportdialog.h"
#include "helper.hpp"
#include "valuehandler.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QFileDialog>
#include <QTreeView>
#include <QMessageBox>

ExportDialog::ExportDialog(QWidget *parent)
    : DDialog( parent)
{
    m_exportView = new QTreeView();
    m_exportView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QCheckBox *checkbox = new QCheckBox();
    checkbox->setText(tr("select all"));
    m_exportBtn = new DSuggestButton(tr("export"));
    m_exportBtn->setEnabled(false);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addStretch(1);
    hLayout->addWidget(checkbox);
    hLayout->addWidget(m_exportBtn);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_exportView);
    vLayout->addLayout(hLayout);

    QWidget *widget = new QWidget;
    widget->setLayout(vLayout);

    addContent(widget);

    connect(checkbox, &QCheckBox::clicked, this, [this, checkbox] {
        for (auto item : m_rootItems)
            checkAllChild(item, checkbox->checkState() == Qt::Checked ? true : false);
    });
    connect(m_exportBtn, &DSuggestButton::clicked, this, &ExportDialog::saveFile);
}

void ExportDialog::loadData(const QString &language)
{
    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << tr("content"));
    connect(model, &QStandardItemModel::itemChanged, this, &ExportDialog::treeItemChanged);
    connect(model, &QStandardItemModel::itemChanged, this, [this] {
        m_exportBtn->setEnabled(hasChildItemChecked());
    });
    m_rootItems.clear();
    m_childItems.clear();

    const auto &apps = applications();
    for (auto app : apps) {
        if (resourcesForApp(app).isEmpty()) {
            continue;
        }
        DStandardItem *rootItem = new DStandardItem(app);
        rootItem->setCheckable(true);
        rootItem->setSizeHint(QSize(200, 45));
        model->appendRow(rootItem);
        m_rootItems.append(rootItem);

        const auto &resources = resourcesForApp(app);
        for (auto resource : resources) {
            auto resourceItem = new DStandardItem();
            resourceItem->setSizeHint(QSize(200, 45));
            resourceItem->setCheckable(true);
            resourceItem->setText(resource);
            rootItem->appendRow(resourceItem);

            // 添加默认路径 key-value
            QString subpath;
            m_getter.reset(new ValueHandler(app, resource, subpath));
            QScopedPointer<ConfigGetter> manager(m_getter->createManager());
            if (!manager) {
                continue;
            }
            for (auto key : manager->keyList()) {
                auto keyValueItem = new DStandardItem();
                keyValueItem->setCheckable(true);

                keyValueItem->setData(app, AppidRole);
                keyValueItem->setData(resource, ResourceRole);
                keyValueItem->setData(subpath, SubpathRole);
                QVariant value =  manager.get()->value(key);
                keyValueItem->setData(key, KeyRole);
                keyValueItem->setData(value, ValueRole);
                QVariant description = manager.get()->description(key, language);
                keyValueItem->setData(description, DescriptionRole);
                keyValueItem->setText(key + ": " + value.toString());

                resourceItem->appendRow(keyValueItem);
                m_childItems.append(keyValueItem);
            }

            // 添加子路径 key-value
            const auto &subpaths = subpathsForResource(app, resource);
            for (auto subpath : subpaths) {
                auto subpathItem = new DStandardItem();
                subpathItem->setCheckable(true);
                subpathItem->setText(subpath);
                rootItem->appendRow(subpathItem);

                m_getter.reset(new ValueHandler(app, resource, subpath));
                QScopedPointer<ConfigGetter> manager(m_getter->createManager());
                if (!manager) {
                    continue;
                }
                for (auto key : manager->keyList()) {
                    auto keyValueItem = new DStandardItem();
                    keyValueItem->setCheckable(true);

                    keyValueItem->setData(app, AppidRole);
                    keyValueItem->setData(resource, ResourceRole);
                    keyValueItem->setData(subpath, SubpathRole);
                    QVariant value =  manager.get()->value(key);
                    keyValueItem->setData(key, KeyRole);
                    keyValueItem->setData(value, ValueRole);
                    QVariant description = manager.get()->description(key, language);
                    keyValueItem->setData(description, DescriptionRole);
                    keyValueItem->setText(key + ": " + value.toString());

                    subpathItem->appendRow(keyValueItem);
                    m_childItems.append(keyValueItem);
                }
            }
        }
        m_exportView->setModel(model);
    }
}

void ExportDialog::treeItemChanged(QStandardItem *item)
{
    if (item == nullptr)
        return ;
    if (item->isCheckable()) {
        Qt::CheckState state = item->checkState();
        if (state != Qt::PartiallyChecked) {
            checkAllChild(item , state == Qt::Checked ? true : false );
        }
    }
}

bool ExportDialog::hasChildItemChecked()
{
    bool isItemChecked = false;
    for (auto item : m_childItems) {
        if (item->isCheckable() && item->checkState() == Qt::Checked) {
            isItemChecked = true;
            break;
        }
    }
    return isItemChecked;
}

void ExportDialog::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("export current configuration"), "", tr("file(*.csv)"));
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        auto datas = exportData();
        for (auto data : datas) {
            stream << data.join(',');
            stream << "\n";
        }
        stream.flush();
        file.close();
        if (stream.status() == QTextStream::Ok) {
            this->close();
        } else {
            qWarning() << "stream.status:" << stream.status();
            DDialog dialog(tr("save failed"), "",this);
            dialog.addButton(tr("ok"), true, ButtonWarning);
            dialog.exec();
        }
    }
}

void ExportDialog::checkAllChild(QStandardItem * item, bool check)
{
    if (item == nullptr)
        return;
    int rowCount = item->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QStandardItem *childItem = item->child(i);
        checkAllChild(childItem, check);
    }
    if (item->isCheckable())
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
}

QList<QStringList> ExportDialog::exportData()
{
    QList<QStringList> datas;
    datas << (QStringList() << "appid" << "resource" << "subpath" << "key" << "value" <<"description" << "set command" <<  "get command");
    for (auto item : m_childItems) {
        if (item->checkState() == Qt::Checked) {
            QStringList data;
            data << item->data(AppidRole).toString();
            data << item->data(ResourceRole).toString();
            data << item->data(SubpathRole).toString();
            data << item->data(KeyRole).toString();
            data << item->data(ValueRole).toString();
            data << item->data(DescriptionRole).toString();
            QString setCmd = QString("dde-dconfig set %1 -r %2 %3 -v %4").
                             arg(item->data(AppidRole).toString()).
                             arg(item->data(ResourceRole).toString()).
                             arg(item->data(KeyRole).toString()).
                             arg(item->data(ValueRole).toString());
            QString getCmd = QString("dde-dconfig get %1 -r %2 %3").
                             arg(item->data(AppidRole).toString()).
                             arg(item->data(ResourceRole).toString()).
                             arg(item->data(KeyRole).toString());
            if (!item->data(SubpathRole).toString().isEmpty()) {
                setCmd.append(QString(" -s %1").arg(item->data(ConfigUserRole + 4).toString()));
                getCmd.append(QString(" -s %1").arg(item->data(ConfigUserRole + 4).toString()));
            }
            data << setCmd << getCmd;
            datas.append(data);
        }
    }
    return  datas;
}
