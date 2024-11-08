#ifndef FONTLISTVIEW_H
#define FONTLISTVIEW_H

#include <QObject>
#include <QListView>
#include <QPoint>
#include <QMenu>
#include <QAction>

#include "fontlistdelegate.h"
#include "fontlistdelegateug.h"
#include "fontlistfilter.h"
#include "fontlistmodel.h"
#include "basepreviewarea.h"
#include "popupremove.h"
#include "popupinstallsuccess.h"
#include "popupinstallfail.h"
#include "popupfontinfo.h"
#include "popuptips.h"
#include "popupmessage.h"
class FontListDelegate;
class FontListDelegateug;
class FontListView : public QListView
{
    Q_OBJECT
public:
    FontListView(QListView *parent = nullptr);
    ~FontListView();

    void setWidgetUi();                            /* 初始化组件 */
    void setSlotConnet();                          /* 关联信号和槽 */
    void showContextMenu(const QPoint &pos);       /* 显示右键菜单 */
    void removeFont(QString path);                 /* 卸载字体 */
    int getFontSize();                             /* 获取预览字号 */
    QString getPreviewValue();                     /* 获取预览文字 */
    void getFilterType(FontType type);             /* 更改过滤类型 */
    void dragEnterAddFont(QStringList fontList);   /* 拖拽安装字体 */
    void updateDelegate();                         /* 更新代理绘制每项高度 */
    bool isControlApplyFont();                     /* 判断当前字体是否为控制面板应用字体 */
    void clickFontFileInstall(QStringList fontLis); /* 双击字体文件安装字体 */
    void clickFontFileInstallAfterOpen(QStringList fontList); /* 双击字体文件安装字体 */
    void setModelAfterGetAllData();          /* 设置model */
    void checkFontFile(QStringList list);    /* 安装字体并检测字体是否被损坏 */
    void chooseFontInstallItem();
    void changeItemInstallStyle(bool style); /* 修改item中的fontinstall的状态 */
    void showFontTips();
    void setFontTips(QStringList list);
    void addFontByOtherWay(QStringList list);
    void showTipsByClickFontFile();

private:
    QMenu   *m_funcMenu = nullptr;            /* 右键菜单 */
    QAction *m_addFontAction = nullptr;       /* 添加 */
    QAction *m_applyFontAction = nullptr;     /* 应用 */
    QAction *m_removeFontAction = nullptr;    /* 卸载 */
    QAction *m_exportFontAction = nullptr;    /* 导出 */
    QAction *m_collectFontAction = nullptr;   /* 收藏 */
    QAction *m_checkFontAction = nullptr;     /* 字体信息 */

    FontListModel *m_fontModel = nullptr;
    FontListDelegate *m_fontDelegate = nullptr;
    FontListDelegateug *m_fontDelegateug = nullptr;

    FontListFilter *m_fontFilter = nullptr;

    PopupRemove *m_popupRemove = nullptr;                   /* 卸载弹窗 */
    PopupInstallSuccess *m_popupInstallSuccess = nullptr;   /* 安装成功 */
    PopupInstallFail *m_popupInstallFail = nullptr;         /* 安装失败 */
    PopupFontInfo *m_popupFontInfo = nullptr;               /* 字体信息 */
    PopupTips *m_popupTips = nullptr;                       /* 字体提示 */
    PopupMessage *m_popupBox = nullptr;                     /* 安装字体提示 */

    int m_fontSize = 24;   /* 预览初始字号 */
    QString m_previewValue = tr("Build the core strength of Chinese operating system");
    FontType m_filterType = FontType::AllFont;   /* 过滤类型 */
    QString m_fontSearch = "";
    QStringList m_fontList;

    QStringList m_fontInstallList;
    bool m_firstOpen = true;

signals:
    void sigGetAllFont();
    void sigChangeFont(QString fontName);
    void sigBlankPage();
    void sigFontPage();
    void sigSearchFontBlankPage();

public slots:
    void slotFilterFontByReg(QString fontSearch);   /* 根据字符串过滤字体 */
    void slotAddFont();                             /* 添加字体 */
    void slotApplyFont();                           /* 应用字体 */
    void slotRemoveFont();                          /* 卸载字体 */
    void slotExportFont();                          /* 导出字体 */
    void slotCollectFont();                         /* 收藏字体 */
    void slotCheckFont();                           /* 字体信息 */
    void slotGetFontSize(int size);                 /* 改变字号 */
    void slotUpdatePreView(QString value);          /* 修改预览内容 */
    void slotClickCollect();                        /* 点击图标收藏 */
    void slotClickFontFile();                       /* 点击字体文件进行安装 */

};

#endif // FONTLISTVIEW_H
