#ifndef TITLEBAR_H
#define TITLEBAR_H
#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QGSettings>
#include <QPainterPath>
#include <QPainter>
#include <QStyleOption>
#include "menumodule.h"
#include "controller/interaction.h"
#include "edit.h"
#include <kborderlessbutton.h>
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();
    QWidget *g_titleWid; //布局
    menuModule *g_menu = nullptr;
    QLabel *g_imageName; //图片名字
    Edit *g_myEdit = nullptr;
    QPushButton *g_fullscreen; //最大化/还原
    QPushButton *m_logoBtn;    //左上角logo
    void showImageName(QString name, QString imagePath);
    void needRename(int mode);                                   //需要重命名
    void setRenameEnable(bool isOpenSucess, bool isCanOpertate); //判断是否可重命名
    void setBtnShowOrHide();                                     //判断是否需要隐藏三联按钮
    void setLeftUpBtnStyle();                                    //设置左上角的按钮
    void dealEditState();                                        //隐藏重命名框

private:
    QMessageBox *m_msg;
    QHBoxLayout *m_titleLayout;

    QString m_oldPath = ""; //存上一次界面刷新的图片路径
    QPushButton *m_backButton = nullptr;
    QLabel *m_logolb;           //左上角logoname
    QPushButton *m_minibtn;     //最小化按钮
    QPushButton *m_closebtn;    //关闭
    QString m_imagePath;        //文件路径
    QString m_oldName;          //存重命名之前的名字
    bool m_renameEnable = true; //判断是否可以重命名--默认为true
    void initControlQss();      //初始化顶栏布局
    void initConnect();         //建立信号与槽的连接
    void longText(QLabel *nameC, QString text);
    void reName();
    void dealRenameResult(int code, QFileInfo newfile); //处理后端返回重命名结果，进行界面显示
    void showOrigName();
    void editSelected(); // lineedit选中
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void dealDoubleClick(bool isleaveOrNot);
    QString m_origName; //对于无法改名字的，改名框需重新设置原始名字
    void initFontSize();
    void platformType();

Q_SIGNALS:

    void recovery();                                           //处理主界面最大化和还原
    void openSignal();                                         //标题栏夏卡菜单打开图片
    void aboutShow();                                          //关于界面打开，两栏的展示情况
    void toInforChangeName();                                  //更改成功，告诉信息栏改名字
    void toCoreChangeName(QString oldName, QFileInfo newName); //后端改名字
    void updateInformation(QFileInfo newName);                 //更新信息栏
    void doubleClick(bool isleaveOrNot);
    void updateFileInfo(QFileInfo fileInfo);      //更新重命名后的文件名-工具栏-删除
    void updateImageWidName(QString newNamePath); //更新展示图片界面文件路径
    void exitApp();
};

#endif // TITLEBAR_H
