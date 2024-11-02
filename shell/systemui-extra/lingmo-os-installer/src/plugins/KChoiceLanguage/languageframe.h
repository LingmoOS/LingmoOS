#ifndef LANGUAGEFRAME_H
#define LANGUAGEFRAME_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include "../PluginService/ui_unit/comboxdelegate.h"
#include "../PluginService/ui_unit/comboxlistmodel.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include <QEvent>
#include <QPushButton>
#include "pushbuttoncheck.h"
#include <QTranslator>
#include <QScrollArea>
using namespace KServer;
namespace KInstaller {

class LanguageFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit LanguageFrame(QWidget *parent = nullptr);

public:
    void initUI();                              //加载界面控件
    void initAllConnect();                      //信号槽连接
    void translateStr();                        //更新需要翻译的对象字符串
    void addStyleSheet();                       //添加窗口样式
    void initLanguageBox();                     //设置下拉框model。已不用
    void initBtn();

    void readSettingIni();                      //读写systemsetting.ini文件
    void writeSettingIni();                     //读写systemsetting.ini文件
    void setBoardTabOrder();

    QString setAllBtnStatus(QString strCurrLang);   //设置所有语言按钮的勾选状态，勾选上strCurrLang语言对应按钮。默认勾选中文
    void showBottArea(QString strCurrLang);      //显示当前按钮所在区域

signals:
    void signalPageChanged(int flag);
    void changeLanguage_sig(QString objname);       //更新全部语言按钮的选中状态

public slots:
//    void changeLanguageEn();
//    void changeLanguageZh();
    void changeLanguage(PushButtonCheck* obj);
    void clickNextButton();

private:
    void changeEvent(QEvent *event) override;       //改变语言事件
    void keyPressEvent(QKeyEvent *event);
private:

    QComboBox* m_languageBox;               //语言下拉框。已不用
    QLabel* m_mainTitle;

    QString m_curSystenlocal;               //当前选中的语言名称
    ComboxListModel* m_model;               //model
    ItemLanguage m_comboxItem;              //列表项

    QLabel* m_lingmopng;
    QLabel* m_titleOS;
    QLabel* m_littleOS;

    QTranslator* m_pCurTranslator = nullptr;

    PushButtonCheck* m_ZHCNBtn;        //中文(简体)按钮
    PushButtonCheck* m_ZHHKBtn;        //中文(繁体)按钮 zh_HK
    PushButtonCheck* m_KKKZBtn;        //Қазақ按钮  哈萨克 kk_KZ
    PushButtonCheck* m_UGCNBtn;        //Uyghur按钮  维吾尔：ug_CN
    PushButtonCheck* m_KYKGBtn;        //قىرعىز تىلى按钮  柯尔克孜：ky_KG
    PushButtonCheck* m_BOCNBtn;        //བོད་ཡིག按钮  藏文：bo_CN
    PushButtonCheck* m_MNMNBtn;        //ᠮᠣᠩᠭᠣᠯ ᠪᠢᠴᠢᠭ᠌按钮  蒙古文：mn_MN
    PushButtonCheck* m_ENUSBtn;        //英文按钮
    PushButtonCheck* m_DEDEBtn;        //德文按钮
    PushButtonCheck* m_ESESBtn;        //西班牙文按钮
    PushButtonCheck* m_FRFRBtn;        //法文按钮

    QScrollArea * m_pBtnScroll;
    QWidget * m_pBtnSWidget;
    QVBoxLayout * m_pBtnSWLayout;
};
}
#endif // LANGUAGEFRAME_H
