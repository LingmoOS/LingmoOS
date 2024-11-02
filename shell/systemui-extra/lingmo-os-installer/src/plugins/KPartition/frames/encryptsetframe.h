#ifndef ENCRYPTSETFRAME_H
#define ENCRYPTSETFRAME_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QAction>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../../KUserRegister/clineeditlabel.h"
#include "../../KUserRegister/checkedlineedit.h"
#include <qpainter.h>
#include <QPainterPath>

using namespace KInstaller;
namespace KServer {

enum class LabelState {
    Username,
    Hostname,
    Pwd,
    CPwd,
    unkown
};
class EncryptSetFrame : public QDialog
{
    Q_OBJECT
public:
    explicit EncryptSetFrame(QDialog *parent = nullptr);

    void initUI();                              //加载界面ui
    void initAllConnect();                      //信号和槽链接
    void addStyleSheet();                       //设置样式
    void translateStr();
    void setMessageInfo(QString info);
    void setCancleHidden();
    void markLineEditCorrect(LabelState state, bool flag);
    int execid;
signals:
    void signalCancel();
    void signalOK();


private:
    void changeEvent(QEvent *event) override;       //改变语言事件
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public:
    QLabel *m_info;
    QPushButton *m_cancel, *m_OK, *m_close;
    CLineEditLabel *m_pwd;
    CLineEditLabel *m_pwd1;
    QLabel *checkpwd, *checkpwd1;
    QString errstr;
    CheckState m_state ;
    LabelState m_LabelState ;
    bool blbtn;
    ArrowWidget *m_tTip;
    bool m_flag = false;
    bool pwdIsOK ,cpwdIsOK;
signals:
     void  signalClickBtn(int execid);
     void signalConfirmPasswd();

public slots:
     void checkPasswd(QString pwd);
     void checkCPasswd(QString cpwd);
};

}
#endif // ENCRYPTSETFRAME_H
