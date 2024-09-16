// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screensaversettingdialog.h"
#include "utils.h"

#include <DSettingsWidgetFactory>
#include <DApplication>
#include <DSettingsOption>
#include <qsettingbackend.h>

#include <QWindow>
#include <QDir>
#include <QFontMetrics>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

ScreenSaverSettingDialog::ScreenSaverSettingDialog(const QString &name, QWidget *parent)
    : DSettingsDialog(parent), m_screenSaverName(name)
{
    QIcon icon(":/images/deepin-screensaver-config.svg");
    setIcon(icon);
    setWindowIcon(icon);

    // 加载翻译
    qApp->loadTranslator();

    QString appName = qApp->applicationName();
    qApp->setApplicationName(name);
    qApp->loadTranslator();
    qApp->setApplicationName(appName);

    QString confFilePath = Utils::configLocationPath(m_screenSaverName);
    QFileInfo confFile(confFilePath);

    // 本地配置文件不存在
    if (!confFile.exists()) {

        QDir confDir = confFile.absoluteDir();
        if (!confDir.exists())
            confDir.mkpath(confDir.absolutePath());

        // 系统配置文件存在，则拷贝。否则不做处理，后面设置值时会自动创建
        QString pathGeneral = Utils::configGeneralPath(m_screenSaverName);
        QFile confGeneralFile(pathGeneral);
        if (confGeneralFile.exists()) {
            confGeneralFile.copy(confFilePath);
        }
    }
    m_backend.reset(new QSettingBackend(confFilePath, this));

    QString jsonFilePath = Utils::jsonPath(m_screenSaverName);
    m_settings.reset(DSettings::fromJsonFile(jsonFilePath));
    m_settings->setParent(this);
    m_settings->setBackend(m_backend.get());

    updateSettings(m_settings.get());

    if (Utils::isWayLand()) {
        // 设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }
    this->setFixedSize(QSize(682, 532));
}

ScreenSaverSettingDialog::~ScreenSaverSettingDialog()
{
}
