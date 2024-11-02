/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "menu-plugin-manager.h"
#include "file-info.h"

//create link
#include "shared-file-link-operation.h"
#include <file-operation-manager.h>
#include <file-link-operation.h>
#include <file-info.h>
#include <gio/gio.h>

#include <QStandardPaths>
#include <QFileDialog>
#include <QApplication>
//create link

//tag file
#include "file-label-model.h"
#include "tag-management.h"
#include <QMenu>

#include <QAction>
#include <QIcon>

#include <QDebug>
#include <QWidgetAction>
#include <QToolButton>
#include <QButtonGroup>

#include <QMainWindow>

#include "color-pushbutton.h"
#include <QRadioButton>
using namespace Peony;

static MenuPluginManager *global_instance = nullptr;

MenuPluginManager::MenuPluginManager(QObject *parent) : QObject(parent)
{
    registerPlugin(new CreateLinkInternalPlugin(this));
    registerPlugin(new FileLabelInternalMenuPlugin(this));
    registerPlugin(new CreateSharedFileLinkMenuPlugin(this));
}

MenuPluginManager::~MenuPluginManager()
{

}

bool MenuPluginManager::registerPlugin(MenuPluginInterface *plugin)
{
    if (m_hash.value(plugin->name())) {
        return false;
    }
    m_hash.insert(plugin->name(), plugin);
    return true;
}

bool MenuPluginManager::unregisterPlugin(MenuPluginInterface *plugin)
{
    if (m_hash.value(plugin->name())) {
        m_hash.remove(plugin->name());
        return true;
    }
    return false;
}

MenuPluginManager *MenuPluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new MenuPluginManager;
    }
    return global_instance;
}

void MenuPluginManager::close()
{
    this->deleteLater();
}

const QStringList MenuPluginManager::getPluginIds()
{
    return m_hash.keys();
}

MenuPluginInterface *MenuPluginManager::getPlugin(const QString &pluginId)
{
    return m_hash.value(pluginId);
}

//CreateLinkInternalPlugin
CreateLinkInternalPlugin::CreateLinkInternalPlugin(QObject *parent) : QObject (parent)
{

}

QList<QAction *> CreateLinkInternalPlugin::menuActions(MenuPluginInterface::Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction *> l;
    if (types == MenuPluginInterface::DesktopWindow || types == MenuPluginInterface::DirectoryView) {
        if (selectionUris.count() == 1) {
            auto select_file_info = FileInfo::fromUri(selectionUris[0]);
            if(select_file_info->isSymbolLink())
                return l;

            QString str_cmp = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            str_cmp.insert(0, QString("file://"));
            //在桌面文件夹中屏蔽 “发送到桌面快捷方式” 和 “创建链接到...” - Block "Create link to desktop" and "Create link to..." in the desktop folder
            if(QString::compare(QUrl::fromPercentEncoding(uri.toLocal8Bit()), str_cmp))
            {
                auto createLinkToDesktop = new QAction(QIcon::fromTheme("emblem-link-symbolic"), tr("Create Link to Desktop"), nullptr);
                auto info = FileInfo::fromUri(selectionUris.first());
                QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                QString originPath = QUrl(selectionUris.first()).path();
                //special type mountable, or isVirtual then return
                if (selectionUris.first().startsWith("computer:///")
                        || info->isVirtual()
                        || selectionUris.first().startsWith("trash:///")
                        || selectionUris.first().startsWith("recent:///")
                        || selectionUris.first().startsWith("mtp://")
                        || selectionUris.first().startsWith("sftp://")
                        || selectionUris.first().startsWith("ftp://")
                        || selectionUris.first().startsWith("smb://")
                        || originPath == desktopPath) {
                    return l;
                }

                connect(createLinkToDesktop, &QAction::triggered, [=]() {
                    //QUrl src = selectionUris.first();
                    QString desktopUri = "file://" + desktopPath;
                    FileLinkOperation *op = new FileLinkOperation(selectionUris.first(), desktopUri);
                    op->setAutoDelete(true);
                    FileOperationManager::getInstance()->startOperation(op, true);
                });
                l<<createLinkToDesktop;

                auto createLinkTo = new QAction(tr("Create Link to..."), nullptr);
                connect(createLinkTo, &QAction::triggered, [=]() {
                    QUrl targetDir = QFileDialog::getExistingDirectoryUrl(nullptr,
                                                                          tr("Choose a Directory to Create Link"),
                                                                          uri);
                    if (!targetDir.isEmpty()) {
                        //QUrl src = selectionUris.first();
                        QString target = targetDir.url();
                        FileLinkOperation *op = new FileLinkOperation(selectionUris.first(), target);
                        op->setAutoDelete(true);
                        FileOperationManager::getInstance()->startOperation(op, true);
                    }
                });
                l<<createLinkTo;
            }
        }
    }
    return l;
}

//FileLabelInternalMenuPlugin
FileLabelInternalMenuPlugin::FileLabelInternalMenuPlugin(QObject *parent)
{

}

QList<QAction *> FileLabelInternalMenuPlugin::menuActions(MenuPluginInterface::Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction *> l;
    //fix virtual path add label fail issue
    auto info = FileInfo::fromUri(uri);
    if (info->isVirtual())
        return l;
    if (qApp->property("tabletMode").toBool()) {
        return l;
    }
    if (types == DirectoryView) {
        //not allow in trash path
        if (uri.startsWith("trash://") || uri.startsWith("smb://")
            || uri.startsWith("recent://") || uri.startsWith("computer://"))
            return l;
        QString version = qApp->property("version").toString();
        if (version == "lingmo3.0") {
            if (selectionUris.count() == 1) {
                auto action = new QAction(tr("Add File Label"), nullptr);
                auto uri = selectionUris.first();
                auto menu = new QMenu();
                auto items = FileLabelModel::getGlobalModel()->getAllFileLabelItems();
                for (auto item : items) {
                    auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(uri);
                    bool checked = ids.contains(item->id());
                    auto a = menu->addAction(item->name(), [=]() {
                        if (!checked) {
                            // note: while add label to file at first time (usually new user created),
                            // it might fail to add a label correctly, but second time will work.
                            // it might be a bug of gvfsd-metadata. anyway we should to avoid this
                            // situation.
                            FileLabelModel::getGlobalModel()->addLabelToFile(uri, item->id());
                            FileLabelModel::getGlobalModel()->addLabelToFile(uri, item->id());
                        } else {
                            FileLabelModel::getGlobalModel()->removeFileLabel(uri, item->id());
                        }
                    });
                    a->setCheckable(true);
                    a->setChecked(checked);
                }
                menu->addSeparator();
                menu->addAction(tr("Delete All Label"), [=]() {
                    FileLabelModel::getGlobalModel()->removeFileLabel(uri);
                });
                action->setMenu(menu);
                l<<action;
            }
        } else {
            auto labelWidgetContainer = new QWidgetAction(this);
            auto labelWidget = new FileLabelWidget(selectionUris);
            labelWidgetContainer->setDefaultWidget(labelWidget);
            l<<labelWidgetContainer;

            QAction *tagAction = new QAction(tr("label management ..."), this);
            connect(tagAction, &QAction::triggered, this, [=]() {
                TagManagement *managent = TagManagement::getInstance();
                managent->show();
            });
            l<<tagAction;

           connect(labelWidget, &FileLabelWidget::changeText, this, [=](const QString &text) {
            tagAction->setText(text);
           });
        }
    }
    return l;
}

CreateSharedFileLinkMenuPlugin::CreateSharedFileLinkMenuPlugin(QObject *parent) : QObject (parent)
{

}

QList<QAction *> CreateSharedFileLinkMenuPlugin::menuActions(MenuPluginInterface::Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction *> l;
    if (types == MenuPluginInterface::DesktopWindow || types == MenuPluginInterface::DirectoryView) {
        if (selectionUris.count() == 1) {
            auto select_file_info = FileInfo::fromUri(selectionUris[0]);
            if(select_file_info->isSymbolLink())
                return l;

            QString str_cmp = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            str_cmp.insert(0, QString("file://"));
            //在桌面文件夹中屏蔽 “发送到桌面快捷方式” 和 “创建链接到...” - Block "Create link to desktop" and "Create link to..." in the desktop folder
            if(QString::compare(QUrl::fromPercentEncoding(uri.toLocal8Bit()), str_cmp))
            {
                QAction* createLinkToDesktop = new QAction(QIcon::fromTheme("emblem-link-symbolic"), tr("Create Link to Desktop"), nullptr);
                auto info = FileInfo::fromUri(selectionUris.first());
                if (!info->isDir()) {
                    return l;
                }
                QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                QString originPath = QUrl(selectionUris.first()).path();
                //special type mountable, or isVirtual then return
                if ( !selectionUris.first().startsWith("smb://")
                        &&  !selectionUris.first().startsWith("ftp://")
                        &&  !selectionUris.first().startsWith("sftp://") ) {
                    return l;
                }

                connect(createLinkToDesktop, &QAction::triggered, [=]() {
                    //QUrl src = selectionUris.first();
                    QString desktopUri = "file://" + desktopPath;
                    SharedFileLinkOperation *op = new SharedFileLinkOperation(selectionUris.first(), desktopUri);
                    op->setAutoDelete(true);
                    FileOperationManager::getInstance()->startOperation(op, false);
                });
                l<<createLinkToDesktop;

            }
        }
    }
    return l;
}

void FileLabelWidget::clickItem(int index)
{
    m_colorgroup->button(index)->setFocus();
    static_cast<ColorPushButton*>(m_colorgroup->button(index))->m_checkInMultiSelect = true;
    QList<int> updateids;
    updateids = m_ids;
    if(m_selectionUris->count() == 1){
       auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(m_selectionUris->first());
       bool checked = ids.contains(index);
       if (!checked) {
           FileLabelModel::getGlobalModel()->addLabelToFile(m_selectionUris->first(), index);
           FileLabelModel::getGlobalModel()->addLabelToFile(m_selectionUris->first(), index);
       }else{
           FileLabelModel::getGlobalModel()->removeFileLabel(m_selectionUris->first(), index);
       }

    }
    else{
        bool check = true;

        QList<QString>::iterator it = m_selectionUris->begin();
        for (; it < m_selectionUris->end(); it++){
            auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(*it);
            FileLabelModel::getGlobalModel()->getFileLabelIds(*it);
            updateids.append(ids);

            if(m_ids.contains(index)){
                if(!ids.contains(index)){
                    check = false;
                    break;
                }
            }
        }
        m_ids = updateids.toSet().toList();

        it = m_selectionUris->begin();

        for (; it < m_selectionUris->end(); it++){
            if(!m_ids.contains(index)){
                FileLabelModel::getGlobalModel()->addLabelToFile(*it, index);
                FileLabelModel::getGlobalModel()->addLabelToFile(*it, index);
            }else{
                if(check){
                    FileLabelModel::getGlobalModel()->removeFileLabel(*it, index);
                }else{
                    auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(*it);
                    if(!ids.contains(index)){
                        FileLabelModel::getGlobalModel()->addLabelToFile(*it, index);
                        FileLabelModel::getGlobalModel()->addLabelToFile(*it, index);
                        m_colorgroup->button(index)->setChecked(true);
                    }
                }
            }
        }
    }
}

FileLabelWidget::FileLabelWidget(const QStringList &selectionUris)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_selectionUris = const_cast<QStringList*>(&selectionUris);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    auto hbox = new QHBoxLayout;
    auto items = FileLabelModel::getGlobalModel()->getAllFileLabelItems();

    auto colorgroup = new QButtonGroup(this);
    m_colorgroup = colorgroup;
    m_colorgroup->setExclusive(false);

    int btnid = 1;
    for (auto item : items) {
        if (!item->isValidInMenu()) {
            continue;
        }
        ColorPushButton *colorButton=new ColorPushButton(item->color(),this);
        colorButton->setName(item->name());
        colorButton->palette().window();
        colorButton->setCheckable(true);
        colorButton->setEnabled(true);
        QList<int> ids;
        bool checkInMultiSelect = true;
        for (auto selectionUri : selectionUris) {
            auto id = FileLabelModel::getGlobalModel()->getFileLabelIds(selectionUri);
            checkInMultiSelect = checkInMultiSelect ? id.contains(item->id()) : false ;
            ids.append(id);
        }
        colorButton->m_checkInMultiSelect = checkInMultiSelect;
        ids = ids.toSet().toList();
        m_ids = ids;
        bool checked = ids.contains(item->id());
        colorgroup->addButton(colorButton,btnid);
        colorButton->setChecked(checked);
        colorgroup->button(btnid)->setChecked(checked);

        connect(colorButton, &ColorPushButton::changeText, this, &FileLabelWidget::changeText);

        btnid++;
    }

    for (QWidget * item : m_colorgroup->buttons()) {
            hbox->addWidget(item,0);
            item->installEventFilter(this);
    }
    hbox->setAlignment(Qt::AlignLeft);
    mainLayout->addLayout(hbox);
    this->setLayout(mainLayout);

    connect(m_colorgroup,SIGNAL(buttonClicked(int)),this,SLOT(clickItem(int)));
}

void FileLabelWidget::paintEvent(QPaintEvent *e)
{
    bool isUnderMouse = false;
    QString manager;
    for (QWidget * item : m_colorgroup->buttons()) {
        if(item->underMouse()) {
            isUnderMouse = true;
            break;
        }
    }
    if (!isUnderMouse){
        Q_EMIT changeText(tr("label management ..."));
    }
}
