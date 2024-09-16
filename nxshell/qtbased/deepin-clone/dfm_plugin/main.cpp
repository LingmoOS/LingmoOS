// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <dfmfilecontrollerplugin.h>
#include <dfmeventdispatcher.h>
#include <dde-file-manager/dfileservices.h>
#include <dfmabstracteventhandler.h>

#include <QWidget>

#include "dimfilecontroller.h"
#include "../app/src/dglobal.h"

bool Global::isOverride = true;
bool Global::disableMD5CheckForDimFile = false;
bool Global::disableLoopDevice = true;
bool Global::fixBoot = false;
bool Global::isTUIMode = false;

int Global::bufferSize = 1024 * 1024;
int Global::compressionLevel = 0;
int Global::debugLevel = 1;

DFM_USE_NAMESPACE

class DIMFileEventFilter : public DFMAbstractEventHandler
{
public:
    bool fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target = 0, QVariant *resultData = 0) override
    {
        Q_UNUSED(target)
        Q_UNUSED(resultData)

        if (event->type() != DFMEvent::OpenFile) {
            return false;
        }

        const DUrl &url = dfmevent_cast<DFMOpenFileEvent>(*event.data()).url();

        if (!url.isLocalFile())
            return false;

        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(0, url);

        if (info->mimeTypeName() == "application/x-deepinclone-dim") {
            const QWidget *widget = qobject_cast<const QWidget*>(event->sender());

            if (!widget)
                return false;

            DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(event->sender(), DUrl("dim://" + url.toLocalFile()), widget->window());

            return true;
        }

        return false;
    }
};

class DIMFilePlugin : public DFMFileControllerPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DFMFileControllerFactoryInterface_iid FILE "dfm-plugin-dim-file.json")

public:
    explicit DIMFilePlugin(QObject *parent = 0)
        : DFMFileControllerPlugin(parent) {
        DFMEventDispatcher::instance()->installEventFilter(new DIMFileEventFilter());
    }

    virtual DAbstractFileController *create(const QString &key) override {
        Q_UNUSED(key)

        return new DIMFileController();
    }
};

#include "main.moc"
