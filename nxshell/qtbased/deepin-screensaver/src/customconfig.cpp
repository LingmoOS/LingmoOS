// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customconfig.h"
#include "screensaversettingdialog.h"
#include "utils.h"

#include <QApplication>
#include <QDebug>

CustomConfig::CustomConfig(QObject *parent)
    : QObject(parent)
{
}

CustomConfig::~CustomConfig()
{
}

bool CustomConfig::startCustomConfig(const QString &name)
{
    if (!Utils::hasConfigFile(name))
        return false;

    if (m_settingDialiog && (name == m_lastConfigName)) {
        m_settingDialiog->activateWindow();
        return true;
    }

    if (m_settingDialiog)
        delete m_settingDialiog;

    m_settingDialiog = new ScreenSaverSettingDialog(name);
    m_settingDialiog->setAttribute(Qt::WA_DeleteOnClose);
    // remove dialog flag to let dock show window entry.
    m_settingDialiog->setWindowFlag(Qt::Dialog, false);
    m_settingDialiog->show();
    m_lastConfigName = name;
    return true;
}
