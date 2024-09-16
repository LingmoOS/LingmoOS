#ifndef DRAPWINDOWSDATA_H
#define DRAPWINDOWSDATA_H

#include <QList>
#include <QString>

class QSettings;
class QJsonObject;
class QPixmap;

namespace BrowserName {
inline constexpr char MicrosoftEdge[]{ "Microsoft Edge" };
inline constexpr char GoogleChrome[]{ "Google Chrome" };
inline constexpr char MozillaFirefox[]{ "Mozilla Firefox" };
}; // namespace BrowserName

struct UosApp
{
    QString UosName;
    QString windowsName;
    QStringList feature;
};
struct WinApp
{
    QString name;
    QString iconPath;
};
class DrapWindowsData
{
public:
    ~DrapWindowsData();

    static DrapWindowsData *instance();

    QList<WinApp> getApplianceList();
    QString getDesktopWallpaperPath();
    QList<QPair<QString, QString>> getBrowserBookmarkPaths();
    QList<QPair<QString, QString>> getBrowserBookmarkList();

    QStringList getBrowserList();
    void getBrowserBookmarkHtml(QString &htmlPath);
    void getBrowserBookmarkInfo(const QSet<QString> &Browsername);
    QString getBrowserBookmarkJSON(QString &jsonPath);

    QString getUserName();
//    QString getIP();

    void getLinuxApplist(QList<UosApp> &list);
    QMap<QString, QString>
    RecommendedInstallationAppList(QMap<QString, QString> &notRecommendedList);

    QPixmap getAppIcon(const QString &path);
private:
    DrapWindowsData();

    bool containsAnyString(const QString &haystack, const QStringList &needles);
    void getBrowserBookmarkPathInfo();

    void getApplianceListInfo();
    void getBrowserListInfo();
    void getDesktopWallpaperPathRegistInfo();
    void getDesktopWallpaperPathAbsolutePathInfo();

    void applianceFromSetting(QSettings &settings, QString registryPath = QString());

    void readFirefoxBookmarks(const QString &dbPath);
    void readMicrosoftEdgeAndGoogleChromeBookmark(const QString &jsonPath);
    void browserBookmarkJsonNode(QJsonObject node);
    void insertBrowserBookmarkList(const QPair<QString, QString> &titleAndUrl);

    QList<WinApp> applianceList;
    QStringList browserList;
    QString desktopWallpaperPath;
    QList<QPair<QString, QString>> browserBookmarkPath;
    QList<QPair<QString, QString>> browserBookmarkList;
};

#endif // DRAPWINDOWSDATA_H
