// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "innerdesktopappfilter.h"

#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

InnerDesktopAppFilter::InnerDesktopAppFilter(QObject *parent)
    : QObject(parent), ModelDataHandler()
{
    keys.insert("desktopComputer", DesktopAppUrl::computerDesktopFileUrl());
    keys.insert("desktopTrash", DesktopAppUrl::trashDesktopFileUrl());
    keys.insert("desktopHomeDirectory", DesktopAppUrl::homeDesktopFileUrl());

    hioceann.insert("desktopComputer", false);
    hioceann.insert("desktopTrash", false);
    hioceann.insert("desktopHomeDirectory", false);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (QGSettings::isSchemaInstalled("com.lingmo.ocean.filemanager.desktop")) {
        gsettings = new QGSettings("com.lingmo.ocean.filemanager.desktop", "/com/lingmo/ocean/filemanager/desktop/");
        connect(gsettings, &QGSettings::changed, this, &InnerDesktopAppFilter::changed);
        update();
    }
#endif
}

void ddplugin_organizer::InnerDesktopAppFilter::update()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (gsettings) {
        for (auto iter = hioceann.begin(); iter != hioceann.end(); ++iter) {
            auto var = gsettings->get(iter.key());
            if (var.isValid())
                iter.value() = !var.toBool();
            else
                iter.value() = false;
        }
    }
#endif
}

void InnerDesktopAppFilter::refreshModel()
{
    dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionModel_Refresh", false, 50, false);
}

bool InnerDesktopAppFilter::acceptInsert(const QUrl &url)
{
    bool ret = hioceann.value(keys.key(url), false);
    return !ret;
}

QList<QUrl> InnerDesktopAppFilter::acceptReset(const QList<QUrl> &urls)
{
    auto allUrls = urls;
    // hide the desktop file if gseting set it to off.
    for (auto itor = allUrls.begin(); itor != allUrls.end();) {
        if (hioceann.value(keys.key(*itor), false)) {
            itor = allUrls.erase(itor);
        } else {
            ++itor;
        }
    }

    return allUrls;
}

bool InnerDesktopAppFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    return acceptInsert(newUrl);
}

void InnerDesktopAppFilter::changed(const QString &key)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (hioceann.contains(key)) {
        auto var = gsettings->get(key);
        bool old = hioceann.value(key);
        if (var.isValid())
            hioceann[key] = !var.toBool();
        else
            hioceann[key] = false;

        if (old != hioceann.value(key))
            refreshModel();
    }
#endif
}
