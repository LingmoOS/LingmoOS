#ifndef USERFRAME_H
#define USERFRAME_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QEvent>
#include <QCheckBox>
#include <QGuiApplication>
#include <QScreen>
#include "checkedlineedit.h"
#include "clineeditlabel.h"
#include "QHash"
#include "QTextEdit"
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"

#include "pwquality.h"

//空字符
#define EMPY    100
//hostnane长度范围
#define HOSTNAME_MIN    1
#define HOSTNAME_MAX    64

//hostname 错误代码
#define HOSTNAME_OK     0
#define HOSTNAME_TOOSHORT   2
#define HOSTNAME_TOOLONG    3
#define HOSTNAME_INVALCHAR  4

//密码安全检测配置文件路径
#define PWQUALITYPATH   "/etc/security/pwquality.conf"
//cpwd
#define CPPASSWD_OK 0
#define CPPASSWD_DIFF   1

namespace KInstaller {

class UserFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit UserFrame(QWidget *parent = nullptr);
    ~UserFrame();
    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void translateStr();
    void writeSettingIni();
    void setNextBtnStyle(); //检测下一步按钮的状态显示
    void setBoardTabOrder();
signals:
    void signalConfirmPasswd();

public slots:

    void setComputerText();
    void checkUserName(QString m_username);
    void checkHostName(QString m_hostname);
    void checkPasswd(QString pwd);
    void checkCPasswd(QString cpwd);
    void markLineEditCorrect();
    bool findFingerPrintDevice();
    void getAutoLogin();
    void clickNextButton();
private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
//    bool eventFilter(QObject *watched, QEvent *event);
private:

    CLineEditLabel *m_username = nullptr;
    CLineEditLabel *m_hostname = nullptr;
    CLineEditLabel *m_passwd = nullptr;
    CLineEditLabel *m_confirm_passwd = nullptr;

    //四个编辑框内容的的错误类型以及错误提示
    int m_userstate=EMPY;
    int m_hoststate=EMPY;
    int m_pwdstate=EMPY;
    int m_cpwdstate=EMPY;
    QString m_usererr;
    QString m_hosterr;
    QString m_pwderr;
    QString m_cpwderr;

    //密码检测设置
    pwquality_settings_t *m_settings;
    void *auxerror;
    //错误图标
    QLabel *m_userCheck = nullptr;
    QLabel *m_hostCheck = nullptr;
    QLabel *m_pwdCheck = nullptr;
    QLabel *m_cpwdCheck = nullptr;
   //错误提示语句
    QLabel *m_userErrorTip=nullptr;
    QLabel *m_hostErrorTip=nullptr;
    QLabel *m_pwdErrorTip=nullptr;
    QLabel *m_cpwdErrortip=nullptr;


    QLabel* fingerprintpng = nullptr;
    QLabel* fingerprinttip = nullptr;

    QLabel* m_mainTitle = nullptr;

    QCheckBox* autoLoginCKBox = nullptr;
    QCheckBox* pwLoginCKBox = nullptr;
    bool m_autoLoginFlag = false;
    bool blfinger = false;

};
}
#endif // USERFRAME_H
