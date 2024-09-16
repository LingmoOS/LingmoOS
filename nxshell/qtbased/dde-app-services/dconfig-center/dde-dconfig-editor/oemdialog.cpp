// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "oemdialog.h"
#include "helper.hpp"
#include "valuehandler.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QTreeView>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLabel>
#include <QHeaderView>
#include <QTableView>
#include <DLineEdit>
#include <DSwitchButton>
#include <DSpinBox>

OEMDialog::OEMDialog(QWidget *parent)
    : DDialog( parent)
{
    m_exportView = new QTreeView();
    m_exportView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_exportView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_exportBtn = new DSuggestButton(tr("OEM"));
    m_exportBtn->setEnabled(false);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addStretch(1);
    hLayout->addWidget(m_exportBtn);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_exportView);
    vLayout->addLayout(hLayout);

    QWidget *widget = new QWidget;
    widget->setLayout(vLayout);

    insertContent(0, widget);

    setOnButtonClickedClose(false);
    connect(m_exportBtn, &DSuggestButton::clicked, this, &OEMDialog::displayChangedResult);
}

void OEMDialog::loadData(const QString &language)
{
    m_overrides.clear();

    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << QStringLiteral("项") << QStringLiteral("值"));
    m_exportView->setModel(m_model);

    const auto &apps = applications();
    for (auto app : apps) {
        if (resourcesForApp(app).isEmpty()) {
            continue;
        }
        DStandardItem *rootItem = new DStandardItem(app);
        rootItem->setSizeHint(QSize(200, 45));
        rootItem->setEditable(false);
        rootItem->setData(app, AppidRole);
        m_model->appendRow(rootItem);

        const auto &resources = resourcesForApp(app);
        for (auto resource : resources) {
            auto resourceItem = new DStandardItem();
            resourceItem->setSizeHint(QSize(200, 45));
            resourceItem->setText(resource);
            resourceItem->setEditable(false);
            resourceItem->setData(resource, ResourceRole);
            rootItem->appendRow(resourceItem);

            // 添加默认路径 key-value
            QString subpath;
            m_getter.reset(new ValueHandler(app, resource, subpath));
            QScopedPointer<ConfigGetter> manager(m_getter->createManager());
            if (!manager) {
                continue;
            }
            for (auto key : manager->keyList()) {
                auto keyItem = new DStandardItem();
                keyItem->setText(key);
                keyItem->setEditable(false);

                QVariant value =  manager.get()->value(key);
                QVariant description = manager.get()->description(key, language);
                QVariant flags = manager.get()->flags(key);
                auto valueItem = new DStandardItem();
                valueItem->setSizeHint(QSize(200, 45));
                valueItem->setData(app, AppidRole);
                valueItem->setData(resource, ResourceRole);
                valueItem->setData(subpath, SubpathRole);
                valueItem->setData(key, KeyRole);
                valueItem->setData(value, ValueRole);
                valueItem->setData(description, DescriptionRole);
                valueItem->setData(flags, FlagsRole);
                resourceItem->appendRow(QList<QStandardItem*>() << keyItem << valueItem);
                m_exportView->setIndexWidget(valueItem->index(), getItemWidget(manager.get(), valueItem));
            }

            // 添加子路径 key-value
            const auto &subpaths = subpathsForResource(app, resource);
            for (auto subpath : subpaths) {
                auto subpathItem = new DStandardItem();
                subpathItem->setText(subpath);
                subpathItem->setEditable(false);
                subpathItem->setData(subpath, SubpathRole);
                rootItem->appendRow(subpathItem);

                m_getter.reset(new ValueHandler(app, resource, subpath));
                QScopedPointer<ConfigGetter> manager(m_getter->createManager());
                if (!manager) {
                    continue;
                }
                for (auto key : manager->keyList()) {
                    auto keyItem = new DStandardItem();
                    keyItem->setText(key);
                    keyItem->setEditable(false);

                    QVariant value =  manager.get()->value(key);
                    QVariant description = manager.get()->description(key, language);
                    auto valueItem = new DStandardItem(value.toString());
                    QVariant flags = manager.get()->flags(key);
                    valueItem->setData(app, AppidRole);
                    valueItem->setData(resource, ResourceRole);
                    valueItem->setData(subpath, SubpathRole);
                    valueItem->setData(key, KeyRole);
                    valueItem->setData(value, ValueRole);
                    valueItem->setData(description, DescriptionRole);
                    valueItem->setData(flags, FlagsRole);

                    subpathItem->appendRow(QList<QStandardItem*>() << keyItem << valueItem);
                    m_exportView->setIndexWidget(valueItem->index(), getItemWidget(manager.get(), valueItem));
                }
            }
        }
    }

}

void OEMDialog::treeItemChanged(DStandardItem *valueItem)
{
    m_exportBtn->setEnabled(true);

    auto keyIndex = valueItem->index().siblingAtColumn(0);
    auto siblingItem = m_model->itemFromIndex(keyIndex);
    QFont sFont = siblingItem->font();
    sFont.setBold(true);
    siblingItem->setFont(sFont);
    siblingItem->setText(QString("* ") + valueItem->data(KeyRole).toString());

    auto parentItem = valueItem->parent();
    QFont pFont = parentItem->font();
    pFont.setBold(true);
    parentItem->setFont(pFont);
    auto pValue = !parentItem->data(SubpathRole).toString().isEmpty() ?
                parentItem->data(SubpathRole).toString() :
                parentItem->data(ResourceRole).toString();
    parentItem->setText(QString("* ") + pValue);

    auto rootItem = parentItem->parent();
    QFont rFont = rootItem->font();
    rFont.setBold(true);
    rootItem->setFont(pFont);
    rootItem->setText(QString("* ") + valueItem->data(AppidRole).toString());

    QString key = !valueItem->data(SubpathRole).toString().isEmpty() ? valueItem->data(SubpathRole).toString() :
                                                                       valueItem->data(ResourceRole).toString();
    if (m_overrides.find(key) != m_overrides.end()) {
        if (!m_overrides[key].contains(valueItem))
            m_overrides[key].append(valueItem);
    } else {
        QList<DStandardItem *> datas;
        datas.append(valueItem);
        m_overrides.insert(key, datas);
    }
}

void OEMDialog::saveFiles()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("export directory"), QDir::homePath(),
                                                        QFileDialog::ShowDirsOnly |
                                                        QFileDialog::DontResolveSymlinks);
    if (dirName.isEmpty())
        return;
    saveOverridesFiles(dirName);
    saveCSVFile(dirName);
}
void OEMDialog::saveOverridesFiles(const QString &dirName)
{
    for (auto override : m_overrides) {
        QString fileName = m_overrides.key(override)+"." + "override" + ".json";
        createJsonFile(dirName + "/" + fileName, override);
    }
}

void OEMDialog::saveCSVFile(const QString &dirName)
{
    QString fileName = dirName + "/" + "data.csv";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "appid," << "resource," << "key," << "value," << "path" << "\n";
        for (auto items : m_overrides) {
            for (auto item : items) {
                QString path = QString("/etc/dsg/configs/overrides/%1/%2").arg(item->data(AppidRole).toString())
                                                                          .arg(item->data(ResourceRole).toString());
                if (!item->data(SubpathRole).toString().isEmpty()) {
                    path.append(item->data(SubpathRole).toString());
                }
                QString content = QString("%1, %2, %3, %4, %5").arg(item->data(AppidRole).toString())
                                                     .arg(item->data(ResourceRole).toString())
                                                     .arg(item->data(KeyRole).toString())
                                                     .arg(item->data(ValueRole).toString())
                                                     .arg(path);
                out << content << "\n";
            }
        }
        out.flush();
        file.close();
        if (out.status() == QTextStream::Ok) {
            m_overrides.clear();
            m_exportBtn->setEnabled(false);
            this->close();
        } else {
            qWarning() << "stream.status:" << out.status();
            DDialog dialog(tr("save filed"), "",this);
            dialog.addButton(tr("ok"), true, ButtonWarning);
            dialog.exec();
        }
    }
}


void OEMDialog::displayChangedResult()
{
    DDialog *d = new DDialog;
    d->setAttribute(Qt::WA_DeleteOnClose, true);

    QTableView *view = new QTableView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->horizontalHeader()->setStretchLastSection(true);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << "appid" << "resource" << "key" << "value" << "path");
    view->setModel(model);

    for (auto items : m_overrides) {
        for (auto item : items) {
            QString path = QString("/etc/dsg/configs/overrides/%1/%2").arg(item->data(AppidRole).toString())
                                                                      .arg(item->data(ResourceRole).toString());
            if (!item->data(SubpathRole).toString().isEmpty()) {
                path.append(item->data(SubpathRole).toString());
            }
            model->appendRow(QList<QStandardItem*>() << new DStandardItem(item->data(AppidRole).toString())
                             << new DStandardItem(item->data(ResourceRole).toString())
                             << new DStandardItem(item->data(KeyRole).toString())
                             << new DStandardItem(item->data(ValueRole).toString())
                             << new DStandardItem(path));
        }
    }
    DSuggestButton *btn = new DSuggestButton(tr("ok"));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch(1);
    hlayout->addSpacing(20);
    hlayout->addWidget(btn);

    QWidget *widget = new QWidget;
    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    vlayout->addWidget(view);
    vlayout->addLayout(hlayout);
    widget->setMinimumSize(600, 300);
    d->addContent(widget);

    connect(btn, &QPushButton::clicked, this, [this, d]() {
        d->close();
        saveFiles();
    });
    d->exec();
}

void OEMDialog::closeEvent(QCloseEvent *event)
{
    if (m_overrides.isEmpty()) {
        event->accept();
        return;
    }
    DDialog dialog(tr("file doesn't save"), "",this);
    dialog.addButton(tr("cancle"), true, ButtonWarning);
    dialog.addButton(tr("ok"), true, ButtonWarning);
    connect(dialog.getButton(0), &QPushButton::clicked, this, [event]{
        event->ignore();
    });
    connect(dialog.getButton(1), &QPushButton::clicked, this, [event]{
        event->accept();
    });
    int i = dialog.exec();
    if (i == -1)
        event->ignore();
}



void OEMDialog::createJsonFile(const QString &fileName, const QList<DStandardItem *> &items)
{
    qWarning() << fileName;
    QJsonObject rootObject, object, contentsObject, keyObject;
    for (auto item : items) {
        keyObject.insert("value", QJsonValue::fromVariant(item->data(ValueRole)));
        keyObject.insert("serial", 0);
        keyObject.insert("permissions", "readwrite");
        contentsObject.insert(item->data(KeyRole).toString(), keyObject);
    }
    object.insert("contents", contentsObject);
    object.insert("magic", "dsg.config.override");
    object.insert("version", "1.0");

    QJsonDocument jsonDocument;
    jsonDocument.setObject(object);

    QByteArray byteArray = jsonDocument.toJson(QJsonDocument::Indented);

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << QString("fail to open the file: %1, %2, %3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__);
        return;
    }
    QTextStream out(&file);
    out << byteArray;

    file.close();
    if (out.status() != QTextStream::Ok) {
        qWarning() << "stream.status:" << out.status();
        DDialog dialog("save failed", "",this);
        dialog.addButton("ok", true, ButtonWarning);
        dialog.exec();
    }
    qDebug() << byteArray;
}

QWidget *OEMDialog::getItemWidget(ConfigGetter *getter, DStandardItem *item)
{
    const int flags = getter->flags(item->data(KeyRole).toString());
    bool canWrite = !(flags & Dtk::Core::DConfigFile::Flag::NoOverride);
    QVariant v = item->data(ValueRole);
    QWidget *valueWidget = nullptr;
    if (v.type() == QVariant::Bool) {
        auto btn = new DSwitchButton();
        btn->setChecked(v.toBool());
        btn->setEnabled(canWrite);
        connect(btn, &DSwitchButton::checkedChanged, btn, [this, btn, item](bool checked){
            btn->clearFocus();
            item->setData(checked, ValueRole);
            treeItemChanged(item);
        });

        QWidget *widget = new QWidget;
        QHBoxLayout *hlayout = new QHBoxLayout(widget);
        hlayout->addStretch(1);
        hlayout->addWidget(btn);

        valueWidget = widget;
    } else if (v.type() == QVariant::Double) {
        auto widget = new DDoubleSpinBox();
        widget->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
        widget->setValue(v.toDouble());
        widget->setEnabled(canWrite);
        connect(widget,  qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, item](double data){
            item->setData(data, ValueRole);
            treeItemChanged(item);
        });
        valueWidget = widget;
    } else {
        auto widget = new DLineEdit();
        widget->setText(v.toString());
        widget->setEnabled(canWrite);
        connect(widget, &DLineEdit::textChanged, widget, [this, item](const QString &text){
            item->setData(text, ValueRole);
            treeItemChanged(item);
        });
        valueWidget = widget;
    }
    return valueWidget;
}
