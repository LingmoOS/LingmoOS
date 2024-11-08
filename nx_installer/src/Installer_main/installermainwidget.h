#ifndef INSTALLERMAINWIDGET_H
#define INSTALLERMAINWIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginDll/plugindll.h"
#include "installingframe.h"
#include "finishedInstall.h"
#include "installingoemconfigframe.h"
#include "../plugins/VirtualKeyboard/src/virtualkeyboard.h"
#include "../plugins/VirtualKeyboard/src/keyboardwidget.h"
#include "QTimer"

namespace KInstaller{

class InstallerMainWidget : public QWidget
{
    Q_OBJECT

public:
    InstallerMainWidget(QWidget *parent = 0);
    ~InstallerMainWidget();
    void setInstallMode(QString flagstr);


    QString m_InstallMode;
    int AllStep;
    QTimer *timer;

private:
    void initUI();                              //加载界面ui
    void initBackground();                      //背景初始化
    void initAllConnect();                      //信号和槽链接
    void addStyleSheet();                       //设置样式

    void addStackWidget();
    void setLabelDotVisible(bool flag);
    void changeLabelDotPix(int step);
    void updateButtonStatus();
    bool eventFilter(QObject *watched, QEvent *event);

    bool showOEMInstallingFrame();

private:
    QPushButton* m_backbtn = nullptr;           //灵墨图标
    QPushButton* m_clostBtn = nullptr;          //关闭按钮
    QPushButton* m_keyboardBtn = nullptr;       //键盘按钮
    QPushButton* m_powerOffBtn = nullptr;       //电源按钮
 //    MiddleFrameManager* m_middleFrame = nullptr;//中间窗显示
    QStackedWidget* m_stackWidget = nullptr;
    int m_curIndex = 0;
    int ImgIndex=0;//记录选择镜像安装的后面一个界面的界面号
    int ImgNext=0;
    int m_stackCount = 0;

    QList<QPair<std::string,IPlugin*>> m_stackPageMap;
    QList<QPair<std::string,IPlugin*>>::iterator it;
    InstallingFrame* m_installingFrame = nullptr;
    FinishedFrame* m_finishedFrame = nullptr;
    InstallingOEMConfigFrame* m_oemFrame = nullptr;

    QList<QLabel*> m_steps;
    ArrowWidget *w;
    VirtualKeyboard* keyboard;
    int SCreenWidth;
    int SCreenHeight;

public slots:

    void clickCloseButton();                    //关闭按钮槽处理
    void clickkeyButton();                      //键盘按钮槽处理
    void clickPowerButton();                    //电源按钮槽处理
    void clickBackButton();

    void changeStackPage(int step);
    void addStackWidgetPage(QWidget* widget);

    void showInstallingFrame();
    void showInstallingErrorFrame(QString str);
    void showFinishedFrame();
     void ScreenAdapt();

signals:
    void signalScriptFinished();
    void signalInstallingClose();
    void signalcloseArrowWidget();
    void signalMainFrameSize(QWidget *pFrame, int SCreenWidth, int SCreenHeight);

};
}
#endif // INSTALLERMAINWIDGET_H
