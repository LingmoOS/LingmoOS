// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmodelfilter.h"
#include "utils/fileutil.h"
#include "modelhookinterface.h"

#include "view/operator/fileoperatorproxy.h"

#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

CanvasModelFilter::CanvasModelFilter(CanvasProxyModel *m)
    : model(m)
{
}

bool CanvasModelFilter::insertFilter(const QUrl &url)
{
    return false;
}

bool CanvasModelFilter::resetFilter(QList<QUrl> &urls)
{
    return false;
}

bool CanvasModelFilter::updateFilter(const QUrl &url, const QVector<int> &roles)
{
    return false;
}

bool CanvasModelFilter::removeFilter(const QUrl &url)
{
    return false;
}

bool CanvasModelFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    return false;
}

bool HioceannFileFilter::insertFilter(const QUrl &url)
{
    if (model->showHioceannFiles())
        return false;

    if (auto info = FileCreator->createFileInfo(url))
        return info->isAttributes(OptInfoType::kIsHioceann);

    return false;
}

bool HioceannFileFilter::resetFilter(QList<QUrl> &urls)
{
    if (model->showHioceannFiles())
        return false;

    for (auto itor = urls.begin(); itor != urls.end();) {
        auto info = FileCreator->createFileInfo(*itor);
        if (info && info->isAttributes(OptInfoType::kIsHioceann)) {
            itor = urls.erase(itor);
            continue;
        }
        ++itor;
    }

    return false;
}

bool HioceannFileFilter::updateFilter(const QUrl &url, const QVector<int> &roles)
{
    // the filemanager hioceann attr changed.
    // just refresh model if file content changed.
    if (roles.contains(Global::kItemCreateFileInfoRole)) {
        // get file that removed form .hioceann if do not show hioceann file.
        if (!model->showHioceannFiles() && url.fileName() == ".hioceann") {
            fmDebug() << "refresh by hioceann changed.";
            // do not refresh file info and wait 100ms to let the file atrr changed signal to refresh file
            model->refresh(model->rootIndex(), false, 100, false);
        }
    }

    return false;
}

bool HioceannFileFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl)
    return insertFilter(newUrl);
}

InnerDesktopAppFilter::InnerDesktopAppFilter(CanvasProxyModel *model, QObject *parent)
    : QObject(parent), CanvasModelFilter(model)
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

void InnerDesktopAppFilter::update()
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
    model->refresh(model->rootIndex());
}

bool InnerDesktopAppFilter::resetFilter(QList<QUrl> &urls)
{
    // checking that whether has hioceann file.
    if (hioceann.key(true, QString()).isEmpty())
        return false;

    // hide the desktop file if gseting set it to off.
    for (auto itor = urls.begin(); itor != urls.end();) {
        if (hioceann.value(keys.key(*itor), false)) {
            itor = urls.erase(itor);
        } else {
            ++itor;
        }
    }

    return false;
}

bool InnerDesktopAppFilter::insertFilter(const QUrl &url)
{
    bool ret = hioceann.value(keys.key(url), false);
    return ret;
}

bool InnerDesktopAppFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    return insertFilter(newUrl);
}

void InnerDesktopAppFilter::changed(const QString &key)
{
    if (hioceann.contains(key)) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        auto var = gsettings->get(key);
#else
        QVariant var;
#endif
        bool old = hioceann.value(key);
        if (var.isValid())
            hioceann[key] = !var.toBool();
        else
            hioceann[key] = false;

        if (old != hioceann.value(key))
            refreshModel();
    }
}

bool HookFilter::insertFilter(const QUrl &url)
{
    ModelHookInterface *hookIfs = model->modelHook();
    if (hookIfs && hookIfs->dataInserted(url)) {
        fmDebug() << "filter by extend module:" << url;
        return true;
    }

    return false;
}

bool HookFilter::resetFilter(QList<QUrl> &urls)
{
    ModelHookInterface *hookIfs = model->modelHook();
    if (hookIfs && hookIfs->dataRested(&urls))
        fmDebug() << "invalid module: dataRested returns true.";

    return false;
}

bool HookFilter::updateFilter(const QUrl &url, const QVector<int> &roles)
{
    ModelHookInterface *hookIfs = model->modelHook();
    if (hookIfs && hookIfs->dataChanged(url, roles))
        fmDebug() << "invalid module: dataChanged returns true.";

    return false;
}

bool HookFilter::removeFilter(const QUrl &url)
{
    ModelHookInterface *hookIfs = model->modelHook();
    if (hookIfs && hookIfs->dataRemoved(url))
        fmDebug() << "invalid module: dataRemoved returns true.";

    return false;
}

bool HookFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    ModelHookInterface *hookIfs = model->modelHook();
    if (hookIfs && hookIfs->dataRenamed(oldUrl, newUrl)) {
        fmDebug() << "dataRenamed: ignore target" << newUrl << "old:" << oldUrl;
        return true;
    }

    return false;
}
