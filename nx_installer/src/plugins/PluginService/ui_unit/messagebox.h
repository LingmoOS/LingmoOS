#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QAction>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "clineeditlabel.h"
#include "../../KUserRegister/checkedlineedit.h"
#include <QPainter>
#include <QPainterPath>
using namespace KInstaller;
namespace KServer {
class MessageBox : public QDialog
{
    Q_OBJECT
public:
    explicit MessageBox( QWidget *parent = nullptr);

    void initUI();                              //加载界面ui
    void initAllConnect();                      //信号和槽链接
    void addStyleSheet();                       //设置样式
    void translateStr();
    void setMessageInfo(QString info);
    void setMessageInfo(QString info, QString infoBelow);
    void setCancleHidden();
    int execid;
signals:
    void signalCancel();
    void signalOK();


private:
    void changeEvent(QEvent *event) override;       //改变语言事件
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

public:
    QLabel *m_info;                                 //第一行文字
    QLabel *m_infoBelow;                                 //第二行文字
    QPushButton *m_cancel, *m_OK, *m_close;

    ArrowWidget *m_tTip;
signals:
     void  signalClickBtn(int execid);


};

}
#endif // MESSAGEBOX_H
