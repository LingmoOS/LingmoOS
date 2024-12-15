// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "policy.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(dsm_policy, "[Policy]")

Policy::Policy(QObject *parent)
    : QObject(parent)
    , dbus(nullptr)
{
}

bool Policy::checkPathHide(const QString &path)
{
    QMapPathHide::iterator iter = mapPathHide.find(path);
    if (iter == mapPathHide.end()) {
        return false;
    }
    return iter.value();
}

bool Policy::checkMethodPermission(const QString &process,
                                   const QString &path,
                                   const QString &interface,
                                   const QString &method)
{
    return checkPermission(process, path, interface, method, CallDestType::Method);
}

bool Policy::checkPropertyPermission(const QString &process,
                                     const QString &path,
                                     const QString &interface,
                                     const QString &property)
{
    return checkPermission(process, path, interface, property, CallDestType::Property);
}

bool Policy::checkPermission(const QString &process,
                             const QString &path,
                             const QString &interface,
                             const QString &dest,
                             const CallDestType &type)
{
    qCInfo(dsm_policy) << "check permission:"
                       << QString("process=%1, path=%2, interface=%3, dest=%4")
                                  .arg(process, path, interface, dest);
    // 时间复杂度跟权限配置复杂度正相关，简单的配置就会校验很快
    QMapPath::const_iterator iterPath = mapPath.find(path);
    if (iterPath == mapPath.end()) {
        // 默认不校验，即有权限
        return true;
    }

    // PATH权限
    const PolicyPath &policyPath = iterPath.value();
    QMapInterface::const_iterator iterInterface = policyPath.interfaces.find(interface);
    if (iterInterface == policyPath.interfaces.end()) {
        // 没有配置interface权限，则用path权限
        if (!policyPath.needPermission) {
            return true;
        }
        return policyPath.processes.contains(process);
    }

    if (type == CallDestType::Method) {
        // INTERFACE权限
        const PolicyInterface &policyInterface = iterInterface.value();
        QMapMethod::const_iterator iterMethod = policyInterface.methods.find(dest);
        if (iterMethod == policyInterface.methods.end()) {
            if (!policyInterface.needPermission) {
                return true;
            }
            return policyInterface.processes.contains(process);
        }
        // METHOD权限
        const PolicyMethod &policyMethod = iterMethod.value();
        if (!policyMethod.needPermission) {
            return true;
        }
        return policyMethod.processes.contains(process);

    } else if (type == CallDestType::Property) {
        // INTERFACE权限
        const PolicyInterface &policyInterface = iterInterface.value();
        QMapProperty::const_iterator iterProp = policyInterface.properties.find(dest);
        if (iterProp == policyInterface.properties.end()) {
            if (!policyInterface.needPermission) {
                return true;
            }
            return policyInterface.processes.contains(process);
        }
        // PROPERTY权限
        const PolicyProperty &policyProp = iterProp.value();
        if (!policyProp.needPermission) {
            return true;
        }
        return policyProp.processes.contains(process);
    }

    qCWarning(dsm_policy) << "check permission error!";
    return false;
}

QStringList Policy::paths() const
{
    return mapSubPath.keys();
}

bool Policy::allowSubPath(const QString &path) const
{
    auto iter = mapSubPath.find(path);
    if (iter != mapSubPath.end()) {
        return iter.value();
    }
    return false;
}

bool Policy::isResident() const
{
    return startType == "Resident";
}

void Policy::print()
{
    qInfo() << "-------------------------------------";
    qInfo() << "DBUS POLICY CONFIG";
    qInfo() << "- name:" << name;
    qInfo() << "- path hide";
    for (QMapPathHide::iterator iter = mapPathHide.begin(); iter != mapPathHide.end(); iter++) {
        qInfo() << "-- path hide:" << iter.key() << iter.value();
    }
    qInfo() << "- whitelist";
    for (QMapWhitelists::iterator iter = mapWhitelist.begin(); iter != mapWhitelist.end(); iter++) {
        qInfo() << "-- whitelist:" << iter.key() << iter.value().name << iter.value().process;
    }
    qInfo() << "- policy";
    for (QMapPath::iterator iter = mapPath.begin(); iter != mapPath.end(); iter++) {
        qInfo() << "-- path:" << iter.key() << iter.value().path;
        qInfo() << "-- permission:" << iter.value().needPermission;
        qInfo() << "-- whitelist:" << iter.value().processes;
        for (QMapInterface::iterator iterInterface = iter.value().interfaces.begin();
             iterInterface != iter.value().interfaces.end();
             iterInterface++) {
            qInfo() << "---- interface:" << iterInterface.key() << iterInterface.value().interface;
            qInfo() << "---- permission:" << iterInterface.value().needPermission;
            qInfo() << "---- whitelist:" << iterInterface.value().processes;
            for (QMapMethod::iterator iterMethod = iterInterface.value().methods.begin();
                 iterMethod != iterInterface.value().methods.end();
                 iterMethod++) {
                qInfo() << "------ method:" << iterMethod.key() << iterMethod.value().method;
                qInfo() << "------ permission:" << iterMethod.value().needPermission;
                qInfo() << "------ whitelist:" << iterMethod.value().processes;
            }
            for (QMapProperty::iterator iterProp = iterInterface.value().properties.begin();
                 iterProp != iterInterface.value().properties.end();
                 iterProp++) {
                qInfo() << "------ property:" << iterProp.key() << iterProp.value().property;
                qInfo() << "------ permission:" << iterProp.value().needPermission;
                qInfo() << "------ whitelist:" << iterProp.value().processes;
            }
        }
    }
    qInfo() << "-------------------------------------";
}

void Policy::parseConfig(const QString &path)
{
    qCInfo(dsm_policy) << "parse config:" << path;
    if (path.isEmpty()) {
        qCWarning(dsm_policy) << "path is empty!";
        return;
    }
    QJsonDocument jsonDoc;
    if (!readJsonFile(jsonDoc, path)) {
        qCWarning(dsm_policy) << "read json file failed!";
        return;
    }
    QJsonObject rootObj = jsonDoc.object();
    jsonGetString(rootObj, "name", name);
    jsonGetString(rootObj, "group", group, "app");
    jsonGetString(rootObj, "libPath", pluginPath); // 兼容
    jsonGetString(rootObj, "pluginPath", pluginPath, pluginPath);
    jsonGetString(rootObj, "policyVersion", version, "1.0"); // 兼容
    jsonGetString(rootObj, "version", version, version);
    jsonGetString(rootObj, "policyStartType", startType, "Resident"); // 兼容
    jsonGetString(rootObj, "startType", startType, startType);
    jsonGetStringList(rootObj, "dependencies", dependencies);
    jsonGetInt(rootObj, "startDelay", startDelay, 0);
    jsonGetInt(rootObj, "idleTime", idleTime, 10);
    // get SDKType
    QString sdkTypeString;
    jsonGetString(rootObj, "pluginType", sdkTypeString, "qt");
    if (sdkTypeString == "qt")
        sdkType = SDKType::QT;
    if (sdkTypeString == "sd")
        sdkType = SDKType::SD;

    if (name.isEmpty()) {
        qCWarning(dsm_policy) << "json error, name is empty.";
        return;
    }
    if (!parseWhitelist(rootObj)) {
        qCWarning(dsm_policy) << "json error, parse whitelist error.";
        return;
    }

    if (!parsePolicy(rootObj)) {
        qCWarning(dsm_policy) << "json error, parse policy error.";
        return;
    }
}

bool Policy::readJsonFile(QJsonDocument &outDoc, const QString &fileName)
{
    QFile jsonFIle(fileName);
    if (!jsonFIle.open(QIODevice::ReadOnly)) {
        qCWarning(dsm_policy) << QString("open file: %1 error!").arg(fileName);
        return false;
    }

    QJsonParseError jsonParserError;
    outDoc = QJsonDocument::fromJson(jsonFIle.readAll(), &jsonParserError);
    jsonFIle.close();
    if (jsonParserError.error != QJsonParseError::NoError) {
        qCWarning(dsm_policy) << "to json document error: " << jsonParserError.errorString();
        return false;
    }
    if (outDoc.isNull()) {
        qCWarning(dsm_policy) << "json document is null!";
        return false;
    }
    return true;
}

// typedef QMap<QString, PolicyWhitelist> QMapWhitelists;
bool Policy::parseWhitelist(const QJsonObject &obj)
{
    mapWhitelist.clear();
    if (!obj.contains("whitelists")) {
        // 为空，不是出错
        return true;
    }
    QJsonValue listsValue = obj.value("whitelists");
    if (!listsValue.isArray()) {
        qCWarning(dsm_policy) << "parse whitelist error, must be json array!";
        return false;
    }
    QJsonArray lists = listsValue.toArray();
    for (int i = 0; i < lists.size(); ++i) {
        QJsonValue whitelistValue = lists.at(i);
        if (whitelistValue.isObject()) {
            PolicyWhitelist whitelist;
            QJsonObject whitelistObj = whitelistValue.toObject();
            QString name;
            jsonGetString(whitelistObj, "name", name);
            if (name.isEmpty()) {
                continue;
            }
            if (!whitelistObj.contains("process")) {
                continue;
            }
            QJsonArray processes = whitelistObj.value("process").toArray();
            if (processes.size() <= 0) {
                continue;
            }
            whitelist.name = name;
            for (int j = 0; j < processes.size(); ++j) {
                if (processes.at(j).isString()) {
                    whitelist.process.append(processes.at(j).toString());
                }
            }
            mapWhitelist.insert(name, whitelist);
        }
    }

    return true;
}

bool Policy::parsePolicy(const QJsonObject &obj)
{
    mapPathHide.clear();
    mapPath.clear();
    if (!obj.contains("policy")) {
        // 为空，不是出错
        return true;
    }
    QJsonValue policyValue = obj.value("policy");
    if (!policyValue.isArray()) {
        qCWarning(dsm_policy) << "parse policy error, must be json array!";
        return false;
    }
    QJsonArray policyList = policyValue.toArray();
    for (int i = 0; i < policyList.size(); ++i) {
        QJsonValue policy = policyList.at(i);
        if (!policy.isObject())
            continue;
        if (!parsePolicyPath(policy.toObject())) {
            return false;
        }
    }
    return true;
}

bool Policy::parsePolicyPath(const QJsonObject &obj)
{
    QString path;
    jsonGetString(obj, "path", path);
    if (path.isEmpty()) {
        qCWarning(dsm_policy) << "parse policy-path error, must be a string!";
        return false;
    }

    bool pathHide;
    jsonGetBool(obj, "pathhide", pathHide, false);
    mapPathHide.insert(path, pathHide);

    bool subpath;
    jsonGetBool(obj, "subpath", subpath, false);
    mapSubPath.insert(path, pathHide);

    PolicyPath policyPath;
    policyPath.path = path;
    jsonGetBool(obj, "permission", policyPath.needPermission, false);
    QString whitelist;
    jsonGetString(obj, "whitelist", whitelist);
    if (!whitelist.isEmpty()) {
        QMapWhitelists::const_iterator iterWhitelist = mapWhitelist.find(whitelist);
        if (iterWhitelist != mapWhitelist.end() && iterWhitelist.value().name == whitelist) {
            policyPath.processes = iterWhitelist.value().process;
        }
    }

    if (obj.contains("interfaces")) {
        QJsonValue interfaces = obj.value("interfaces");
        if (interfaces.isArray()) {
            QJsonArray interfaceList = interfaces.toArray();
            for (int i = 0; i < interfaceList.size(); ++i) {
                QJsonValue interface = interfaceList.at(i);
                if (interface.isObject()) {
                    bool ret = parsePolicyInterface(interface.toObject(), policyPath);
                    if (!ret) {
                        return false;
                    }
                }
            }
        }
    }

    mapPath.insert(path, policyPath);

    return true;
}

bool Policy::parsePolicyInterface(const QJsonObject &obj, PolicyPath &policyPath)
{
    QString interface;
    jsonGetString(obj, "interface", interface);
    if (interface.isEmpty()) {
        qCWarning(dsm_policy) << "parse policy-interface error, must be a string!";
        return false;
    }

    PolicyInterface policyInterface;
    policyInterface.interface = interface;
    // interface没有指定permission，则使用上级path的permission
    jsonGetBool(obj, "permission", policyInterface.needPermission, policyPath.needPermission);
    QString whitelist;
    jsonGetString(obj, "whitelist", whitelist);
    if (!whitelist.isEmpty()) {
        QMapWhitelists::const_iterator iterWhitelist = mapWhitelist.find(whitelist);
        if (iterWhitelist != mapWhitelist.end() && iterWhitelist.value().name == whitelist) {
            policyInterface.processes = iterWhitelist.value().process;
        } // esle 错误的whitelist认为是空值
    } else {
        // interface没有指定whitelist，则使用上级path的whitelist
        policyInterface.processes = policyPath.processes;
    }

    if (obj.contains("methods")) {
        QJsonValue methods = obj.value("methods");
        if (methods.isArray()) {
            QJsonArray methodList = methods.toArray();
            for (int i = 0; i < methodList.size(); ++i) {
                QJsonValue method = methodList.at(i);
                if (method.isObject()) {
                    bool ret = parsePolicyMethod(method.toObject(), policyInterface);
                    if (!ret) {
                        return false;
                    }
                }
            }
        }
    }

    if (obj.contains("properties")) {
        QJsonValue properties = obj.value("properties");
        if (properties.isArray()) {
            QJsonArray propertiesList = properties.toArray();
            for (int i = 0; i < propertiesList.size(); ++i) {
                QJsonValue property = propertiesList.at(i);
                if (property.isObject()) {
                    bool ret = parsePolicyProperties(property.toObject(), policyInterface);
                    if (!ret) {
                        return false;
                    }
                }
            }
        }
    }

    policyPath.interfaces.insert(interface, policyInterface);
    return true;
}

bool Policy::parsePolicyMethod(const QJsonObject &obj, PolicyInterface &policyInterface)
{
    QString method;
    jsonGetString(obj, "method", method);
    if (method.isEmpty()) {
        qCWarning(dsm_policy) << "parse policy-method error, must be a string!";
        return false;
    }

    PolicyMethod policyMethod;
    policyMethod.method = method;
    // method没有指定permission，则使用上级interface的permission
    jsonGetBool(obj, "permission", policyMethod.needPermission, policyInterface.needPermission);
    QString whitelist;
    jsonGetString(obj, "whitelist", whitelist);
    if (!whitelist.isEmpty()) {
        QMapWhitelists::const_iterator iterWhitelist = mapWhitelist.find(whitelist);
        if (iterWhitelist != mapWhitelist.end() && iterWhitelist.value().name == whitelist) {
            policyMethod.processes = iterWhitelist.value().process;
        }
    } else {
        // method没有指定whitelist，则使用上级interface的whitelist
        policyMethod.processes = policyInterface.processes;
    }

    policyInterface.methods.insert(method, policyMethod);
    return true;
}

bool Policy::parsePolicyProperties(const QJsonObject &obj, PolicyInterface &policyInterface)
{
    QString property;
    jsonGetString(obj, "property", property);
    if (property.isEmpty()) {
        qCWarning(dsm_policy) << "parse policy-property error, must be a string!";
        return false;
    }

    PolicyProperty policyproperty;
    policyproperty.property = property;
    jsonGetBool(obj, "permission", policyproperty.needPermission, policyInterface.needPermission);
    QString whitelist;
    jsonGetString(obj, "whitelist", whitelist);
    if (!whitelist.isEmpty()) {
        QMapWhitelists::const_iterator iterWhitelist = mapWhitelist.find(whitelist);
        if (iterWhitelist != mapWhitelist.end() && iterWhitelist.value().name == whitelist) {
            policyproperty.processes = iterWhitelist.value().process;
        }
    } else {
        policyproperty.processes = policyInterface.processes;
    }

    policyInterface.properties.insert(property, policyproperty);
    return true;
}

bool Policy::jsonGetString(const QJsonObject &obj,
                           const QString &key,
                           QString &value,
                           QString defaultValue)
{
    if (obj.contains(key)) {
        const QJsonValue &v = obj.value(key);
        if (v.isString()) {
            value = v.toString();
            return true;
        }
    }
    value = defaultValue;
    return false;
}

bool Policy::jsonGetStringList(const QJsonObject &obj,
                               const QString &key,
                               QStringList &value,
                               QStringList defaultValue)
{
    value = defaultValue;
    if (!obj.contains(key))
        return false;
    const QJsonValue &v = obj.value(key);
    if (v.isString()) {
        value.append(v.toString());
        return true;
    }
    if (v.isArray()) {
        const QJsonArray &array = v.toArray();
        for (auto &&a : array) {
            if (a.isString())
                value.append(a.toString());
        }
    }
    return true;
}

bool Policy::jsonGetBool(const QJsonObject &obj, const QString &key, bool &value, bool defaultValue)
{
    if (obj.contains(key)) {
        const QJsonValue &v = obj.value(key);
        if (v.isBool()) {
            value = v.toBool();
            return true;
        }
    }
    value = defaultValue;
    return false;
}

bool Policy::jsonGetInt(const QJsonObject &obj, const QString &key, int &value, int defaultValue)
{
    if (obj.contains(key)) {
        const QJsonValue &v = obj.value(key);
        if (v.isDouble()) {
            value = v.toInt();
            return true;
        }
    }
    value = defaultValue;
    return false;
}
