#include "utils.h"
#include <random>

Utils::Utils()
{

}
/**
 * @brief 移到鼠标所在屏幕中央。兼容990
 */
void Utils::centerToScreen(QWidget *widget)
{
    if (!widget)

       return;

     QDesktopWidget* m = QApplication::desktop();

     QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));

     int desk_x = desk_rect.width();

     int desk_y = desk_rect.height();

     int x = widget->width();

     int y = widget->height();

     widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

QString Utils::loadFontFamilyFromTTF()
{
    static QString font;
    static bool loaded = false;
    if(!loaded)
    {
        loaded = true;
        int loadedFontID = QFontDatabase::addApplicationFont(":/image/DFPKingGothicGB-Semibold-2.ttf");
        QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        if(!loadedFontFamilies.empty())
            font = loadedFontFamilies.at(0);
    }
    return font;
}

QString Utils::loadFontHuaKangJinGangHeiRegularTTF()
{
    static QString font;
    static bool loaded = false;
    if(!loaded)
    {
        loaded = true;
        int loadedFontID = QFontDatabase::addApplicationFont(":/image/HuaKangJinGangHei-Regular-2.ttf");
        QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        if(!loadedFontFamilies.empty())
            font = loadedFontFamilies.at(0);
    }
    return font;
}

QString Utils::getRandomId()
{
    //time(0) 随机种子
    static std::default_random_engine e(time(0));
    //5位数
    static std::uniform_int_distribution<unsigned> u(10000, 99999);
    //转字符串
    return QString::number(u(e));
}

QIcon Utils::getQIcon(QString themeUrl, QString localUrl)
{
    QIcon icon = QIcon::fromTheme(themeUrl);
    if(icon.isNull()){
        qDebug()<<"dbq-空的url"<<themeUrl;
        icon = QIcon(localUrl);
    }
    return icon;
}
QString Utils::getOmitStr(QString str, int sizeLimit)
{
    if(str.length()>sizeLimit){
        str = str.left(sizeLimit);
        str = str+"...";
    }
    return str;
}

int Utils::handelColorRange(int value)
{
    if(value<0){
        return 0;
    }else if(value>255){
        return 255;
    }else{
        return value;
    }
}

bool Utils::checkLocalChina()
{
    return QLocale().name()=="zh_CN";
}




