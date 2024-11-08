#ifndef KSETTING_H
#define KSETTING_H
#include <QString>
#include <QVariant>
#include "plugindll.h"
#include "plugintask.h"


namespace KInstaller {


enum class Action : char{
    view,
    exec
};

class Ksetting
{

public:
    Ksetting(QString& filePath);
    static Ksetting* getInstance();
    static Ksetting* init(QString settingFilePath);

    using PluginConf = QList<QPair<KInstaller::Action, QStringList>>;
    Ksetting::PluginConf PluginConfInfo() const;
    bool loadAllLib(QString& path);    //加载所有插件Lib
    void loadViewsPlugin();            //加载视图显示插件
    void loadExecsPlugin();            //加载后台执行插件

    void checkPlugins();               //检查插件是否都存在


public:
    //static QString m_settingFilePath = "";              //配置文件路径
    QVariant m_varint;
    PluginConf m_settingsConfInfo;

    static Ksetting* m_setInstance;
    QList<QPair<std::string,IPlugin*>> iPlugins;           //视图插件存储
    QList<QPair<std::string,TPlugin*>> tPlugins;           //后台执行插件存储

    std::vector<std::string> *cls;


signals:

public slots:
};
}
#endif // KSETTING_H
