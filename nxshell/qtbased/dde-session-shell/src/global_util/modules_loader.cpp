// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules_loader.h"

#include "base_module_interface.h"
#include "tray_module_interface.h"

#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <QPluginLoader>

namespace dss {
namespace module {

const QString ModulesDir = "/usr/lib/dde-session-shell/modules";
ModulesLoader::ModulesLoader(QObject *parent)
    : QThread(parent)
{
    QString localDir = QCoreApplication::applicationDirPath() + "/modules";
    addModulePath(localDir);
    addModulePath(ModulesDir);
}

ModulesLoader::~ModulesLoader()
{
}

ModulesLoader &ModulesLoader::instance()
{
    static ModulesLoader modulesLoader;
    return modulesLoader;
}

BaseModuleInterface *ModulesLoader::findModuleByName(const QString &name) const
{
    return m_modules.value(name, nullptr);
}

QHash<QString, BaseModuleInterface *> ModulesLoader::findModulesByType(const int type) const
{
    QHash<QString, BaseModuleInterface *> modules;
    for (BaseModuleInterface *module : m_modules.values()) {
        if (module->type() == type) {
            modules.insert(module->key(), module);
        }
    }
    return modules;
}

void ModulesLoader::run()
{
    for (const auto &path : m_modulePaths) {
        findModule(path);
    }
}

bool ModulesLoader::checkVersion(const QString &target, const QString &base)
{
    if (target == base) {
        return true;
    }
    const QStringList baseVersion = base.split(".");
    const QStringList targetVersion = target.split(".");

    const int baseVersionSize = baseVersion.size();
    const int targetVersionSize = targetVersion.size();
    const int size = baseVersionSize < targetVersionSize ? baseVersionSize : targetVersionSize;

    for (int i = 0; i < size; i++) {
        if (targetVersion[i] == baseVersion[i]) continue;
        return targetVersion[i].toInt() > baseVersion[i].toInt() ? true : false;
    }
    return true;
}

void ModulesLoader::findModule(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << path << "is not exists.";
        return;
    }
    const QFileInfoList modules = dir.entryInfoList();
    for (QFileInfo module : modules) {
        const QString path = module.absoluteFilePath();
        if (!QLibrary::isLibrary(path)) {
            continue;
        }
        qInfo() << module << "is found";
        QPluginLoader loader(path);
        const QJsonObject &meta = loader.metaData().value("MetaData").toObject();
        QString moduleVersion = meta.value("api").toString();
        if (!ValidVersions.contains(moduleVersion)) {
            qWarning() << "The module version is error!";
            continue;
        }

        BaseModuleInterface *moduleInstance = dynamic_cast<BaseModuleInterface *>(loader.instance());
        if (!moduleInstance) {
            qWarning() << loader.errorString();
            continue;
        }

        // 新版本BaseModuleInterface，新增isNeedInitPlugin来判断插件是否需要加载。
        if (moduleVersion == "1.0.1" && !moduleInstance->isNeedInitPlugin()) {
            qInfo() << "plugin :" << moduleInstance->key() << " version:"
                    << moduleVersion << " is valid, but not need load";
            continue;
        }

        if (m_modules.contains(moduleInstance->key())) {
            continue;
        }
        m_modules.insert(moduleInstance->key(), moduleInstance);
        emit moduleFound(moduleInstance);
    }
}

} // namespace module
} // namespace dss
