#include "ksetting.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QSettings>
#include <QLibrary>
#include "../PluginService/kyamlcpp.h"
#include <QApplication>

using namespace KServer;

namespace KInstaller {

bool Ksetting::loadAllLib(QString& path)
{
    QDir dir(path);
    QStringList nameFilters = {};
    nameFilters << "*.so";
    QStringList files = dir.entryList(nameFilters,QDir::Files|QDir::Readable,QDir::Name);
    for(const auto& plugin : m_settingsConfInfo) {
        QStringList sl = plugin.second;
        for(const QString& name : sl) {
            QString sp2 = path + name + ".so";
            qDebug() << sp2;
            QLibrary *lib2=new QLibrary(sp2);
            if(!lib2->load()) {
                qDebug() <<"lib" << name <<".so" << ":加载出错!";
                qDebug() << "lib加载出错!" << lib2->errorString();
            }
            delete lib2;
        }
    }

    return true;
}

void Ksetting::loadViewsPlugin()
{
    qDebug() << "装载显示视图插件";

    //根据配置文件加载插件
    std::vector<std::string> *cls = Factory::GetClassNames();
    for (auto c_iter = cls->begin(); c_iter != cls->end(); ++c_iter) {
        if (Factory::CreateObject(*c_iter)->GetFlag() == 0) {
            IPlugin *plg=(IPlugin*)(Factory::CreateObject(*c_iter));
            iPlugins.append(qMakePair(*c_iter, plg));
        }
    }
}

void Ksetting::loadExecsPlugin()
{
    qDebug() << "装载后台执行插件";

    //根据配置文件加载插件
    std::vector<std::string> *cls = Factory::GetClassNames();

    for (auto c_iter = cls->begin();c_iter != cls->end(); ++c_iter) {
        if (Factory::CreateObject(*c_iter)->GetFlag() == 1) {
            TPlugin *plg = (TPlugin*)(Factory::CreateObject(*c_iter));
            tPlugins.append(qMakePair(*c_iter,plg));
        }
    }
}

Ksetting::Ksetting(QString& filePath)
{
    qDebug() << "加载配置文件:";

    m_setInstance = this;

    QFile file(filePath);
    if (file.exists() && file.open(QFile::ReadOnly | QFile ::Text)) {
        QByteArray array = file.readAll();
        YAML::Node node = YAML::Load(array.constData());
        if (node) {

            QVariant variant = KServer::yamlToVariant(node["sequence"]);

            const auto list = variant.toList();
            for (const QVariant& item : list) {
                QString actionStr = item.toMap().firstKey();
                KInstaller::Action tmpAction;
                if (actionStr == "view") {
                    tmpAction = KInstaller::Action::view;
                } else if(actionStr == "exec") {
                    tmpAction = KInstaller::Action::exec;
                } else {
                    continue;
                }
                QStringList actionList = item.toMap().value(actionStr).toStringList();
                m_settingsConfInfo.append(qMakePair(tmpAction, actionList));
                qDebug()<<actionList;
            }
        }

    }

    // TODO
#if 0
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
    QString path2 = dir.path();
    path2.append("/lingmo-os-installer-1.0.17/src");
    path2.append("/KPlugins/");
#else
    QString path2 = "/usr/lib/";
#endif

    if (!loadAllLib(path2)) {
        qDebug() << "插件加载失败!";
        return;
    }
    //加载插件
    loadViewsPlugin();
    loadExecsPlugin();

}

Ksetting* Ksetting::m_setInstance = nullptr;

Ksetting* Ksetting::getInstance()
{
    return m_setInstance;
}

Ksetting* Ksetting::init(QString settingFilePath)
{
    //m_settingFilePath = settingFilePath;
    if (m_setInstance) {
        qDebug() << "settings句柄已存在";
        return m_setInstance;
    }
    return new Ksetting(settingFilePath);
}

Ksetting::PluginConf Ksetting::PluginConfInfo() const
{
    return m_settingsConfInfo;
}

}
