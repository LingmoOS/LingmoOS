#ifndef UTILS_H
#define UTILS_H
#include <QObject>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>
#include <QFontDatabase>
#include <QIcon>


class Utils
{
public:
    Utils();
    void centerToScreen(QWidget* widget);
    static QString loadFontFamilyFromTTF();
    static QString loadFontHuaKangJinGangHeiRegularTTF();
    static QString getRandomId();
    static QIcon getQIcon(QString themeUrl,QString localUrl);
    static QString getOmitStr(QString str, int sizeLimit);
    static int handelColorRange(int value);
    static bool checkLocalChina();
};

#endif // UTILS_H
