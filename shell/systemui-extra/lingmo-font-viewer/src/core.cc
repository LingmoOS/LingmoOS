#include "include/core.h"

#include <QDebug>
#include <QList>
#include <QStandardPaths>

Core::Core()
{
    this->slotStart();
}

Core::~Core()
{

}

Core *Core::getInstance(void)
{
    static Core instance;
    return &instance;
}

bool Core::init(void)
{
    this->m_installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/";

    this->m_fontData = LibFun::getAllFontInformation();

    QList<QString> collectFontPaths = this->m_external.getCollectFont();
    for (int i = 0 ; i < collectFontPaths.count() ; i++) {
        QString path = collectFontPaths.at(i);
        for (int j = 0 ; j < this->m_fontData.count() ; j++) {
            if (this->m_fontData.at(j).path == path) {
                this->m_collectFontData << this->m_fontData.at(j);
                break;
            }
        }
    }

    return true;
}

QList<ViewData> Core::getViewData(void)
{
    QList<ViewData> ret;
    ret.clear();

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        ViewData item;
        FontInformation tmp = this->m_fontData.at(i);

        item.path = tmp.path;
        item.family = tmp.family;
        item.style = tmp.style;

        ret << item;
    }

    return ret;
}

FontInformation Core::getFontDetails(QString path)
{
    FontInformation ret;

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        if (this->m_fontData.at(i).path == path) {
            ret = this->m_fontData.at(i);

            break;
        }
    }

    return ret;
}

QList<ViewData> Core::getSystemFont(void)
{
    QList<ViewData> ret;

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        FontInformation item = this->m_fontData.at(i);

        if (!item.path.contains(this->m_installPath)) {
            ViewData tmp;
            tmp.path = item.path;
            tmp.family = item.family;
            tmp.style = item.style;

            ret << tmp;
        }
    }

    return ret;
}

QList<ViewData> Core::getMyFont(void)
{
    QList<ViewData> ret;

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        FontInformation item = this->m_fontData.at(i);

        if (item.path.contains(this->m_installPath)) {
            ViewData tmp;
            tmp.path = item.path;
            tmp.family = item.family;
            tmp.style = item.style;

            ret << tmp;
        }
    }

    return ret;
}

QList<ViewData> Core::getCollectFont(void)
{
    QList<ViewData> ret;

    for (int i = 0 ; i < this->m_collectFontData.count() ; i++) {
        FontInformation item = this->m_collectFontData.at(i);
        ViewData tmp;
        tmp.path = item.path;
        tmp.family = item.family;
        tmp.style = item.style;

        ret << tmp;
    }

    return ret;
}

bool Core::unCollectFont(QString path)
{
    for (int i = 0 ; i < this->m_collectFontData.count() ; i++) {
        FontInformation item = this->m_collectFontData.at(i);

        if (item.path == path) {
            if (!this->m_external.removeCollectFont(item.path)) {
                qCritical() << "Error : Core , unCollectFont , font cancel collect fail";
                return false;
            }

            this->m_collectFontData.removeAt(i);

            break;
        }
    }

    return true;
}

bool Core::collectFont(QString path)
{
    FontInformation item;

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        if (this->m_fontData.at(i).path == path) {
            item = this->m_fontData.at(i);

            break;
        }

    }

    if (item.path.isEmpty()) {
        qCritical() << "Error : Core , collectFont , font not found";
        return false;
    }

    if (!this->m_external.addCollectFont(item.path)) {
        qCritical() << "Error : Core , collectFont , font collect fail";
        return false;
    }

    this->m_collectFontData << item;

    return true;
}

bool Core::installFont(QString path)
{
    if (!LibFun::installFont(path)) {
        qCritical() << "Error : Core , installFont , font install fail";

        return false;
    }

    return true;
}

bool Core::uninstallFont(QString path, QString fontName)
{            
    qDebug() << "remove font :" << fontName;
    if (m_external.removeFontFormContro(fontName) == false) {
        return false;
    }

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        if (this->m_fontData.at(i).path == path) {
            if (!LibFun::uninstallFont(this->m_fontData.at(i).path)) {
                qCritical() << "Error : Core , uninstallFont , font uninstall fail";

                return false;
            }

            for (int j = 0 ; j < this->m_collectFontData.count() ; j++) {
                if (this->m_collectFontData.at(j).path == path) {
                    if (!this->m_external.removeCollectFont(this->m_collectFontData.at(j).path)) {
                        qWarning() << "Waring : Core , remove collect font fail";
                    }

                    this->m_collectFontData.removeAt(j);

                    break;
                }
            }

            this->m_fontData.removeAt(i);

            break;
        }
    }

    return true;
}

bool Core::copyFont(QString srcPath , QString dstPath)
{
    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        if (this->m_fontData.at(i).path == srcPath) {
            if (!LibFun::copyFont(this->m_fontData.at(i).path , dstPath)) {
                qCritical() << "Error : Core , copyFont , copy font fail";

                return false;
            }

            break;
        }
    }

    return true;
}

bool Core::isSystemFont(QString path)
{
    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        FontInformation item = this->m_fontData.at(i);

        if (item.path == path) {
            if (item.path.contains(this->m_installPath)) {
                return false;
            } else {
                return true;
            }
        }
    }

    return false;
}

bool Core::applyFont(QString family, QString path)
{
    QString foundryLast = ""; // 上个字体铸造商
    QString foundryCurrent = ""; // 当前字体铸造商
    QString foundryTarget = ""; // 目标字体铸造商
    bool firstFont = true;

    QString fontFamily = "";
    bool isDiffFontExist = false; // 存在名字相同，铸造厂商不同的字体（默认为false）
    // bool isSameFontExist = false;  // 存在名字相同，铸造厂商相同的字体 （默认为false）

    // 循环查找列表
    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        FontInformation item = this->m_fontData.at(i);
        
        // 寻找目标字体，判断铸造商是否存在不同
        if (item.family == family) {
            qDebug() << item.foundry << item.family << item.path;
            foundryCurrent = item.foundry;
            // 判断当前是否为第一个被找到的字体文件
            if (firstFont) {
                firstFont = false;
                foundryLast = item.foundry;
            }
            // 比较上一个字体和当前的字体的铸造商是否存在不同
            if (QString::compare(foundryLast, foundryCurrent) != 0) {
                isDiffFontExist = true;
            }
            if (item.path == path) {
                foundryTarget = item.foundry;
            }
            foundryLast = foundryLast;
        }
    }
    if (!isDiffFontExist) {
        fontFamily = family;
    } else {
        fontFamily = family + " [" + foundryTarget + "]";
    }

    return this->m_external.applyFont(fontFamily);
}

void Core::slotStart(void)
{
    if (!this->init()) {
        qCritical() << "Error : Core , Core , init fail";

        return;
    }

    return;
}

void Core::printFontData(void)
{
    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        FontInformation item = this->m_fontData.at(i);

        qDebug() << "**************************************************************************************************************";

        qDebug() << " path : " << item.path << " name : " << item.name << " famile : " << item.family << "\n"
                 << " style : " << item.style << " type : " << item.type << " version " << item.version << "\n"
                 << " copyright : " << item.copyright << "\n"
                 << " manufacturer : " << item.manufacturer << "\n"
                 << " description : " << item.description << "\n"
                 << " designer : " << item.designer << "\n"
                 << " license : " << item.license;
    }

    qDebug() << "**************************************************************************************************************";

    return;
}

QString Core::ControlUsingFont()
{
    QString font = m_external.isUsingFont();
    qDebug() << "当前正在使用的字体是： " << font;
    return font;
}

void Core::updateFontList()
{
    /* PRO : 此处先重新获取全部字体信息来处理 , 后续优化为获取单个字体信息 */
    this->m_fontData = LibFun::getAllFontInformation();
    return;
}

bool Core::judgeBadFontFile(QString path)
{
    // 判断文件是否为损坏文件
    if (LibFun::getFontInfo(path) == path) {
        return true;
    }
    
    return false;
}

bool Core::getSettingFontTips()
{
    bool setTips = m_external.getFontTipsData();
    return setTips;
}

bool Core::settingFontTips(bool state)
{
    bool setTips = m_external.setFontIntoList(state);
    return setTips;
}
