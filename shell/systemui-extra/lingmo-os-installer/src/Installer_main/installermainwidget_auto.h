#ifndef INSTALLERMAINWIDGET_AUTO_H
#define INSTALLERMAINWIDGET_AUTO_H

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QThread>
#include <QStackedWidget>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginDll/plugindll.h"
#include "installingframe.h"
#include "finishedInstall.h"
#include "../PluginService/backendThread/quickinsatllerprocess.h"
#include "installingoemconfigframe.h"
namespace KInstaller{

class InstallerMainWidget_auto : public QWidget
{
    Q_OBJECT

public:
    InstallerMainWidget_auto(bool oemflag, QWidget *parent = 0);
    ~InstallerMainWidget_auto();

private:
    void initUI();                              //加载界面ui
    void initBackground();                      //背景初始化
    void addStyleSheet();                       //设置样式
    void initAllConnect();


private:

    QStackedWidget* m_stackWidget = nullptr;
    bool bloemflag;
    InstallingFrame* m_installingFrame = nullptr;
    QThread* m_backQuickInstallThread = nullptr;
    QuickInsatllerProcess* m_backquickProcess = nullptr;
    InstallingOEMConfigFrame* m_oemFrame = nullptr;

public slots:

    void showInstallingFrame();
    void showInstallingErrorFrame();
    void showFinishedFrame();


signals:
    void signalScriptFinished();
    void signalInstallingClose();

};
}
#endif // INSTALLERMAINWIDGET_AUTO_H
