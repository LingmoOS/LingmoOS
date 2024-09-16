// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODULES_LOADER_H
#define MODULES_LOADER_H

#include <QHash>
#include <QThread>

namespace dss {
namespace module {

class BaseModuleInterface;
class ModulesLoader : public QThread
{
    Q_OBJECT
public:
    static ModulesLoader &instance();

    inline QHash<QString, BaseModuleInterface *> moduleList() { return m_modules; }
    BaseModuleInterface *findModuleByName(const QString &name) const;
    QHash<QString, BaseModuleInterface *> findModulesByType(const int type) const;
    inline void setModulePaths(const QStringList &paths) { m_modulePaths.clear(); m_modulePaths.append(paths); }
    inline void addModulePath(const QString &path) { m_modulePaths.append(path); }

signals:
    void moduleFound(BaseModuleInterface *);

protected:
    void run() override;

private:
    explicit ModulesLoader(QObject *parent = nullptr);
    ~ModulesLoader() override;
    ModulesLoader(const ModulesLoader &) = delete;
    ModulesLoader &operator=(const ModulesLoader &) = delete;

    bool checkVersion(const QString &target, const QString &base);
    void findModule(const QString &path);

private:
    QHash<QString, BaseModuleInterface *> m_modules;
    QStringList m_modulePaths;
};

} // namespace module
} // namespace dss
#endif // MODULES_LOADER_H
