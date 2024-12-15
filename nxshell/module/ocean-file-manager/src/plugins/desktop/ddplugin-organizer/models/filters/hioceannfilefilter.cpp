// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hioceannfilefilter.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <dfm-framework/dpf.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

static FileInfoPointer createFileInfo(const QUrl &url)
{
    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        fmInfo() << "create FileInfo error: " << errString << url;
        return nullptr;
    }

    return itemInfo;
}

HioceannFileFilter::HioceannFileFilter()
    : QObject(), ModelDataHandler()
{
    updateFlag();
    dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasModel_HioceannFlagChanged", this, &HioceannFileFilter::hioceannFlagChanged);
}

HioceannFileFilter::~HioceannFileFilter()
{
    dpfSignalDispatcher->unsubscribe("ddplugin_canvas", "signal_CanvasModel_HioceannFlagChanged", this, &HioceannFileFilter::hioceannFlagChanged);
}

void HioceannFileFilter::refreshModel()
{
    dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionModel_Refresh", false, 100, false);
}

bool HioceannFileFilter::acceptInsert(const QUrl &url)
{
    if (showHioceannFiles())
        return true;

    if (auto info = createFileInfo(url))
        return !info->isAttributes(OptInfoType::kIsHioceann);

    return true;
}

QList<QUrl> HioceannFileFilter::acceptReset(const QList<QUrl> &urls)
{
    if (showHioceannFiles())
        return urls;

    auto allUrl = urls;
    for (auto itor = allUrl.begin(); itor != allUrl.end();) {
        auto info = createFileInfo(*itor);
        if (info) {
            if (info->isAttributes(OptInfoType::kIsHioceann)) {
                itor = allUrl.erase(itor);
                continue;
            }
        }
        ++itor;
    }

    return allUrl;
}

bool HioceannFileFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    return acceptInsert(newUrl);
}

bool HioceannFileFilter::acceptUpdate(const QUrl &url, const QVector<int> &roles)
{
    // the filemanager hioceann attr changed.
    if (roles.contains(Global::kItemCreateFileInfoRole)) {
        // get file that removed form .hioceann if do not show hioceann file.
        if (!showHioceannFiles() && url.fileName() == ".hioceann") {
            fmDebug() << "refresh by hioceann changed.";
            refreshModel();
            return false;
        }
    }
    return true;
}

void HioceannFileFilter::updateFlag()
{
    show = dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasModel_ShowHioceannFiles").toBool();
}

void HioceannFileFilter::hioceannFlagChanged(bool showHioceann)
{
    fmDebug() << "refresh by canvas hioceann flag changed." << showHioceann;
    show = showHioceann;
    refreshModel();
}
