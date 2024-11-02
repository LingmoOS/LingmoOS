#ifndef __LIB_FUN_H__
#define __LIB_FUN_H__

#include <QString>
#include <QList>

#define INSTALL_PATH "/.local/share/fonts/lingmo-font-viewer/"

#define WHITESPACE_CHARS "\f \t"
#define MATCH_VERSION_STR "Version"

typedef struct _FontInformation
{
    QString path;             /* 路径 */
    QString name;             /* 名称 */
    QString family;           /* 系列 */
    QString style;            /* 样式 */
    QString type;             /* 种类 */
    QString foundry;          /* 铸造商 */
    QString version;          /* 版本 */
    QString copyright;        /* 版权 */
    QString manufacturer;     /* 商标 */
    QString description;      /* 描述 */
    QString designer;         /* 设计师 */
    QString license;          /* 许可证 */
} FontInformation;

typedef struct _ViewData
{
    QString path;
    QString family;
    QString style;
} ViewData;

class LibFun
{
public:
    static QList<FontInformation> getAllFontInformation(void);
    static bool installFont(QString path);
    static bool uninstallFont(QString path);
    static bool copyFont(QString src , QString dst);
    static void remove_whitespace(char **ori);
    static void remove_version(char **ori);
    static bool chooseFontFile(QString path);
    static bool caseInsensitiveLessThan(const FontInformation &s1, const FontInformation &s2);
    static QString getFontInfo(QString path);
    static bool isFontFile(QString path);
};

#endif
