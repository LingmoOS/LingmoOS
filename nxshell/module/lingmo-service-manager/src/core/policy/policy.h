// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POLICY_H
#define POLICY_H

#include <QDBusConnection>
#include <QMap>
#include <QObject>

enum class SDKType { QT, SD };

struct PolicyWhitelist
{
    QString name;
    QStringList process;
};

typedef QMap<QString, PolicyWhitelist> QMapWhitelists;
// hide, default:false
typedef QMap<QString, bool> QMapPathHide;
// subpath, default:false
typedef QMap<QString, bool> QMapSubPath;

// typedef QMap<QString, bool> QMapProcess;
struct PolicyMethod
{
    QString method;
    bool needPermission;
    QStringList processes;
};

typedef QMap<QString, PolicyMethod> QMapMethod;

struct PolicyProperty
{
    QString property;
    bool needPermission;
    QStringList processes;
};

typedef QMap<QString, PolicyProperty> QMapProperty;

struct PolicyInterface
{
    QString interface;
    bool needPermission;
    QStringList processes;
    QMapMethod methods;
    QMapProperty properties;
};

typedef QMap<QString, PolicyInterface> QMapInterface;

struct PolicyPath
{
    QString path;
    bool needPermission;
    QStringList processes;
    QMapInterface interfaces;
};

typedef QMap<QString, PolicyPath> QMapPath;

enum CallDestType { Method, Property };

class Policy : public QObject
{
    Q_OBJECT
public:
    explicit Policy(QObject *parent = nullptr);

    void parseConfig(const QString &path);

    bool checkPathHide(const QString &path);
    bool checkMethodPermission(const QString &process,
                               const QString &path,
                               const QString &interface,
                               const QString &method);
    bool checkPropertyPermission(const QString &process,
                                 const QString &path,
                                 const QString &interface,
                                 const QString &property);
    bool checkPermission(const QString &process,
                         const QString &path,
                         const QString &interface,
                         const QString &dest,
                         const CallDestType &type);
    QStringList paths() const;
    bool allowSubPath(const QString &path) const;
    bool isResident() const;

    //    void Check(); // TODO
    void print();

private:
    bool readJsonFile(QJsonDocument &outDoc, const QString &fileName);
    bool parseWhitelist(const QJsonObject &obj);
    bool parsePolicy(const QJsonObject &obj);
    bool parsePolicyPath(const QJsonObject &obj);
    bool parsePolicyInterface(const QJsonObject &obj, PolicyPath &policyPath);
    bool parsePolicyMethod(const QJsonObject &obj, PolicyInterface &policyInterface);
    bool parsePolicyProperties(const QJsonObject &obj, PolicyInterface &policyInterface);

    bool jsonGetString(const QJsonObject &obj,
                       const QString &key,
                       QString &value,
                       QString defaultValue = "");
    bool jsonGetStringList(const QJsonObject &obj,
                           const QString &key,
                           QStringList &value,
                           QStringList defaultValue = {});
    bool
    jsonGetBool(const QJsonObject &obj, const QString &key, bool &value, bool defaultValue = false);
    bool jsonGetInt(const QJsonObject &obj, const QString &key, int &value, int defaultValue = 0);

public:
    // 数据定义
    // 插入速度要求不高，查询速度要求很高，因此解析json的结果会通过冗余、预处理来提高查询速度，即以查询的速度角度来定义结构
    // 配置文件和此处数据没有一一对应，解析文件时，需要为此处数据服务，填充相关数据
    // 如果文件没有配置某参数，那也不允许空值，根据数据向下继承，比如某个参数interface没有设置，则interface的这个参数继承path
    // 不允许空值的作用是减少查询时的逻辑判断，把这些逻辑处理转移到解析文件阶段
    // 隐藏 - In:path，Out:pathhide
    // 权限PATH - In:process、path，Out：bool
    // 权限INTERFACE - In:process、path、interface，Out：bool
    // 权限METHOD - In:process、path、interface、method，Out：bool
    // 权限PROPERTIES - In:process、path、interface，Out：bool
    QMapWhitelists mapWhitelist;
    QMapPathHide mapPathHide;
    QMapSubPath mapSubPath;
    QMapPath mapPath;

public:
    QString name;
    QString group;
    QString pluginPath;
    QString version;
    QString startType;
    QStringList dependencies;
    SDKType sdkType;
    int startDelay;
    int idleTime;
    QDBusConnection *dbus;
};

#endif // POLICY_H
