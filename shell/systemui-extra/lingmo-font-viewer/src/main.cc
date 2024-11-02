#include <fcntl.h>
#include <syslog.h>

#include <QApplication>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QTranslator>
#include <QObject>
#include <QFileInfo>

/* 适配SDK */
#include <singleapplication.h>
#include "log.hpp"
#include "windowmanage.hpp"

#include "src/view/mainview.h"

int main(int argc , char *argv[])
{
    /* 使用sdk管理日志 */
    qInstallMessageHandler(::kabase::Log::logOutput);

    /* 适配4K屏以及分数缩放 */
    ::kabase::WindowManage::setScalingProperties();

    //加在最上面的原因：防止QApplication将(-title)参数吞掉，导致拿不到
    QString fontFileName = "";
    if (argc > 1) {
        for(int i=1; i<argc; i++) {
            QString fontFile = argv[i];
            QFileInfo f(fontFile);
            if(f.isFile())
                fontFileName = fontFileName + fontFile;

            if (i < (argc - 1)) {
                fontFileName = fontFileName + "/n/n";
            }

        }
    }
    

    /* 适配4K屏以及分数缩放 */
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif

    kdk::QtSingleApplication app(argc, argv);
    app.setApplicationVersion("1.1.0");
    app.setWindowIcon(QIcon::fromTheme("lingmo-font"));

    if (app.isRunning()) {
        qDebug() << "is running";
        app.sendMessage(fontFileName);
        return 0;
    }

    /* 设置不使用gtk3平台文件对话框 */
    app.setProperty("useFileDialog", false);

    /* 文件锁实现VNC单例 */
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    int fd = open(QString(homePath.at(0) + "/.config/lingmo-font-viewer%1.lock").arg(getenv("DISPLAY")).toUtf8().data() , O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR);
    if (fd < 0) {
        exit(1);
    }

    if (lockf(fd , F_TLOCK , 0)) {
        syslog(LOG_ERR , "Can't lock single file, lingmo-font-viewer is already running!");
        exit(-1);
    }

    /* 加载翻译文件 */
    QString lingmoFontViewerTransPath = "/usr/share/lingmo-font-viewer/data/translations";
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QString locale = QLocale::system().name();

    QTranslator trans_global , trans_qt;
    QTranslator trans_SDK;

    if (!trans_global.load(QLocale(), "lingmo-font-viewer", "_", lingmoFontViewerTransPath)) {
        qWarning() << "Load global translations file" <<QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global);
    }

    if (!trans_qt.load(QLocale(), "qt", "_", qtTransPath)) {
        qWarning() << "Load qt translations file" <<QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_qt);
    }

    QString path = QString(":/translations/gui_%1.qm").arg(locale);
    if (trans_SDK.load(path)) {
        app.installTranslator(&trans_SDK);
    }

    /* 移除标题栏 */
    ::kabase::WindowManage::removeHeader(MainView::getInstance());

    /* 最小化拉起 */
    app.setActivationWindow(MainView::getInstance());

    /* wayland 下最小化拉起 */
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [=]() {
            kabase::WindowManage::activateWindow(MainView::getInstance()->m_windowId);
        });
        
    }

    MainView::getInstance()->show();  
    ::kabase::WindowManage::setMiddleOfScreen(MainView::getInstance());

    MainView::getInstance()->getFontFileFromClick(fontFileName);
    
    QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, MainView::getInstance(), &MainView::getFontFileFromClickAferOpen);

    return app.exec();
}
