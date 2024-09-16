/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "chameleontheme.h"
#include "chameleonconfig.h"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>

//#include "netwm_def.h"

Q_LOGGING_CATEGORY(CHAMELEON, "chameleon", QtWarningMsg)

#define BASE_THEME "deepin"
#define BASE_THEME_DIR ":/deepin/themes"

static QMap<NET::WindowType, UIWindowType> mapNETWindowType2UIWindowType = {
    {NET::WindowType::Normal,        UIWindowType::Normal},
    {NET::WindowType::Dialog,        UIWindowType::Dialog},
    {NET::WindowType::Dock,          UIWindowType::Dock},
    {NET::WindowType::PopupMenu,     UIWindowType::PopupMenu},
    {NET::WindowType::DropdownMenu,  UIWindowType::DropdownMenu},
    {NET::WindowType::Tooltip,       UIWindowType::Tooltip},
};

QPair<qreal, qreal> ChameleonTheme::takePair(const QVariant &value, const QPair<qreal, qreal> defaultValue)
{
    if (!value.isValid()) {
        return defaultValue;
    }

    QStringList l = value.toString().split(",");

    if (l.count() < 2) {
        return defaultValue;
    }

    QPair<qreal, qreal> ret;

    ret.first = l.first().toDouble();
    ret.second = l.at(1).toDouble();

    return ret;
}

QMarginsF ChameleonTheme::takeMargins(const QVariant &value, const QMarginsF &defaultValue)
{
    if (!value.isValid()) {
        return defaultValue;
    }

    QStringList l = value.toStringList();

    if (l.isEmpty()) {
        l = value.toString().split(",");
    }

    if (l.count() < 4) {
        return defaultValue;
    }

    return QMarginsF(l.at(0).toDouble(), l.at(1).toDouble(),
                     l.at(2).toDouble(), l.at(3).toDouble());
}

static inline QPointF toPos(const QPair<qreal, qreal> &pair)
{
    return QPointF(pair.first, pair.second);
}

QPointF ChameleonTheme::takePos(const QVariant &value, const QPointF defaultValue)
{
    return toPos(takePair(value, qMakePair(defaultValue.x(), defaultValue.y())));
}

static QColor takeColor(const QVariant &value, const QColor &defaultValue)
{
    const QString &color_name = value.toString();

    QColor c(color_name);

    if (!c.isValid())
        return defaultValue;

    return c;
}

static QIcon takeIcon(const QJsonObject &iconObj, QIcon base, QString defaultValue)
{
    if (!base.isNull()) {
        defaultValue.clear();
    }

    const QString normal = iconObj.value("normal").toString();
    const QString hover = iconObj.value("hover").toString();
    const QString press = iconObj.value("press").toString();
    const QString disabled = iconObj.value("disabled").toString();

    if (base.isNull()) {
        base.addFile(normal);
        base.addFile(hover, QSize(), QIcon::Active);
        base.addFile(press, QSize(), QIcon::Selected);
        base.addFile(disabled, QSize(), QIcon::Disabled);
    } else { // 开启fallback到base icon的行为
        if (!normal.startsWith("_"))
            base.addFile(normal);

        if (!hover.startsWith("_"))
            base.addFile(hover, QSize(), QIcon::Active);

        if (!press.startsWith("_"))
            base.addFile(press, QSize(), QIcon::Selected);

        if (!disabled.startsWith("_"))
            base.addFile(disabled, QSize(), QIcon::Disabled);
    }

    return base;
}

class _ChameleonTheme : public ChameleonTheme {
public:
    _ChameleonTheme() : ChameleonTheme() {}
};
Q_GLOBAL_STATIC(_ChameleonTheme, _global_ct)

ChameleonTheme *ChameleonTheme::instance()
{
    return _global_ct;
}

/**
 * @brief
 *
 * @param jsonObj
 * @param key
 * @param status
 * @return QVariant
 * example:
 * "shadowColor": {
 *      "active": "#FF0000"
 *      "inactive": "#00FF00"
 *      "unmanaged": "#0000FF"
 * }
 * "shadowColor": {
 *      "managed": "#FF0000"
 *      "unmanaged": "#0000FF"
 * }
 * jsonObj: QJson["shadowColor"]
 * key:     shadowColor
 * status:  managed/inactive/unmanaged
 */
static QVariant paserAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status)
{
    const QJsonValue& val = jsonObj[key];
    if (val.isUndefined()) {
        return QVariant();
    }
    static QMap<QString, QVector<QString>> mapStatus2FindString = {
        {"active", {"managed", "active"} },
        {"inactive", {"managed", "inactive"} },
        {"unmanaged", {"unmanaged"} },
    };
    QVector<QString> vec{};
    if (!status.isEmpty() && mapStatus2FindString.contains(status)) {
        vec = *(mapStatus2FindString.find(status));
    } else {
        vec = *(mapStatus2FindString.find("active"));
    }

    if (val.isObject()) {
        const QJsonObject& valObj = val.toObject();
        for (const auto& k : valObj.keys()) {
            if (std::find(vec.begin(), vec.end(), k) != vec.end()) {
                const QJsonValue& v = valObj[k];
                if (!v.isUndefined() && !v.isNull()) {
                    return v.toVariant();
                }
            }
        }
    } else if (!val.isNull()) {
        return val.toVariant();
    }

    return QVariant();
}

template<typename T>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, T& config, const T& defaultValue)
{
    // QVariant result = paserAttribute(jsonObj, key, status);
    // if (std::is_same<T, qreal>::value) {
    //     config = result.isValid() ? result.toDouble() : defaultValue;
    // } else if (std::is_same<T, int>::value) {
    //     config = result.isValid() ? result.toInt() : defaultValue;
    // }
}

template<>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, double& config, const double& defaultValue)
{
    QVariant result = paserAttribute(jsonObj, key, status);
    config = result.isValid() ? result.toDouble() : defaultValue;
}

template<>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, int& config, const int& defaultValue)
{
    QVariant result = paserAttribute(jsonObj, key, status);
    config = result.isValid() ? result.toInt() : defaultValue;
}

template<>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, QColor& config, const QColor& defaultValue)
{
    QVariant result = paserAttribute(jsonObj, key, status);
    config = takeColor(result, defaultValue);
}

template<>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, Qt::Edge& config, const Qt::Edge& defaultValue)
{
    QVariant result = paserAttribute(jsonObj, key, status);
    config = result.isValid() ? static_cast<Qt::Edge>(result.toInt()) : defaultValue;
}

template<>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, QPointF& config, const QPointF& defaultValue)
{
    QVariant result = paserAttribute(jsonObj, key, status);
    config = ChameleonTheme::takePos(result, defaultValue);
}

template<>
void paserSpecialAttribute(const QJsonObject& jsonObj, const QString& key, const QString& status, QMarginsF& config, const QMarginsF& defaultValue)
{
    QVariant result = paserAttribute(jsonObj, key, status);
    config = ChameleonTheme::takeMargins(result, defaultValue);
}

static void parserWindowDecoration(const UIWindowType& windowType, const QJsonValue& windowDecoration, const QString& status,
                                    ChameleonTheme::ThemeConfig *config, const ChameleonTheme::ThemeConfig *base)
{
    const QJsonObject& windowDecObj = windowDecoration.toObject();
    config->windowDesc = windowDecObj.value("desc").toString();
    qCDebug(CHAMELEON) << "parser window: " << config->windowDesc;

    //tilebar
    const QJsonObject titlebarObj = windowDecObj.value("titlebar").toObject();
    //unmanaged窗口没有titlebar
    if (status != "unmanaged" && !titlebarObj.isEmpty()) {
        paserSpecialAttribute<qreal>(titlebarObj, "height", status, config->titlebarConfig.height, base ? base->titlebarConfig.height : 50.0);
        paserSpecialAttribute<Qt::Edge>(titlebarObj, "area", status, config->titlebarConfig.area, base ? base->titlebarConfig.area : Qt::TopEdge);
        paserSpecialAttribute<QColor>(titlebarObj, "bgcolor", status, config->titlebarConfig.backgroundColor, base ? base->titlebarConfig.backgroundColor : QColor());

        //font
        paserSpecialAttribute<QString>(titlebarObj, "font-family", status, config->titlebarConfig.font.fontFamily, base ? base->titlebarConfig.font.fontFamily : QString());
        paserSpecialAttribute<int>(titlebarObj, "font-size", status, config->titlebarConfig.font.fontSize, base ? base->titlebarConfig.font.fontSize : 14);
        paserSpecialAttribute<QString>(titlebarObj, "text-align", status, config->titlebarConfig.font.textAlign, base ? base->titlebarConfig.font.textAlign : QString("center"));
        paserSpecialAttribute<QColor>(titlebarObj, "text-color", status, config->titlebarConfig.font.textColor, base ? base->titlebarConfig.font.textColor : QColor());

        //button-group
        const QJsonObject& btnObj = titlebarObj.value("button-group").toObject();
        if (!btnObj.isEmpty()) {
            auto parseTitleBarButton = [&](const QJsonObject& btnObj, ChameleonTheme::ThemeConfig::TitlebarBtn& configTitlebarBtn, const ChameleonTheme::ThemeConfig::TitlebarBtn& baseTitlebarBtn) {
                paserSpecialAttribute<QPointF>(btnObj, "pos", status, configTitlebarBtn.pos, base ? baseTitlebarBtn.pos : QPointF(0.0, 0.0));
                paserSpecialAttribute<int>(btnObj, "width", status, configTitlebarBtn.width, base ? baseTitlebarBtn.width : 50);
                paserSpecialAttribute<int>(btnObj, "height", status, configTitlebarBtn.height, base ? baseTitlebarBtn.height : 50);
            };
            for (const auto& item : btnObj.keys()) {
                if (item == "menu") {
                    const QJsonObject& menuObj = btnObj.value(item).toObject();
                    parseTitleBarButton(menuObj, config->titlebarConfig.menuBtn, base->titlebarConfig.menuBtn);
                    config->titlebarConfig.menuBtn.btnIcon = takeIcon(menuObj.value("icon").toObject(), QIcon(), ":/deepin/themes/deepin/light/icons/menu");
                } else if (item == "minimize") {
                    const QJsonObject& minimizeObj = btnObj.value(item).toObject();
                    parseTitleBarButton(minimizeObj, config->titlebarConfig.minimizeBtn, base->titlebarConfig.minimizeBtn);
                    config->titlebarConfig.minimizeBtn.btnIcon = takeIcon(minimizeObj.value("icon").toObject(), QIcon(), ":/deepin/themes/deepin/light/icons/minimize");
                } else if (item == "maximize") {
                    const QJsonObject& maximizeObj = btnObj.value(item).toObject();
                    parseTitleBarButton(maximizeObj, config->titlebarConfig.maximizeBtn, base->titlebarConfig.maximizeBtn);
                    config->titlebarConfig.maximizeBtn.btnIcon = takeIcon(maximizeObj.value("icon").toObject(), QIcon(), ":/deepin/themes/deepin/light/icons/maximize");
                } else if (item == "unmaximize") {
                    const QJsonObject& unmaximizeObj = btnObj.value(item).toObject();
                    parseTitleBarButton(unmaximizeObj, config->titlebarConfig.unmaximizeBtn, base->titlebarConfig.unmaximizeBtn);
                    config->titlebarConfig.unmaximizeBtn.btnIcon = takeIcon(unmaximizeObj.value("icon").toObject(), QIcon(), ":/deepin/themes/deepin/light/icons/unmaximize");
                } else if (item == "close") {
                    const QJsonObject& closeObj = btnObj.value(item).toObject();
                    parseTitleBarButton(closeObj, config->titlebarConfig.closeBtn, base->titlebarConfig.closeBtn);
                    config->titlebarConfig.closeBtn.btnIcon = takeIcon(closeObj.value("icon").toObject(), QIcon(), ":/deepin/themes/deepin/light/icons/close");
                }
            }
        }
    }

    //rounded-corner-radius TODO
    paserSpecialAttribute<QPointF>(windowDecObj, "rounded-corner-radius", status, config->radius, base ? base->radius : QPointF(18.0, 18.0));

    //blur
    paserSpecialAttribute<qreal>(windowDecObj, "blur", status, config->blur, base ? base->blur : 20.0);

    //opcaity
    paserSpecialAttribute<qreal>(windowDecObj, "opcaity", status, config->opacity, base ? base->opacity : 20.0);

    //mouseInputAreaMargins
    paserSpecialAttribute<QMarginsF>(windowDecObj, "mouseInputAreaMargins", status, config->mouseInputAreaMargins, base ? base->mouseInputAreaMargins : QMarginsF(10, 10, 10, 10));

    //shadow
    const QJsonObject& shadowObj = windowDecObj.value("shadow").toObject();
    paserSpecialAttribute<qreal>(shadowObj, "shadowRadius", status, config->shadowConfig.shadowRadius, base ? base->shadowConfig.shadowRadius : 60.0);
    paserSpecialAttribute<QColor>(shadowObj, "shadowColor", status, config->shadowConfig.shadowColor, base ? base->shadowConfig.shadowColor : QColor(0, 0, 0, 255 * 0.6));
    paserSpecialAttribute<QPointF>(shadowObj, "shadowOffset", status, config->shadowConfig.shadowOffset, base ? base->shadowConfig.shadowOffset : QPointF(0.0, 16.0));

    //border
    const QJsonObject borderObj = windowDecObj.value("border").toObject();
    //border width
    paserSpecialAttribute<qreal>(borderObj, "width", status, config->borderConfig.borderWidth, base ? base->borderConfig.borderWidth : 1.0);

    //border color
    paserSpecialAttribute<QColor>(borderObj, "color", status, config->borderConfig.borderColor, base ? base->borderConfig.borderColor : QColor(0, 0, 0, 255 * 0.15));
}

static void writeConfig(const UIWindowType& windowType, const QJsonValue& windowDecoration, ChameleonTheme::ConfigGroupMap *configs, const ChameleonTheme::ConfigGroupMap *base)
{
    const ChameleonTheme::ThemeConfig* base_active = nullptr;
    const ChameleonTheme::ThemeConfig* base_inactive = nullptr;
    if (base) {
        if (base->managedConfigGroupMap.contains(windowType)) {
            const auto& it = base->managedConfigGroupMap.find(windowType);
            base_active = &(it->normal);
            base_inactive = &(it->inactive);
        }
        if (base->unmanagedConfigMap.contains(windowType)) {
            const auto& it = base->unmanagedConfigMap.find(windowType);
            base_active = &*it;
        }
    }

    ChameleonTheme::ConfigGroup managedConfigGroup;
    ChameleonTheme::ThemeConfig unmanagedConfig;
    ChameleonTheme::ThemeConfig* config_active = &managedConfigGroup.normal;
    ChameleonTheme::ThemeConfig* config_inactive = &managedConfigGroup.inactive;
    ChameleonTheme::ThemeConfig* config_unmanaged = &unmanagedConfig;

    parserWindowDecoration(windowType, windowDecoration, "active", config_active, base_active);
    parserWindowDecoration(windowType, windowDecoration, "inactive", config_inactive, config_active);
    parserWindowDecoration(windowType, windowDecoration, "unmanaged", config_unmanaged, config_active);

    configs->managedConfigGroupMap.insert(windowType, managedConfigGroup);
    configs->unmanagedConfigMap.insert(windowType, unmanagedConfig);
}

static bool loadTheme(ChameleonTheme::ConfigGroupMap *configs, const ChameleonTheme::ConfigGroupMap *base,
                      ChameleonTheme::ThemeType themeType, const QString &themeName, const QList<QDir> &themeDirList)
{
    QDir theme_dir("/");
    for (const QDir &dir : themeDirList) {
        for (const QFileInfo &info : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (info.fileName() == themeName) {
                theme_dir.setPath(info.filePath());
                break;
            }
        }
    }

    if (theme_dir.path() == "/")
        return false;

    const QString themeJsonPath = theme_dir.filePath(ChameleonTheme::typeString(themeType) + "/decoration.json");
    QFile f(themeJsonPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qCCritical(CHAMELEON) << "Could not open file: " << themeJsonPath;
        return false;
    }

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCCritical(CHAMELEON) << "Failed to parse" << themeJsonPath << error.errorString();
        return false;
    }
    qCDebug(CHAMELEON) << "Begin to parse" << themeJsonPath;

    const QJsonObject rootObj = doc.object();
    double version = rootObj.value("version").toDouble();
    qCDebug(CHAMELEON) << "chameleon theme version: " << version;
    QString name = rootObj.value("name").toString();
    qCDebug(CHAMELEON) << "chameleon theme name: " << name;

    for (const auto& key : rootObj.keys()) {
        qCDebug(CHAMELEON) << "parser chameleon theme window type " << key;
        int uiWindowTypeId = key.toInt();
        if (uiWindowTypeId == 0) {
            continue;
        }
        const QJsonObject& windowDecObj = rootObj[key].toObject();
        if (uiWindowTypeId == static_cast<int>(UIWindowType::Normal)) {
            writeConfig(UIWindowType::Normal, windowDecObj, configs, base);
        } else if (uiWindowTypeId == static_cast<int>(UIWindowType::Dialog)) {
            writeConfig(UIWindowType::Dialog, windowDecObj, configs, base);
        } else if (uiWindowTypeId == static_cast<int>(UIWindowType::Dock)) {
            writeConfig(UIWindowType::Dock, windowDecObj, configs, base);
        } else if (uiWindowTypeId == static_cast<int>(UIWindowType::PopupMenu)) {
            writeConfig(UIWindowType::PopupMenu, windowDecObj, configs, base);
        } else if (uiWindowTypeId == static_cast<int>(UIWindowType::DropdownMenu)) {
            writeConfig(UIWindowType::DropdownMenu, windowDecObj, configs, base);
        } else if (uiWindowTypeId == static_cast<int>(UIWindowType::Tooltip)) {
            writeConfig(UIWindowType::Tooltip, windowDecObj, configs, base);
        }

    }

    return true;
}

static bool formatThemeName(const QString &fullName, ChameleonTheme::ThemeType &type, QString &name)
{
    int split = fullName.indexOf("/");

    if (split > 0 && split < fullName.size() - 1) {
        type = ChameleonTheme::typeFromString(fullName.left(split));
        name = fullName.mid(split + 1);

        return true;
    }

    return false;
}

ChameleonTheme::ConfigGroupMapPtr ChameleonTheme::loadTheme(const QString &themeFullName, const QList<QDir> themeDirList)
{
    ThemeType type;
    QString name;

    if (!formatThemeName(themeFullName, type, name)) {
        return ConfigGroupMapPtr();
    }

    return loadTheme(type, name, themeDirList);
}

ChameleonTheme::ConfigGroupMapPtr ChameleonTheme::loadTheme(ThemeType themeType, const QString &themeName, const QList<QDir> themeDirList)
{
    auto base = getBaseConfig(themeType, themeDirList);

    if (themeName == BASE_THEME)
        return base;

    auto *newConfigMap = new ConfigGroupMap();
    bool ok = ::loadTheme(newConfigMap, base.data(), themeType, themeName, themeDirList);

    if (ok) {
        return ChameleonTheme::ConfigGroupMapPtr(newConfigMap);
    } else {
        delete newConfigMap;
    }

    return ConfigGroupMapPtr(nullptr);
}

ChameleonTheme::ConfigGroupMapPtr ChameleonTheme::getBaseConfig(ChameleonTheme::ThemeType type, const QList<QDir> &themeDirList)
{
    //static ConfigGroupMapPtr base_configs[ThemeTypeCount];
    static ConfigGroupMapPtr baseConfigs[ThemeTypeCount];
    if (!baseConfigs[type]) {
        auto *base = new ConfigGroupMap;
        // 先从默认路径加载最基本的主题
        ::loadTheme(base, nullptr, type, BASE_THEME, {QDir(BASE_THEME_DIR)});
        // 再尝试从其它路径加载主题，以允许基本主题中的值可以被外界覆盖
        ::loadTheme(base, base, type, BASE_THEME, themeDirList);
        // 将对应类型的基础主题缓存
        baseConfigs[type] = ConfigGroupMapPtr(base);
    }

    return baseConfigs[type];
}

QString ChameleonTheme::typeString(ChameleonTheme::ThemeType type)
{
    return type == Dark ? "dark" : "light";
}

ChameleonTheme::ThemeType ChameleonTheme::typeFromString(const QString &type)
{
    if (type == "dark") {
        return Dark;
    }

    return Light;
}

QString ChameleonTheme::theme() const
{
    return m_theme;
}

bool ChameleonTheme::setTheme(const QString &themeFullName)
{
    ThemeType type;
    QString name;

    if (!formatThemeName(themeFullName, type, name)) {
        return false;
    }

    return setTheme(type, name);
}

bool ChameleonTheme::setTheme(ThemeType type, const QString &theme)
{
    if (m_type == type && m_theme == theme)
        return true;

    ConfigGroupMapPtr configs = loadTheme(type, theme, m_themeDirList);

    if (configs) {
        m_type = type;
        m_theme = theme;
        m_configGroupMap = configs;
    }

    return configs;
}

ChameleonTheme::ConfigGroupMapPtr ChameleonTheme::loadTheme(const QString &themeFullName)
{
    return loadTheme(themeFullName, m_themeDirList);
}

ChameleonTheme::ConfigGroupMapPtr ChameleonTheme::themeConfig() const
{
    return m_configGroupMap;
}

ChameleonTheme::ConfigGroup* ChameleonTheme::themeConfig(NET::WindowType windowType) const
{
    const auto it = mapNETWindowType2UIWindowType.find(windowType);
    UIWindowType uiWindowType;
    if (it != mapNETWindowType2UIWindowType.end()) {
        uiWindowType = *it;
    }

    auto itt = m_configGroupMap.data()->managedConfigGroupMap.find(uiWindowType);
    if (itt != m_configGroupMap.data()->managedConfigGroupMap.end()) {
        return const_cast<ChameleonTheme::ConfigGroup*>(&*itt);
    }

    return const_cast<ChameleonTheme::ConfigGroup*>(&*(m_configGroupMap.data()->managedConfigGroupMap.find(UIWindowType::Normal)));
}

ChameleonTheme::ThemeConfig* ChameleonTheme::themeUnmanagedConfig(NET::WindowType windowType) const
{
    const auto it = mapNETWindowType2UIWindowType.find(windowType);
    UIWindowType uiWindowType;
    if (it != mapNETWindowType2UIWindowType.end()) {
        uiWindowType = *it;
    }

    auto itt = m_configGroupMap.data()->unmanagedConfigMap.find(uiWindowType);
    if (itt != m_configGroupMap.data()->unmanagedConfigMap.end()) {
        return const_cast<ChameleonTheme::ThemeConfig*>(&*itt);
    }

    return const_cast<ChameleonTheme::ThemeConfig*>(&*(m_configGroupMap.data()->unmanagedConfigMap.find(UIWindowType::Normal)));
}

ChameleonTheme::ChameleonTheme()
{
    for (const QString &data_path : QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                              "deepin/themes",
                                                              QStandardPaths::LocateDirectory)) {
        m_themeDirList.prepend(QDir(data_path));
    }

    // 默认主题
    setTheme(ThemeType::Light, BASE_THEME);
}

ChameleonTheme::~ChameleonTheme()
{

}
