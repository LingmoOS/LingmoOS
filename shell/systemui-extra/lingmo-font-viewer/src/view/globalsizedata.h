#ifndef GLOBALSIZEDATA_H
#define GLOBALSIZEDATA_H

#include <QObject>
#include <QGSettings>
#include "rotatechangeinfo.h"

#define LIGHT_THEME "lingmo-default"
#define BLACK_THEME "lingmo-dark"
#define FONT_PATH "/lingmo-font-viewer/"

/* 字体类型 */
enum FontType {
    AllFont = 0,
    SystemFont,
    MyFont,
    CollectFont,
};

enum CurrentMode {
    PCMode = 0,
    HMode,
    VMode,
};

class GlobalSizeData : public QObject
{
    Q_OBJECT
public:
    static GlobalSizeData *getInstance();
    ~GlobalSizeData();

    /* 获取主题的样式，字号，透明度 */
    void getSystemTheme();
    void getSystemFontSize();
    void getSystemTransparency();
    void getSystemFontName();

    /* 获取dbus平板/pc数据 */
    void getSystemDBusInfo();
    QPair<int,int> getCurrentScale();
    bool getIsPCMode();

    /* SDK功能打点 */
    void addFontSDKPoint();
    void removeFontSDKPoint();
    void searchFontSDKPoint();
    void applyFontSDKPoint();
    void exportFontSDKPoint();
    void collectionFontSDKPoint();
    void cancelCollectionFontSDKPoint();
    void fontInfoSDKPoint();
    void changecopywritingSDKPoint();
    void changeFontSizeSDKPoint();
    
    // 主题色
    static int THEME_COLOR;

    enum ThemeColor {
        LINGMOLight = 0,
        LINGMODark,
    };

    double g_transparency;   /* 毛玻璃透明度 */
    double g_fontSize;          /* 字号 */

    /* 字体监控 */
    double g_font14pxToPt;
    double g_font16pxToPt;   /* 弹窗标题 */
    double g_font18pxToPt;   /* 关于标题 */

    static FontType g_fontType;     /* 当前查看字体类型 */
    QString m_fontName = "";
    QGSettings *m_themeGsettings = nullptr;

    enum FontRoles {
        FontPath = Qt::UserRole,
        FontName,
        FontStyle,
        FontFamily,
        CollectState,
        FontHeight,
        FontInstall,
    };

    enum FamilyType {
        SystemFont = 0,
        MyFont,
    };

    enum CollectType {
        IsCollected = 0,
        NotCollected
    };

    RotateChangeInfo *m_rotateChange = nullptr;

    // 获取当前模式
    CurrentMode getCurrentMode();
    CurrentMode m_currentMode = CurrentMode::PCMode;
    QPair<int, int> m_currentSize;
signals:
    void sigFontChange();
    void sigFontNameChange();
    void sigPCMode();
    void sigVFlatMode();
    void sigHFlatMode();
private:
    GlobalSizeData();
    void changeFontSize();
    void changeFontName();
    void slotChangeModel(bool mode, int width, int height);
};

#endif // GLOBALSIZEDATA_H
