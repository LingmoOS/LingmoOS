#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "OobeWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("lingmo-oobe");

    // Load translations
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale::system(), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);

    QTranslator appTranslator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString lang = QLocale(locale).name();
        if (appTranslator.load(":/i18n/lingmo-oobe_" + lang)) {
            app.installTranslator(&appTranslator);
            break;
        }
    }

    // Check if OOBE already done
    if (QFile::exists("/etc/.firstboot-complete"))
        return 0;

    OobeWindow w;
    w.show();

    return app.exec();
}
