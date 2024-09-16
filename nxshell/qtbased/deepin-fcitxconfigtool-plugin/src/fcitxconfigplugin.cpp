// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "fcitxconfigplugin.h"
#include <QTranslator>

#include "window/imwindow.h"

FcitxConfigPlugin::FcitxConfigPlugin()
{
    m_translator.load(QLocale::system(),
                      QStringLiteral("deepin-fcitxconfigtool-plugin"),
                      QStringLiteral("_"),
                      QStringLiteral("/usr/share/deepin-fcitxconfigtool-plugin/translations"));
    qApp->installTranslator(&m_translator);
    m_imWindow = nullptr;
}

FcitxConfigPlugin::~FcitxConfigPlugin()
{
    qApp->removeTranslator(&m_translator);
    deleteImWindow();
}

void FcitxConfigPlugin::preInitialize(bool sync)
{
    Q_UNUSED(sync);
    qDebug() << __FUNCTION__;
}

QString FcitxConfigPlugin::path() const
{
    //返回控制中心键盘模块
    return KEYBOARD;
}

QString FcitxConfigPlugin::follow() const
{
    //返回三级菜单插入位置
    return "3";
}

void FcitxConfigPlugin::deactive()
{
    deleteImWindow();
}

void FcitxConfigPlugin::addChildPageTrans() const
{
    if (m_frameProxy != nullptr) {
        //defapp
        m_frameProxy->addChildPageTrans("Input Methods", tr("Input Methods"));
    }
}

void FcitxConfigPlugin::initSearchData()
{
    QString module = tr("Keyboard and Language");
    #ifdef QT_DEBUG
        module = "键盘和语言";
    #endif

        auto func_process_all = [ = ]() {
            m_frameProxy->setWidgetVisible(module, tr("Input Methods"), true);
            m_frameProxy->setDetailVisible(module, tr("Input Methods"), tr("Manage Input Methods"), true);
            m_frameProxy->setDetailVisible(module, tr("Input Methods"), tr("Switch input methods"), true);
            m_frameProxy->setDetailVisible(module, tr("Input Methods"), tr("Switch to the first input method"), true);
            m_frameProxy->setDetailVisible(module, tr("Input Methods"), tr("Advanced Settings"), true);
            m_frameProxy->updateSearchData(module);
        };

        func_process_all();
}

void FcitxConfigPlugin::initialize()
{
    qDebug() << __FUNCTION__ << QLocale::system().name();
}

void FcitxConfigPlugin::active()
{
    qDebug() << __FUNCTION__;
    m_imWindow = new IMWindow();
    connect(m_imWindow, &IMWindow::requestNextPage, [ = ](QWidget * const w) {
        m_frameProxy->pushWidget(this, w, dccV20::FrameProxyInterface::PushType::CoverTop);
    });
    m_frameProxy->pushWidget(this, m_imWindow, dccV20::FrameProxyInterface::PushType::Normal);
}

const QString FcitxConfigPlugin::name() const
{
    return tr("Input Methods");
}

//这个注释和翻译和使用控制台启动有关
const QString FcitxConfigPlugin::displayName() const
{
    //下面一行注释,和第二行文案有关联是控制中心搜索规范快捷键规范.不可以修改,不可以移动位置,下面三行要在一起
    //~ contents_path /keyboard/Manage Input Methods
    return tr("Input Methods");
}

QIcon FcitxConfigPlugin::icon() const
{
    return QIcon::fromTheme("input_method");
}

QString FcitxConfigPlugin::translationPath() const
{
    return QStringLiteral(":/translations/deepin-fcitxconfigtool-plugin_%1.ts");
}

void FcitxConfigPlugin::deleteImWindow()
{
    if (m_imWindow) {
        m_imWindow->deleteLater();
        m_imWindow = nullptr;
    }
}

