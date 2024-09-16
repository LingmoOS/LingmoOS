// SPDX-FileCopyrightText: 2021 - 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include <csignal>
#include <DConfig>
#include <DLog>
DCORE_USE_NAMESPACE

class DConfigExample {
public:
    explicit DConfigExample(const QString &fileName)
        :fileName(fileName)
    {
        baseAPI();
        watchValueChanged();
        subpath();
        mutilFetchConfig();
        generalConfig();
    }

    void watchValueChanged()
    {
        heapConfig.reset(new DConfig(fileName));
        if (!heapConfig->isValid()) {
            qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").
                          arg(heapConfig->name(), heapConfig->subpath());
            return;
        }

        // 监听值改变的信号
        const auto &oldValue = heapConfig->value("canExit").toBool();
        QObject::connect(heapConfig.get(), &DConfig::valueChanged, [oldValue, this](const QString &key){
            qDebug() << "value changed, oldValue:" << oldValue << ", newValue:" << heapConfig->value(key).toBool();
        });

        // reset
        heapConfig->setValue("canExit", !oldValue);
        heapConfig->reset("canExit");
    }

    void baseAPI()
    {
        // 构造DConfig
        DConfig config(fileName);

        // 判断是否有效
        if (!config.isValid()) {
            qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").
                          arg(config.name(), config.subpath());
            return;
        }

        // 获取所有配置项的key
        qDebug() << "All key items." << config.keyList();

        for (auto item :config.keyList()) {
            qDebug() << item << config.value(item);
        }

        // 获取指定配置项的值
        qDebug() << "key item's value:" << config.value("canExit").toBool();

        // 设置值指定配置项的值
        config.setValue("canExit", false);

        qDebug() << "key item's value is default:" << config.isDefaultValue("canExit");

        // 重置指定配置项的值
        config.reset("canExit");
    }

    void subpath()
    {
        // 构造DConfig
        DConfig config(fileName, "/a");

        // 判断是否有效
        if (!config.isValid()) {
            qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").
                          arg(config.name(), config.subpath());
            return;
        }

        // 获取指定配置项的值
        qDebug() << "subpath's key item's value:" << config.value("key3").toString();

        qDebug() << "no subpath's key item's value:" << DConfig(fileName).value("key3").toString();
    }

    void mutilFetchConfig()
    {
        QVariantMap map;
        for (int i = 0; i < 1; i++) {
            QVariantMap nestItem;
            for (int j = 0; j < 1; j++) {
                nestItem[QString::number(j)] = QString::number(j);
            }
            map[QString::number(i)] = nestItem;
        }
        {
            DConfig config(fileName);
            config.setValue("map", map);
        }
        {
            DConfig config(fileName);
            qDebug() << config.value("map", map);
        }
        // reset
        {
            DConfig config(fileName);
            config.reset("map");
        }
    }
    void generalConfig()
    {
        // 空appid，直接获取公共配置
        {
            QScopedPointer<DConfig> config(DConfig::createGeneric(fileName));
            qDebug() << config->value("canExit");
        }
        // 空appid，设置公共配置
        {
            QScopedPointer<DConfig> config(DConfig::createGeneric(fileName));
            config->setValue("canExit", true);
            qDebug() << config->value("canExit");
        }
        // 默认使用本appid去获取，会fallback到公共配置
        {
            DConfig config(fileName);
            qDebug() << config.value("canExit");
        }
        // 指定使用appid去获取，会fallback到公共配置
        {
            QScopedPointer<DConfig> config(DConfig::create("noexist-appid", fileName));
            qDebug() << config->value("canExit");
        }
        // reset
        {
            QScopedPointer<DConfig> config(DConfig::createGeneric(fileName));
            config->reset("canExit");
        }
    }

private:
    QScopedPointer<DConfig> heapConfig;
    QString fileName;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Dtk::Core::DLogManager::registerConsoleAppender();

    DConfigExample example("example");

    // 异常处理，调用QCoreApplication::exit，使DConfig正常析构。
    std::signal(SIGINT, &QCoreApplication::exit);
    std::signal(SIGABRT, &QCoreApplication::exit);
    std::signal(SIGTERM, &QCoreApplication::exit);
    std::signal(SIGKILL, &QCoreApplication::exit) ;

    return a.exec();
}
