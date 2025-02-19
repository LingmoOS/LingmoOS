#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include "welcome_manager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载翻译
    QTranslator translator;
    QString locale = QLocale::system().name();
    
    // 尝试加载对应语言的翻译文件
    if (translator.load(":/translations/translations/" + locale + ".qm")) {
        a.installTranslator(&translator);
    }
    // 如果找不到完全匹配的语言，尝试加载同一语言的其他地区翻译
    else if (locale.contains("zh_")) {
        if (translator.load(":/translations/translations/zh_CN.qm")) {
            a.installTranslator(&translator);
        }
    }

    WelcomeManager w;
    w.show();

    return a.exec();
} 