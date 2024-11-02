/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: WenJie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "extensions-manager-widget.h"
#include "plugin-manager.h"
#include "global-settings.h"

#include <QIcon>
#include <QHeaderView>
#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <QHBoxLayout>

static Peony::ExtensionsManagerWidget *global_instance = nullptr;

Peony::ExtensionsManagerWidget *Peony::ExtensionsManagerWidget::getInstance()
{
    if (!global_instance) {
        global_instance = new Peony::ExtensionsManagerWidget;
    }
    return global_instance;
}

Peony::ExtensionsManagerWidget::ExtensionsManagerWidget(QWidget *parent)
    :QWidget(parent)
{
    this->initUI();
}

Peony::ExtensionsManagerWidget::~ExtensionsManagerWidget()
{
    for (auto plugin : m_pluginMap) {
        delete plugin;
    }
    m_pluginMap.clear();
}

void Peony::ExtensionsManagerWidget::initUI()
{
    setAutoFillBackground(true);
    this->setContentsMargins(0, 0, 0, 0);
    this->resize(EXTENSIONS_SHOW_WIDTH, EXTENSIONS_SHOW_HEIGHT);
    this->setMinimumWidth(EXTENSIONS_SHOW_WIDTH);
    this->setWindowTitle(tr("Extensions Manager"));
    this->setWindowIcon(QIcon::fromTheme("system-file-manager"));

    m_mainLayout = new QVBoxLayout(this);
    m_useLabel = new QLabel(tr("Available extensions"));
    m_useLabel->setContentsMargins(20, 10, 20, 10);
    m_okBtn = new QPushButton(tr("Ok"));
    m_cancelBtn = new QPushButton(tr("Cancel"));
    m_mainLayout->addWidget(m_useLabel);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(22, 0, 22, 0);
    hBoxLayout->setSpacing(0);
    hBoxLayout->addStretch(1);
    hBoxLayout->addWidget(m_cancelBtn);
    hBoxLayout->addSpacing(16);
    hBoxLayout->addWidget(m_okBtn);

    this->initExtensionInfo();
    this->initTableWidget();
    this->addSeparator();

    m_mainLayout->addLayout(hBoxLayout);

    connect(m_okBtn, &QPushButton::clicked, this, [=](){
        QStringList extensions;
        for (int row = 0; row < m_tableWidget->rowCount(); row++) {
            QWidget *w = m_tableWidget->cellWidget(row, 0);
            QCheckBox *box = w->findChild<QCheckBox*>();
            QString filePath = box->property("filePath").toString();
            //qDebug() << "========filePath:" << filePath;
            if (!box->isChecked()) {
                QFileInfo fileInfo(filePath);
                if (fileInfo.exists()) {
                    extensions.append(fileInfo.fileName());
                }
            }
        }

        GlobalSettings::getInstance()->setGSettingValue(DISABLED_EXTENSIONS, extensions);
        this->close();
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [=](){
        this->close();
        if (global_instance) {
            delete global_instance;
            global_instance = nullptr;
        }
    });
}

void Peony::ExtensionsManagerWidget::initTableWidget()
{
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setVisible(false);
    m_tableWidget->verticalHeader()->setMinimumSectionSize(12);
    m_tableWidget->horizontalHeader()->setFrameShape(QFrame::NoFrame);
    m_tableWidget->setFrameShape(QFrame::NoFrame);
    m_tableWidget->horizontalHeader()->setSelectionMode(QTableWidget::NoSelection);
    m_tableWidget->verticalHeader()->setSelectionMode(QTableWidget::NoSelection);
    m_tableWidget->setSelectionMode(QTableWidget::SingleSelection);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableWidget->setShowGrid(false);

    m_tableWidget->horizontalHeader()->setMinimumHeight(54);
    m_tableWidget->rowHeight(54);

    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    m_tableWidget->horizontalHeader()->setMinimumSectionSize(30);
    m_tableWidget->horizontalHeader()->setMaximumSectionSize(300);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(46);

    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    int count = m_pluginMap.count();
    m_tableWidget->setRowCount(count);

    QMap<QString, PluginInterface*>::iterator iter;
    int row = 0;
    for (iter = m_pluginMap.begin();  iter != m_pluginMap.end(); ++iter) {
        QString filePath = iter.key();
        PluginInterface* iface = iter.value();

        //Add checkBox
        m_tableWidget->setCellWidget(row, 0, nullptr);
        QWidget *w = new QWidget(m_tableWidget);
        QHBoxLayout *l = new QHBoxLayout(w);
        l->setMargin(0);
        w->setLayout(l);
        l->setAlignment(Qt::AlignCenter);
        auto checkBox = new QCheckBox(w);
        bool check = updateCheckBox(filePath);
        checkBox->setChecked(check);
        checkBox->setProperty("filePath", filePath);
        l->addWidget(checkBox);
        m_tableWidget->setCellWidget(row, 0, w);

        QIcon icon = iface->icon();
        QLabel *iconLabel = new QLabel();
        iconLabel->setProperty("useIconHighlightEffect", 0x2);
        iconLabel->setPixmap(QIcon::fromTheme(icon.name(), QIcon::fromTheme("unknown")).pixmap(QSize(24, 24)));
        iconLabel->setAlignment(Qt::AlignCenter);
        m_tableWidget->setCellWidget(row, 1, iconLabel);

        QTableWidgetItem* itemC2 = new QTableWidgetItem(iface->description());
        itemC2->setFlags(itemC2->flags() | Qt::ItemIsSelectable);
        itemC2->setToolTip(iface->description());
        m_tableWidget->setItem(row, 2, itemC2);
        row++;
    }

    m_mainLayout->addWidget(m_tableWidget);
}

void Peony::ExtensionsManagerWidget::initExtensionInfo()
{
    if (GlobalSettings::getInstance()->isExist(DISABLED_EXTENSIONS)) {
        m_disabledList = GlobalSettings::getInstance()->getValue(DISABLED_EXTENSIONS).toStringList();
    }

    qDebug() << __func__ << __LINE__ << m_disabledList;


    QDir pluginsDir(PLUGIN_INSTALL_DIRS);
    pluginsDir.setFilter(QDir::Files);

    Q_FOREACH(QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        qDebug()<<pluginLoader.fileName();
        qDebug()<<pluginLoader.metaData();
        qDebug()<<pluginLoader.load();

        if (pluginLoader.metaData().value("MetaData").toObject().value("version").toString() != VERSION)
            continue;

        QObject *plugin = pluginLoader.instance();
        if (!plugin)
            continue;
        PluginInterface *piface = dynamic_cast<PluginInterface*>(plugin);
        if (!piface)
            continue;
        m_pluginMap.insert(pluginLoader.fileName(), piface);

        QFileInfo fileInfo(pluginLoader.fileName());
        if (fileInfo.exists()) {

        }
    }

    qDebug() << __func__ << __LINE__ << m_pluginMap.keys();
}

bool Peony::ExtensionsManagerWidget::updateCheckBox(const QString &path)
{
    bool enable = true;
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        if (m_disabledList.contains(fileInfo.fileName())) {
            enable = false;
        }
    }
    return enable;
}

void Peony::ExtensionsManagerWidget::addSeparator()
{
    QPushButton *separate = new QPushButton;
    separate->setFixedHeight(1);
    separate->setFocusPolicy(Qt::NoFocus);
    separate->setEnabled(false);
    m_mainLayout->addWidget(separate);
}
