// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_icon.h"

#include "process_icon_cache.h"
#include "process.h"
#include "desktop_entry_cache.h"
#include "common/common.h"
#include "system/system_monitor.h"
#include "process/process_db.h"
#include "wm/wm_window_list.h"

#include <QFileInfo>
#include <QImage>

using namespace common::init;
using namespace common::core;
using namespace core::system;
using namespace core::process;
using namespace core::wm;

namespace core {
namespace process {

enum icon_data_type_t {
    kIconDataNameType,
    kIconDataPixmapType
};

struct icon_data_t {
    icon_data_type_t type;
    char __pad__[4];
    QString proc_name;
    bool desktopentry = false;

    virtual ~icon_data_t() {}
};

struct icon_data_name_type : public icon_data_t {
    QString icon_name;
};
struct icon_data_pix_map_type : public icon_data_t {
    QImage image;
};

ProcessIcon::ProcessIcon()
{
}

ProcessIcon::~ProcessIcon()
{

}

void ProcessIcon::refreashProcessIcon(Process *proc)
{
    if (proc) {
        ProcessIconCache *cache = ProcessIconCache::instance();
        if (cache->contains(proc->pid())) {
            auto *procIcon = cache->getProcessIcon(proc->pid());
            if (proc->name().compare(procIcon->m_data->proc_name, Qt::CaseInsensitive) == 0) {
                m_data = procIcon->m_data;
            } else {
                m_data = getIcon(proc);
                procIcon->m_data = m_data;
            }

            if (m_data->desktopentry)
                ProcessDB::instance()->windowList()->addDesktopEntryApp(proc);
        } else {
            auto *procIcon = new ProcessIcon();
            m_data = getIcon(proc);
            procIcon->m_data = m_data;
            cache->addProcessIcon(proc->pid(), procIcon);
        }
    }
}

QIcon ProcessIcon::icon() const
{
    QIcon icon;

    if (m_data) {
        if (m_data->type == kIconDataNameType) {
            auto *iconData = reinterpret_cast<struct icon_data_name_type *>(m_data.get());
            if (iconData)
                icon = QIcon::fromTheme(iconData->icon_name);
        } else if (m_data->type == kIconDataPixmapType) {
            auto *iconData = reinterpret_cast<struct icon_data_pix_map_type *>(m_data.get());
            if (iconData) {
                icon.addPixmap(QPixmap::fromImage(iconData->image));
            } // ::if(iconData)
        }
    } // ::if(m_data)

    return icon;
}

struct icon_data_t *ProcessIcon::defaultIconData(const QString &procname) const {
    auto *iconData = new struct icon_data_name_type();
    iconData->type = kIconDataNameType;
    iconData->proc_name = "[::default::]";
    iconData->icon_name = "application-x-executable";
    return iconData;
}

struct icon_data_t *ProcessIcon::terminalIconData(const QString &procname) const {
    auto *iconData = new struct icon_data_name_type();
    iconData->type = kIconDataNameType;
    iconData->proc_name = "[::terminal::]";
    iconData->icon_name = "terminal";
    return iconData;
}

QString ProcessIcon::getFileManagerString()
{
    DesktopEntryCache *desktopEntryCache = ProcessDB::instance()->desktopEntryCache();
    QList<QString> desktopEntryKeys = desktopEntryCache->getCache().keys();
    for (auto it = desktopEntryKeys.begin(); it != desktopEntryKeys.end(); it++) {
        if ((*it).contains("dde-file-manager") && desktopEntryCache->getCache().value((*it))->icon.contains("dde-file-manager")) {
            return (*it);
        }
    }
    return "";
}

std::shared_ptr<icon_data_t> ProcessIcon::getIcon(Process *proc)
{
    std::shared_ptr<icon_data_t> iconDataPtr;
    auto processDB = ProcessDB::instance();
    WMWindowList *windowList = processDB->windowList();
    DesktopEntryCache *desktopEntryCache = processDB->desktopEntryCache();

    if (!proc->cmdline().isEmpty()) {
        if (windowList->isTrayApp(proc->pid())) {
            if (proc->environ().contains("GIO_LAUNCHED_DESKTOP_FILE")) {
                auto desktopFile = proc->environ()["GIO_LAUNCHED_DESKTOP_FILE"];
                auto entry = desktopEntryCache->entryWithDesktopFile(desktopFile);
                if (entry && !entry->icon.isEmpty()) {
                    auto *iconData = new struct icon_data_name_type();
                    iconData->type = kIconDataNameType;
                    iconData->proc_name = proc->name();
                    iconData->icon_name = entry->icon;
                    iconDataPtr.reset(iconData);
                    return iconDataPtr;
                }
            }
        }

        if (windowList->isGuiApp(proc->pid())) {
            const QImage &image = windowList->getWindowIcon(proc->pid());
            if (!image.isNull()) {
                auto *iconData = new struct icon_data_pix_map_type();
                iconData->image = image;
                iconData->proc_name = proc->name();
                iconData->type = kIconDataPixmapType;
                iconDataPtr.reset(iconData);
                return iconDataPtr;
            }
        }

        if (desktopEntryCache->contains(proc->name())) {
            DesktopEntry entry;
            entry = desktopEntryCache->entry(proc->name());
            if (entry && !entry->icon.isEmpty()) {
                auto *iconData = new struct icon_data_name_type();
                iconData->desktopentry = true;
                iconData->type = kIconDataNameType;
                iconData->proc_name = proc->name();
                //由于文件管理器和主目录在desktopEntryCache的m_cache中的key都包含"dde-file-manager"
                //且value中的结构体里的name都是"dde-file-manager"，所以在这里对于文件管理器的图标要多做判断
                QString tempString = getFileManagerString();
                if (tempString.contains(proc->name())) {
                    iconData->icon_name = desktopEntryCache->getCache().value(tempString)->icon;
                } else {
                    iconData->icon_name = entry->icon;
                }
                iconDataPtr.reset(iconData);
                windowList->addDesktopEntryApp(proc);
                return iconDataPtr;
            }
        }

        QHash<QString, QString> allEnviron = proc->environ();
        if (allEnviron.contains("GIO_LAUNCHED_DESKTOP_FILE") && ((allEnviron.contains("XDG_DATA_DIRS") && !allEnviron["XDG_DATA_DIRS"].isEmpty())
                                                                 || (allEnviron.contains("GIO_LAUNCHED_DESKTOP_FILE_PID") && allEnviron["GIO_LAUNCHED_DESKTOP_FILE_PID"].toInt() == proc->pid()))) {
            auto desktopFile = proc->environ()["GIO_LAUNCHED_DESKTOP_FILE"];
            auto entry = desktopEntryCache->entryWithDesktopFile(desktopFile);
            if (entry && !entry->icon.isEmpty()) {
                auto *iconData = new struct icon_data_name_type();
                iconData->desktopentry = true;
                iconData->type = kIconDataNameType;
                iconData->proc_name = proc->name();
                iconData->icon_name = entry->icon;
                iconDataPtr.reset(iconData);
                windowList->addDesktopEntryApp(proc);
                return iconDataPtr;
            }
        }

        if (shellList.contains(proc->name())) {
            iconDataPtr.reset(terminalIconData(proc->name()));
            return iconDataPtr;
        }

        if (!proc->cmdline().isEmpty() && proc->cmdline()[0].startsWith("/opt")) {
            QString fname = QFileInfo(QString(proc->cmdline()[0]).split(' ')[0]).fileName();
            auto entry = desktopEntryCache->entryWithSubName(fname);
            if (entry && !entry->icon.isEmpty()) {
                auto *iconData = new struct icon_data_name_type();
                iconData->type = kIconDataNameType;
                iconData->proc_name = proc->name();
                iconData->icon_name = entry->icon;
                iconDataPtr.reset(iconData);
                return iconDataPtr;
            }
        }
    }

    // fallback to use default icon
    iconDataPtr.reset(defaultIconData(proc->name()));
    return iconDataPtr;
}

} // namespace process
} // namespace core
