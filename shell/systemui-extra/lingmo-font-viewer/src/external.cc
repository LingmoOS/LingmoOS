#include <QDebug>
#include <QProcess>

#include "include/external.h"
#include "src/view/globalsizedata.h"

#define LINGMO_FONT_VIEWER_GSETTINGS "org.lingmo.font.viewer.gsettings"
#define COLLECT_FONT_KEY "collect-font"
#define TIPS_FONT_KEY "setting-font-tips"
#define THEME_GSETTINGS "org.lingmo.style"
#define APPLY_FONT_KEY "system-font"
#define LINGMO_CONTROL_CENTER_GSETTINGS "org.lingmo.control-center"
#define FONTS_LIST "fonts-list"
#define LINGMO_GTK_GSTTINGS "org.mate.interface"
#define FONT_NAME "font-name"
#define DOC_FONT_KEY "document-font-name" 

External::External()
{
    if (QGSettings::isSchemaInstalled(LINGMO_FONT_VIEWER_GSETTINGS)) {
        this->m_gsettings = new QGSettings(LINGMO_FONT_VIEWER_GSETTINGS);
    } else {
        qCritical() << "Error : External , External , create gsettings fail";
    }

    if (QGSettings::isSchemaInstalled(THEME_GSETTINGS)) {
        this->m_themeGsettings = new QGSettings(THEME_GSETTINGS);
    } else {
        qCritical() << "Error : External , External , create theme gsettings fail";
    }

    if (QGSettings::isSchemaInstalled(LINGMO_CONTROL_CENTER_GSETTINGS)) {
        this->m_contolGsettings = new QGSettings(LINGMO_CONTROL_CENTER_GSETTINGS);
    } else {
        qCritical() << "Error : External , External , create control gsettings fail";
    }

    if (QGSettings::isSchemaInstalled(LINGMO_GTK_GSTTINGS)) {
        this->m_gtkGsettings = new QGSettings(LINGMO_GTK_GSTTINGS);
    } else {
        qCritical() << "Error : External , External , create GTK gsettings fail";
    }
    // m_process = new QProcess(this);
}

External::~External()
{
    if (this->m_gsettings != nullptr) {
        delete this->m_gsettings;
    }
    if (this->m_themeGsettings != nullptr) {
        delete this->m_themeGsettings;
    }
    if (this->m_contolGsettings != nullptr) {
        delete this->m_contolGsettings;
    }
    if (this->m_gtkGsettings != nullptr) {
        delete this->m_gtkGsettings;
    }
    // if (this->m_process != nullptr) {
    //     delete this->m_process;
    // }

}

QList<QString> External::getCollectFont(void)
{
    QList<QString> ret;
    ret.clear();

    if (this->m_gsettings == nullptr) {
        return ret;
    }

    QString paths = this->m_gsettings->get(COLLECT_FONT_KEY).toString();

    ret = paths.split(';');

    return ret;
}

bool External::addCollectFont(QString path)
{
    if (this->m_gsettings == nullptr) {
        return false;
    }

    QString paths = this->m_gsettings->get(COLLECT_FONT_KEY).toString();

    if (paths.isEmpty()) {
        paths.append(path);
    } else {
        paths.append(";");
        paths.append(path);
    }

    this->m_gsettings->set(COLLECT_FONT_KEY , paths);

    return true;
}

bool External::removeCollectFont(QString path)
{
    if (this->m_gsettings == nullptr) {
        return false;
    }

    QString paths = this->m_gsettings->get(COLLECT_FONT_KEY).toString();

    QList<QString> tmp = paths.split(';');
    QString dst;
    dst.clear();

    for (int i = 0 ; i < tmp.count() ; i++) {
        if (tmp.at(i) != path) {
            if (dst.isEmpty()) {
                dst.append(tmp.at(i));
            } else {
                dst.append(';');
                dst.append(tmp.at(i));
            }
        }
    }

    this->m_gsettings->set(COLLECT_FONT_KEY , dst);

    return true;
}

bool External::applyFont(QString family)
{
    qDebug() << "apply font name "<< family;
    if (setFontIntoList(family) != true) {
        return false;
    }

    QString fontStyle = family + " " + QString::number(GlobalSizeData::getInstance()->g_fontSize, 'f', 1);

    this->m_themeGsettings->set(APPLY_FONT_KEY, family);// 改变主题字体
    this->m_gtkGsettings->set(FONT_NAME, fontStyle);    // 改变GTK的字体
    this->m_gtkGsettings->set(DOC_FONT_KEY, fontStyle); // 改变GTK的文档字体

    const int fontSize = GlobalSizeData::getInstance()->g_fontSize;
    qDebug() << fontSize;
    QDBusMessage message =QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "slotFontChange");
    QList<QVariant> args;
    args.append(fontSize);
    args.append(family);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message); // 使用dbus改变GTK标题栏的字体

    // QString cmd = "lingmo-control-center -m fonts";
    // m_process->startDetached(cmd);

    return true;
}

QStringList External::getFontFromControl()
{
    QStringList fontList;
    fontList.clear();

    if (this->m_contolGsettings == nullptr) {
        qCritical() << "Error : gsettings fail , org.lingmo.control-center";
        return fontList;
    }

    fontList = this->m_contolGsettings->get(FONTS_LIST).toStringList();
    
    return fontList;
}

bool External::setFontIntoList(QString font)
{
    QStringList fontList;
    fontList.clear();
    fontList = getFontFromControl();
    if (fontList.size() == 0) {
        return false;
    }
    if (!fontList.contains(font)) {
        fontList.append(font);
    }
    m_contolGsettings->set(FONTS_LIST, fontList);
    
    return true;
}

QString External::isUsingFont()
{
    QString font = "";
    if (this->m_themeGsettings == nullptr) {
        qCritical() << "Error : gsettings fail , org.lingmo.style";
        return font;
    }
    font = this->m_themeGsettings->get(APPLY_FONT_KEY).toString();
    qDebug() << "using font name: " << font;
    
    return font;
}

bool External::removeFontFormContro(QString font)
{
    QStringList fontList;
    fontList.clear();
    fontList = getFontFromControl();
    if (fontList.size() == 0) {
        return false;
    }
    if (font == isUsingFont()) {
        return false;
    }
    if (fontList.contains(font)) {
        fontList.removeOne(font);
    }
    m_contolGsettings->set(FONTS_LIST, fontList);

    return true;
}

bool External::getFontTipsData()
{
    bool settingTips;
    if (this->m_gsettings == nullptr) {
        return false;
    }

    settingTips = this->m_gsettings->get(TIPS_FONT_KEY).toBool();

    return settingTips;
}

bool External::setFontIntoList(bool state)
{
    if (this->m_gsettings == nullptr) {
        return false;
    }

    this->m_gsettings->set(TIPS_FONT_KEY , state);
    return true;
}
