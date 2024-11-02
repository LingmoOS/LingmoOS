
//*************************************
//代优化列表:
//动态加载；------已完成
//裁剪绘制部分：1）代码拆分，细化功能，目前该函数下代码行数过多；2）有些情况可以合并，需要时间来优化这里；
//优先mimetype读取而非后缀；
//界面qss整理：工具栏按钮的qss设置，主界面部分；
//缩略图读取；
//一些支持查看但不支持操作并保存的图片格式会陆续加进来
//代码文件开头补充开源协议
//看图-ocr，看图识字
// svg-放大缩小单独svg处理
//*************************************

#include "view/kyview.h"
#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include "global/log.h"
#include "controller/interaction.h"
#include <QLoggingCategory>
#include "view/openimage.h"
#include <window_management.hpp>
#include <lingmostylehelper/lingmostylehelper.h>
#include "windowmanager/windowmanager.h"
#include <log.hpp>

int main(int argc, char *argv[])
{
    //适配4K屏以及分数缩放
    kdk::kabase::WindowManagement::setScalingProperties();
    //日志输出
   // qInstallMessageHandler(::kabase::KabaseLog::logOutput);
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
    QApplication a(argc, argv);
    qApp->setWindowIcon(QIcon::fromTheme("lingmo-photo-viewer", QIcon(":/res/res/kyview_logo.png")));

    //翻译
    QTranslator app_trans;
    QTranslator qt_trans;
    QString locale = QLocale::system().name();
    QString trans_path;
    if (QDir("/usr/share/lingmo-photo-viewer/translations").exists()) {
        trans_path = "/usr/share/lingmo-photo-viewer/translations";
    } else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
    QString qt_trans_path;
    qt_trans_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath); // /usr/share/qt5/translations
    if (!qt_trans.load("qt_" + locale + ".qm", qt_trans_path)) {
        qDebug() << "Load translation file："
                 << "qt_" + locale + ".qm from" << qt_trans_path << "failed!";
    } else {
        a.installTranslator(&qt_trans);
    }
    QTranslator app_trans_penoy;
    app_trans_penoy.load("/usr/share/libexplor-qt/libexplor-qt_" + QLocale::system().name());
    a.installTranslator(&app_trans_penoy);
    QTranslator sdk_trans;
    if (sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        a.installTranslator(&sdk_trans);
    }
    if (!app_trans.load("lingmo-photo-viewer_" + locale + ".qm", trans_path)) {
        qDebug() << "Load translation file："
                 << "lingmo-photo-viewer_" + locale + ".qm from" << trans_path << "failed!";
    } else {
        a.installTranslator(&app_trans);
    }

    // 设置线程池最大线程数
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    a.setApplicationName(QApplication::tr("Pictures"));
    //主题框架
    KyView w(a.arguments());
    kdk::LingmoUIStyleHelper::self()->removeHeader(&w);
    Interaction::getInstance()->initUiFinish();
    return a.exec();
}
