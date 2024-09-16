// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "customwidget.h"
#include "uitools.h"

#include <DPushButton>
#include <DApplicationHelper>
#include <DSettingsWidgetFactory>
#include <DFileDialog>
#include <DLabel>
#include <DStandardPaths>

#include <QDebug>
#include <QLayout>
#include <QFileInfo>
#include <qsettingbackend.h>

DGUI_USE_NAMESPACE

SettingDialog::SettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    initUI();
    initConnections();
}

SettingDialog::~SettingDialog()
{
    SAFE_DELETE_ELE(m_settings);
}

QString SettingDialog::getDefaultExtractPath()
{
    return m_curpath;
}

bool SettingDialog::isAutoCreatDir()
{
    // 读取配置文件信息 是否自动创建文件夹
    return m_settings->value("base.decompress.create_folder").toBool();
}

bool SettingDialog::isAutoOpen()
{
    // 读取配置文件信息 是否自动打开文件夹
    return m_settings->value("base.decompress.open_folder").toBool();
}

bool SettingDialog::isAutoDeleteFile()
{
    // 读取配置文件信息 压缩后是否删除源文件
    return m_settings->value("base.file_management.delete_file").toBool();
}

QString SettingDialog::isAutoDeleteArchive()
{
    // 读取配置文件信息 解压后是否删除压缩文件
    return m_deleteArchiveOption->value().toString();
}

bool SettingDialog::isAssociatedType(QString mime) // 暂时未被调用
{
    // 读取配置文件信息 是否是解压缩关联文件类型
    return m_settings->option("file_association.file_association_type." + mime.remove("application/"))->value().toBool();
}

void SettingDialog::initUI()
{
    createSettingButton(); // 创建选择按钮
    createPathBox(); // 创建默认解压路径
    createDeleteBox(); // 创建删除选项

    // 通过json文件创建DSettings对象
    m_settings = DSettings::fromJsonFile(":assets/data/deepin-compressor.json");

    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); // 换了枚举值，待验证

    const QString confPath = confDir + QDir::separator() + "deepin-compressor.conf";

    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath, this);
    m_settings->setBackend(backend);

    // 通过DSettings对象构建设置界面
    updateSettings(m_settings);

    writeConfbf();  // 在应用被打开的时候，写新的config文件 具体功能待验证
}

void SettingDialog::initConnections()
{
    //m_setting需要setBackend,该信号才会有效
    connect(m_settings, &DSettings::valueChanged, this, &SettingDialog::slotSettingsChanged);
}

void SettingDialog::createSettingButton()
{
    this->widgetFactory()->registerWidget("custom-button", [this](QObject * obj) -> QWidget* {
        m_customButtonOption = qobject_cast<DSettingsOption *>(obj);
        if (m_customButtonOption)
        {
            QWidget *buttonwidget = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout();
            CustomPushButton *selectAllButton = new CustomPushButton(tr("Select All", "button"));
            CustomPushButton *cancelSelectButton = new CustomPushButton(tr("Clear All"));
            CustomSuggestButton *recommendedButton = new CustomSuggestButton(tr("Recommended"));
            selectAllButton->setMinimumSize(153, 36);
            cancelSelectButton->setMinimumSize(153, 36);
            recommendedButton->setMinimumSize(153, 36);
            layout->addStretch();
            layout->addWidget(selectAllButton);
            layout->addStretch();
            layout->addWidget(cancelSelectButton);
            layout->addStretch();
            layout->addWidget(recommendedButton);
            layout->addStretch();
            buttonwidget->setLayout(layout);

            connect(selectAllButton, &QPushButton::clicked, this, &SettingDialog::slotClickSelectAllButton);
            connect(cancelSelectButton, &QPushButton::clicked, this, &SettingDialog::slotClickCancelSelectAllButton);
            connect(recommendedButton, &QPushButton::clicked, this, &SettingDialog::slotClickRecommendedButton);
            return buttonwidget;
        }

        return nullptr;
    });
}

void SettingDialog::createPathBox()
{
    this->widgetFactory()->registerWidget("pathbox", [this](QObject * obj) -> QWidget* {
        m_extractPathOption = qobject_cast<DSettingsOption *>(obj);
        if (m_extractPathOption)
        {
            DWidget *widget = new DWidget(this);
            QHBoxLayout *layout = new QHBoxLayout();

            DLabel *label = new DLabel(widget);
            label->setForegroundRole(DPalette::WindowText);
            label->setText(tr("Extract archives to") + ":");

            CustomCombobox *combobox = new CustomCombobox(widget);
            combobox->setMinimumWidth(300);
            combobox->setEditable(false);
            QStringList list;
            list << tr("Current directory") << tr("Desktop") << tr("Other directory");
            combobox->addItems(list);

            if (QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) == m_extractPathOption->value()) {
                combobox->setCurrentIndex(1);
                m_curpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                m_index_last = 1;
            } else if ("" == m_extractPathOption->value()) {
                combobox->setCurrentIndex(0);
                m_curpath = "";
                m_index_last = 0;
            } else {
                combobox->setEditable(true);
                combobox->setCurrentIndex(2);
                m_curpath = m_extractPathOption->value().toString();
                combobox->setEditText(m_curpath);
                m_index_last = 2;
            }

            layout->addWidget(label, 0, Qt::AlignLeft);
            layout->addWidget(combobox, 0, Qt::AlignLeft);

            widget->setLayout(layout);

            connect(this, &SettingDialog::sigResetPath, this, [ = ] {
                combobox->setCurrentIndex(0);
            });

            connect(combobox, &DComboBox::currentTextChanged, [combobox, this] {
                if (tr("Current directory") == combobox->currentText())
                {
                    combobox->setEditable(false);
                    m_curpath = "";
                    m_index_last = 0;
                } else if (tr("Desktop") == combobox->currentText())
                {
                    combobox->setEditable(false);
                    m_curpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                    m_index_last = 1;
                } else if (tr("Other directory") == combobox->currentText())
                {
                    combobox->setEditable(true);
                    DFileDialog dialog(this);
                    dialog.setAcceptMode(DFileDialog::AcceptOpen);
                    dialog.setFileMode(DFileDialog::Directory);
                    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

                    const int mode = dialog.exec();

                    if (mode != QDialog::Accepted) {
                        if (0 == m_index_last) {
                            combobox->setEditable(false);
                            combobox->setCurrentIndex(0);
                        } else if (1 == m_index_last) {
                            combobox->setEditable(false);
                            combobox->setCurrentIndex(1);
                        } else {
                            combobox->setEditText(m_curpath);
                        }
                        return;
                    }

                    QList<QUrl> pathlist = dialog.selectedUrls();
                    QString curpath = pathlist.at(0).toLocalFile();

                    combobox->setEditText(curpath);
                    m_curpath = curpath;
                    m_index_last = 2;
                } else
                {
                    m_curpath = combobox->currentText();
                    QDir dir(m_curpath);
                    DPalette plt = DApplicationHelper::instance()->palette(combobox);

                    if (!dir.exists()) {
                        plt.setBrush(DPalette::Text, plt.color(DPalette::TextWarning));
                    } else {
                        plt.setBrush(DPalette::Text, plt.color(DPalette::WindowText));
                    }

                    combobox->setPalette(plt);
                    m_index_last = 2;
                }

                m_extractPathOption->setValue(m_curpath);
            });

            qInfo() << m_curpath;
            return widget;
        }

        return nullptr;
    });
}

void SettingDialog::createDeleteBox()
{
    this->widgetFactory()->registerWidget("deletebox", [this](QObject * obj) -> QWidget* {
        m_deleteArchiveOption = qobject_cast<DSettingsOption *>(obj);
        if (m_deleteArchiveOption)
        {
            DWidget *widget = new DWidget(this);
            QHBoxLayout *layout = new QHBoxLayout();

            DLabel *label = new DLabel(widget);
            label->setForegroundRole(DPalette::WindowText);
            label->setText(tr("Delete archives after extraction") + ":");

            CustomCombobox *combobox = new CustomCombobox(widget);
            combobox->setMinimumWidth(300);
            combobox->setEditable(false);
            QStringList list;
            list << tr("Never") << tr("Ask for confirmation") << tr("Always");
            combobox->addItems(list);

            if ("Always" == m_deleteArchiveOption->value()) {
                combobox->setCurrentIndex(2);
            } else if ("Ask for confirmation" == m_deleteArchiveOption->value()) {
                combobox->setCurrentIndex(1);
            } else {
                combobox->setCurrentIndex(0);
            }

            layout->addWidget(label, 0, Qt::AlignLeft);
            layout->addWidget(combobox, 0, Qt::AlignLeft);

            widget->setLayout(layout);

            connect(this, &SettingDialog::sigResetDeleteArchive, this, [ = ] {
                combobox->setCurrentIndex(0);
            });

            connect(combobox, &DComboBox::currentTextChanged, [combobox, this] {
                if (tr("Never") == combobox->currentText())
                {
                    m_autoDeleteArchive = "Never";
                } else if (tr("Ask for confirmation") == combobox->currentText())
                {
                    m_autoDeleteArchive = "Ask for confirmation";
                } else if (tr("Always") == combobox->currentText())
                {
                    m_autoDeleteArchive = "Always";
                } else
                {
                    m_autoDeleteArchive = combobox->currentText();
                }

                m_deleteArchiveOption->setValue(m_autoDeleteArchive);
            });

            return widget;
        }

        return nullptr;
    });
}

void SettingDialog::writeConfbf()
{
    // 设置配置文件的绝对路径
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QString confPath = confDir + QDir::separator() + "deepin-compressor.confbf";

    // 打开配置文件
    QFile file(confPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    // 写配置
    foreach (QString key, UiTools::m_associtionlist) {
        QString bValue = m_settings->option(key)->value().toString();
        QString content = key + ":" + bValue + "\n";
        file.write(content.toUtf8());
    }

    file.close();
}

void SettingDialog::slotSettingsChanged(const QString &key, const QVariant &value)
{
    // 设置界面点击恢复默认选项  具体功能待确认、完善
    qInfo() << "slotSettingsChanged:  " << key  << value;
    writeConfbf();
}

void SettingDialog::slotClickSelectAllButton()
{
    foreach (QString key, UiTools::m_associtionlist) {
        m_settings->setOption(key, true);
    }
}

void SettingDialog::slotClickCancelSelectAllButton()
{
    foreach (QString key, UiTools::m_associtionlist) {
        m_settings->setOption(key, false);
    }
}

void SettingDialog::slotClickRecommendedButton()
{
    foreach (QString key, UiTools::m_associtionlist) {
        if ("file_association.file_association_type.x-iso9660-image" == key
                || "file_association.file_association_type.x-iso9660-appimage" == key
                || "file_association.file_association_type.x-source-rpm" == key) {
            m_settings->setOption(key, false);
        } else {
            m_settings->setOption(key, true);
        }
    }
}
