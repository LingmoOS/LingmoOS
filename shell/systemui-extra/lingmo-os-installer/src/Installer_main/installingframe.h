#ifndef INSTALLINGFRAME_H
#define INSTALLINGFRAME_H

#include <QObject>
#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "./uilt/slidershow.h"
#include "./uilt/showprogressbar.h"
#include "finishedInstall.h"
#include "qtextbrowser.h"
#include "qtextstream.h"
#include "qfilesystemwatcher.h"
//#include "uilt/progressbarframe.h"

#include <myprobar.h>


namespace KInstaller {
class InstallingFrame : public QWidget
{
    Q_OBJECT
public:
    explicit InstallingFrame(QWidget *parent = nullptr);
    ~InstallingFrame();
    void initUI();
    void addStyleSheet();
    void translateStr();
    void initAllConnect();
    void clickNextButton();


signals:
    void signalFinished();

public slots:
    void updateProgressValue(int value);
    void findFile();
    void closeW();
    void showMsgDetail();
    void flashMsg();
    void showErrorFrame();
    QLabel* geterrorContext();
    void clickRestartBtn();

private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
public:
    QLabel* littleTitle;
    myprobar *m_installProgressBar;
    QLabel* m_mainTitle;
    QPushButton* m_restartBtn;
    SlideShow* slide;
    ShowProgressBar* progressBarCtrl;
    QTimer* timer;
    int tmpvalue=0;

    QPushButton *textDetail;
     QGridLayout* gridLayout;
     QGridLayout* textbox;
      QLabel *spic;
      QLabel *stext;

     QLabel *textContext;
     QTextBrowser *textMsg;
     QLabel *MsgContext;
     QFile *fi;
     QFileSystemWatcher *mywatcher;
     qint64 currline=0;
     bool flag=1;
     bool is_faild=0;
     int provalue=0;

};
}

#endif // INSTALLINGFRAME_H
