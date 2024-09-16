// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datatransfercoreplugin.h"
#include "common/commonutils.h"
#include "gui/mainwindow.h"

#include <net/helper/transferhepler.h>

#include <utils/transferutil.h>

using namespace data_transfer_core;
using namespace deepin_cross;

DataTransferCorePlugin::DataTransferCorePlugin(QObject *parent)
    : QObject(parent)
{
    initialize();
}

DataTransferCorePlugin::~DataTransferCorePlugin()
{
}

void DataTransferCorePlugin::initialize()
{
    CommonUitls::initLog();
    CommonUitls::loadTranslator();
}

bool DataTransferCorePlugin::start()
{
    TransferUtil::instance();
    TransferHelper::instance();
    return loadMainPage();
}

void DataTransferCorePlugin::stop()
{
    TransferHelper::instance()->finish();
    if (w) {
        delete w;
        w = nullptr;
    }
}

bool DataTransferCorePlugin::loadMainPage()
{
    w = new MainWindow();
    w->show();
    return true;
}
