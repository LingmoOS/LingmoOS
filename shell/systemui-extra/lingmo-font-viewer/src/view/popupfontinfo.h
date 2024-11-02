#ifndef POPUPFONTINFO_H
#define POPUPFONTINFO_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QGridLayout>

#include "basepopuptitle.h"
#include "include/core.h"

/* 字体信息弹窗 */
class PopupFontInfo : public QWidget
{
    Q_OBJECT
public:
    PopupFontInfo(QString fontPath , QWidget *parent = nullptr);
    ~PopupFontInfo();
    void setWidegtUi();
    void setWidLayout();
    void changeFontSize();
    void initWidgetStyle();

private:
    BasePopupTitle *m_titleBar = nullptr;
    QLabel *m_tipText = nullptr;
    QWidget *m_widget = nullptr;
    QScrollArea *m_scrollArea = nullptr;

    QLabel *m_fontName = nullptr;         /* 属性：名称 */
    QLabel *m_nameValue = nullptr;        /* 名称 */

    QLabel *m_fontSeries = nullptr;       /* 属性：系列 */
    QLabel *m_seriesValue = nullptr;      /* 系列 */

    QLabel *m_fontStyle = nullptr;        /* 属性：样式 */
    QLabel *m_styleValue = nullptr;       /* 样式 */

    QLabel *m_fontType = nullptr;         /* 属性：种类 */
    QLabel *m_typeValue = nullptr;        /* 种类 */

    QLabel *m_fontVersion = nullptr;      /* 属性：版本 */
    QLabel *m_versionValue = nullptr;     /* 版本 */

    QLabel *m_fontPath = nullptr;         /* 属性：位置 */
    QLabel *m_pathValue = nullptr;        /* 位置 */

    QLabel *m_fontCopyright = nullptr;    /* 属性：版权 */
    QLabel *m_copyrightValue = nullptr;   /* 版权 */

    QLabel *m_fontTrademark = nullptr;    /* 属性：商标 */
    QLabel *m_trademarkValue = nullptr;   /* 商标 */

    QPushButton *m_continueBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;

    QVBoxLayout *m_vLayout = nullptr;
    QGridLayout *m_gridLayout = nullptr;   /* 字体信息内容网格布局 */
    QVBoxLayout *m_vGridLayout = nullptr;
    QHBoxLayout *m_hBtnLayout = nullptr;
    QHBoxLayout *m_hMainLayout = nullptr;
    QVBoxLayout *m_vMainLayout = nullptr;

    FontInformation m_font;   /* 字体信息结构体 */

signals:
public slots:
    void slotContinueClick();
    void slotCancelClick();
    void slotChangeFontName();
    void slotChangeFaltSize();
    void slotChangePCSize();
};

#endif // POPUPFONTINFO_H
