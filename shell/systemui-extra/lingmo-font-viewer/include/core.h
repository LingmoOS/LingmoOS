#ifndef __CORE_H__
#define __CORE_H__

#include <QObject>

#include "include/libfun.h"
#include "include/external.h"

class Core : public QObject
{
    Q_OBJECT
public:
    ~Core();

    static Core *getInstance(void);                     /* 获取单例指针 */
    QList<ViewData> getViewData(void);                  /* 获取全部字体 */
    FontInformation getFontDetails(QString path);       /* 获取单个字体详细信息 */
    QList<ViewData> getSystemFont(void);                /* 获取系统字体 */
    QList<ViewData> getMyFont(void);                    /* 获取我的字体 */
    QList<ViewData> getCollectFont(void);               /* 获取收藏字体 */
    bool collectFont(QString path);                     /* 收藏字体 */
    bool unCollectFont(QString path);                   /* 取消收藏字体 */
    bool installFont(QString path);                     /* 安装字体 */
    bool uninstallFont(QString path, QString fontName); /* 卸载字体 */
    bool copyFont(QString srcPath , QString dstPath);   /* 复制字体 */
    bool isSystemFont(QString path);                    /* 判断是否为系统字体 */
    bool applyFont(QString family, QString path);       /* 应用字体 */
    QString ControlUsingFont();                         /* 获取正在使用的字体名称 */
    void updateFontList();                              /* 更新字体列表 */

    bool judgeBadFontFile(QString path);   /* 判断字体文件是否损坏 */
    bool getSettingFontTips();             /* 获取字体提示状态 */
    bool settingFontTips(bool state);      /* 字体提示状态 */
private:
    Core();

    bool init(void);
    void printFontData(void);

    QString m_installPath;

    External m_external;
    QList<FontInformation> m_fontData;
    QList<FontInformation> m_collectFontData;

public slots:
    void slotStart(void);
};

#endif
