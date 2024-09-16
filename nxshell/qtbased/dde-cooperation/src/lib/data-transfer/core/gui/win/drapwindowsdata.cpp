#include "drapwindowsdata.h"
//#include "common/log.h"
//#include "common/commonutils.h"

#include <tchar.h>
#include <QDebug>
#include <QFile>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>
#include <QDateTime>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QSettings>
#include <ShlObj.h>
#include <QtWin>

#define MAXNAME 256

namespace Registry {
inline constexpr char BrowerRegistryPath[]{ "SOFTWARE\\Clients\\StartMenuInternet" };
inline constexpr char ApplianceRegistryPath1[]{
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
};
inline constexpr char ApplianceRegistryPath2[]{
    "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
};
inline constexpr char ApplianceRegistryPath3[]{
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
};
inline constexpr char DesktopwallpaperRegistryPath[]{ "Control Panel\\Desktop" };
} // namespace Registry

namespace BrowerPath {
inline constexpr char MicrosoftEdgeBookMark[]{
    "\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Bookmarks"
};
inline constexpr char GoogleChromeBookMark[]{
    "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Bookmarks"
};
inline constexpr char MozillaFirefoxBookMark[]{ "\\AppData\\Roaming\\Mozilla\\Firefox" };
} // namespace BrowerPath

DrapWindowsData::DrapWindowsData() { }

DrapWindowsData *DrapWindowsData::instance()
{
    static DrapWindowsData ins;
    return &ins;
}

DrapWindowsData::~DrapWindowsData() { }
QStringList DrapWindowsData::getBrowserList()
{
    if (browserList.isEmpty())
        getBrowserListInfo();
    return browserList;
}

void DrapWindowsData::getBrowserBookmarkHtml(QString &htmlPath)
{
    if (htmlPath.isEmpty()) {
        htmlPath = QString::fromLocal8Bit(".");
    }

    QStringList bookmarkItems;
    for (const QPair<QString, QString> &bookmark : browserBookmarkList) {
        QString bookmarkItem =
                QString("<a href=\"%1\">%2</a>").arg(bookmark.second).arg(bookmark.first);
        bookmarkItems.append(bookmarkItem);
    }

    QString htmlTemplate = QString::fromLocal8Bit(
            "<!DOCTYPE NETSCAPE-Bookmark-file-1>\n"
            "<!-- This is an automatically generated file. It will be read and overwritten."
            "DO NOT EDIT! -->\n"
            "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n"
            "<TITLE>Bookmarks</TITLE>\n"
            "<H1>Bookmarks</H1>\n"
            "<DL><p>\n"
            "<DT><H3 ADD_DATE=\"1688527902\" LAST_MODIFIED=\"1693460686\" "
            "PERSONAL_TOOLBAR_FOLDER=\"true\">书签栏</H3>\n"
            "<DL><p>\n"
            "<urlAndtile>\n"
            "</DL><p>\n"
            "</DL><p>\n");

    QString bookmarkList = bookmarkItems.join("\n");
    QString htmlContent = htmlTemplate.replace("<urlAndtile>", bookmarkList);
    QString htmlFile = htmlPath + "/bookmarks.html";

    QFile outputFile(htmlFile);
    if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&outputFile);
        out.setCodec("UTF-8");
        out << htmlContent;
        outputFile.close();
    } else {
//        DLOG << "Failed to open file";
        return;
    }
}

QList<WinApp> DrapWindowsData::getApplianceList()
{
    if (applianceList.isEmpty())
        getApplianceListInfo();

//    for (auto value : applianceList)
//        LOG << "app name:" << value.name.toStdString();
//    LOG << "applianceList.size:" << applianceList.size();
    return applianceList;
}

QString DrapWindowsData::getDesktopWallpaperPath()
{
    if (desktopWallpaperPath.isEmpty()) {
        getDesktopWallpaperPathRegistInfo();
    }
    if (desktopWallpaperPath.isEmpty()) {
        getDesktopWallpaperPathAbsolutePathInfo();
    }
    return desktopWallpaperPath;
}

void DrapWindowsData::readFirefoxBookmarks(const QString &dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Error opening firefox bookmark database:" << db.lastError();
        return;
    }

    QSqlQuery query;
    if (query.exec("SELECT moz_places.url, moz_bookmarks.title FROM moz_places "
                   "INNER JOIN moz_bookmarks ON moz_places.id = moz_bookmarks.fk")) {
        while (query.next()) {
            QString url = query.value(0).toString();
            QString title = query.value(1).toString();
            QPair<QString, QString> titleAndUrl(title, url);
            insertBrowserBookmarkList(titleAndUrl);
        }
    } else {
        qDebug() << "read firefox bookmark failed:" << query.lastError();
    }
    db.close();
}

void DrapWindowsData::readMicrosoftEdgeAndGoogleChromeBookmark(const QString &jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        ELOG << "Failed to open file :" << jsonPath.toStdString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject obj = doc.object();

    QJsonObject roots = obj["roots"].toObject();
    for (const QString &key : roots.keys()) {
        browserBookmarkJsonNode(roots[key].toObject());
    }
}

QList<QPair<QString, QString>> DrapWindowsData::getBrowserBookmarkPaths()
{
    return browserBookmarkPath;
}

QList<QPair<QString, QString>> DrapWindowsData::getBrowserBookmarkList()
{
    return browserBookmarkList;
}

void DrapWindowsData::getBrowserBookmarkPathInfo()
{
    if (browserList.isEmpty()) {
        getBrowserListInfo();
    }

    QString appData = std::getenv("USERPROFILE");

    if (browserList.contains(BrowserName::MicrosoftEdge)) {
        QString path = appData + BrowerPath::MicrosoftEdgeBookMark;
        auto bookMark = QPair<QString, QString>(BrowserName::MicrosoftEdge, path);
        browserBookmarkPath.push_back(bookMark);
    }

    if (browserList.contains(BrowserName::GoogleChrome)) {
        QString path = appData + BrowerPath::GoogleChromeBookMark;
        auto bookMark = QPair<QString, QString>(BrowserName::GoogleChrome, path);
        browserBookmarkPath.push_back(bookMark);
    }

    if (browserList.contains(BrowserName::MozillaFirefox)) {
        QString path = appData + BrowerPath::MozillaFirefoxBookMark;
        QString installIni = path + QString("\\installs.ini");
        QFile file(installIni);
        QString bookMarkPath;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains("Default")) {
                    bookMarkPath = "\\" + line.split("=").at(1) + "\\places.sqlite";
                }
            }
            file.close();
        } else {
//            DLOG << "Can not open file:" << installIni.toStdString();
        }

        if (!bookMarkPath.isEmpty()) {
            path = path + bookMarkPath;
            auto bookMark = QPair<QString, QString>(BrowserName::MozillaFirefox, path);
            browserBookmarkPath.push_back(bookMark);
        } else {
//            DLOG << "Can not find bookMark path in installs.ini";
        }
    }
}

void DrapWindowsData::getBrowserBookmarkInfo(const QSet<QString> &Browsername)
{
    if (browserBookmarkPath.isEmpty()) {
        getBrowserBookmarkPathInfo();
    }
    // clear browserBookmark
    browserBookmarkList.clear();

    if (!Browsername.isEmpty()) {
        for (auto &value : browserBookmarkPath) {
            if (value.first == BrowserName::MozillaFirefox) {
                if (Browsername.contains(BrowserName::MozillaFirefox))
                    readFirefoxBookmarks(value.second);
            } else if (value.first == BrowserName::MicrosoftEdge) {
                if (Browsername.contains(BrowserName::MicrosoftEdge))
                    readMicrosoftEdgeAndGoogleChromeBookmark(value.second);
            } else if (value.first == BrowserName::GoogleChrome) {
                if (Browsername.contains(BrowserName::GoogleChrome))
                    readMicrosoftEdgeAndGoogleChromeBookmark(value.second);
            }
        }
    }
}

QString DrapWindowsData::getBrowserBookmarkJSON(QString &jsonPath)
{
    if (jsonPath.isEmpty()) {
        jsonPath = QString::fromLocal8Bit(".");
    }

    QJsonArray childrenArray;
    int id = 0;
    for (auto bookmark : browserBookmarkList) {
        QJsonObject bookmarkJsonObject;
        bookmarkJsonObject["date_added"] = QString::number(QDateTime::currentMSecsSinceEpoch());
        bookmarkJsonObject["id"] = QString::number(id);
        bookmarkJsonObject["name"] = bookmark.first;
        bookmarkJsonObject["type"] = "url";
        bookmarkJsonObject["url"] = bookmark.second;
        childrenArray.append(bookmarkJsonObject);
        id++;
    }

    QJsonObject bookmarkBarObject;
    bookmarkBarObject["children"] = childrenArray;
    bookmarkBarObject["date_added"] = QString::number(QDateTime::currentMSecsSinceEpoch());
    bookmarkBarObject["date_modified"] = "0";
    bookmarkBarObject["id"] = "1";
    bookmarkBarObject["name"] = "Bookmarks Bar";
    bookmarkBarObject["type"] = "folder";

    QJsonObject otherBookmarksObject;
    otherBookmarksObject["children"] = QJsonArray();
    otherBookmarksObject["date_added"] = QString::number(QDateTime::currentMSecsSinceEpoch());
    otherBookmarksObject["date_modified"] = "0";
    otherBookmarksObject["id"] = "2";
    otherBookmarksObject["name"] = "Other Bookmarks";
    otherBookmarksObject["type"] = "folder";

    QJsonObject syncedBookmarksObject;
    syncedBookmarksObject["children"] = QJsonArray();
    syncedBookmarksObject["date_added"] = QString::number(QDateTime::currentMSecsSinceEpoch());
    syncedBookmarksObject["date_modified"] = "0";
    syncedBookmarksObject["id"] = "3";
    syncedBookmarksObject["name"] = "synced Bookmarks";
    syncedBookmarksObject["type"] = "folder";

    QJsonObject rootsObject;
    rootsObject["bookmark_bar"] = bookmarkBarObject;
    rootsObject["other"] = otherBookmarksObject;
    rootsObject["synced"] = syncedBookmarksObject;

    QJsonObject rootObject;
    rootObject["roots"] = rootsObject;
    rootObject["version"] = 1;

    QJsonDocument doc(rootObject);
    QString jsonfilePath = jsonPath + "/bookmarks.json";
    QFile file(jsonfilePath);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << doc.toJson();
        file.close();
//        DLOG << "JSON file saved successfully.";
        return jsonfilePath;
    } else {
//        WLOG << "Failed to save JSON file.";
        return QString();
    }
}

QString DrapWindowsData::getUserName()
{
    QString userDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QFileInfo fileInfo(userDir);
    QString userName = fileInfo.fileName();

//    DLOG << "User Name: " << userName.toStdString();
    return userName;
}

void DrapWindowsData::getLinuxApplist(QList<UosApp> &list)
{
    QFile file(":/fileResource/apps.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        WLOG << "can not open app json";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
//        WLOG << "app json Parsing failed";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    QStringList keys = jsonObj.keys();

    for (const QString &key : keys) {
        UosApp app;
        QJsonObject appValue = jsonObj.value(key).toObject();

        QVariantList variantList = appValue.value("feature").toArray().toVariantList();
        QStringList featureList;
        for (const QVariant &variant : variantList) {
            if (variant.canConvert<QString>()) {
                featureList.append(variant.toString());
            }
        }
        app.feature = featureList;
        app.windowsName = key;
        app.UosName = appValue.value("packageName").toString();
        list.push_back(app);
    }

    return;
}

//QString DrapWindowsData::getIP()
//{
//    QString ip = deepin_cross::CommonUitls::getFirstIp().data();
//    return ip;
//}

void DrapWindowsData::getApplianceListInfo()
{
    QSettings settings1(Registry::ApplianceRegistryPath1, QSettings::NativeFormat);
    applianceFromSetting(settings1);

    QSettings settings2(Registry::ApplianceRegistryPath2, QSettings::NativeFormat);
    applianceFromSetting(settings2);

    QSettings settings3(Registry::ApplianceRegistryPath3, QSettings::NativeFormat);
    applianceFromSetting(settings3);
}

void DrapWindowsData::getBrowserListInfo()
{
    HKEY hKey;
    LSTATUS queryStatus;
    LPCTSTR lpSubKey;
    lpSubKey = _T(Registry::BrowerRegistryPath);
    queryStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey);
    if (queryStatus == ERROR_SUCCESS) {
        DWORD index = 0;
        CHAR subKeyName[MAX_PATH];
        DWORD subKeyNameSize = sizeof(subKeyName);

        while (RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL)
               != ERROR_NO_MORE_ITEMS) {
            QString strBuffer(subKeyName);
            QString strMidReg;
            strMidReg = (QString)lpSubKey + ("\\") + strBuffer;

            char browerNameBuffer[MAXNAME];
            DWORD bufferSize = sizeof(browerNameBuffer);
            DWORD valueType;
            HKEY hkRKey;

            QByteArray byteArray = strMidReg.toLatin1();
            LPCSTR strMidReglpcstr = byteArray.constData();

            LSTATUS status =
                    RegOpenKeyEx(HKEY_LOCAL_MACHINE, strMidReglpcstr, 0, KEY_READ, &hkRKey);
            if (status == ERROR_SUCCESS) {
                status = RegQueryValueEx(hkRKey, NULL, NULL, &valueType, (LPBYTE)browerNameBuffer,
                                         &bufferSize);
                if (status == ERROR_SUCCESS) {
                    QString name = QString::fromLocal8Bit(browerNameBuffer);

                    if ((!name.isEmpty()) && (!browserList.contains(name))) {
                        browserList.push_back(name);
                    }
                } else {
//                    DLOG << "Failed to read brower name on registry. error code:" << status;
                }
            } else {
//                DLOG << "Failed to open registry HKEY_LOCAL_MACHINE\\" << strMidReglpcstr
//                         << " error code:" << status;
            }
            index++;
            subKeyNameSize = sizeof(subKeyName);
        }
        RegCloseKey(hKey);
    } else {
//        DLOG << "Failed to open registry HKEY_LOCAL_MACHINE\\" << lpSubKey
//                 << " error code:" << queryStatus;
    }
}

void DrapWindowsData::getDesktopWallpaperPathRegistInfo()
{
    WCHAR wallpaperPath[MAX_PATH];
    if (SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0)) {
        QString wallpaperPathStr = QString::fromWCharArray(wallpaperPath);
        QFileInfo fileInfo(wallpaperPathStr);
        if (fileInfo.exists()) {
//            DLOG << "Current wallpaper path: " << wallpaperPathStr.toStdString();
            desktopWallpaperPath = wallpaperPathStr;
        } else {
//            DLOG << "Wallpaper file does not exist.";
        }
    } else {
//        DLOG << "Failed to retrieve wallpaper path.";
    }
}

void DrapWindowsData::getDesktopWallpaperPathAbsolutePathInfo()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString wallpaperFilePath =
            appDataPath + "/AppData/Roaming/Microsoft/Windows/Themes/TranscodedWallpaper";
    QPixmap wallpaperPixmap(wallpaperFilePath);
    if (!wallpaperPixmap.isNull()) {
        QImage wallpaperImage = wallpaperPixmap.toImage();
        QString wallpaperPathStr = QDir::tempPath() + "/ConvertedWallpaper.png";
        if (wallpaperImage.save(wallpaperPathStr, "PNG")) {
//            DLOG << "TranscodedWallpaper converted and saved as PNG to: " << wallpaperPathStr.toStdString();
            desktopWallpaperPath = wallpaperPathStr;
        } else {
//            DLOG << "Failed to save the converted wallpaper.";
        }
    } else {
//        DLOG << "Failed to load TranscodedWallpaper as QPixmap.";
    }
}

void DrapWindowsData::applianceFromSetting(QSettings &settings, QString registryPath)
{
    settings.beginGroup(registryPath);

    QStringList appKeys = settings.childGroups();
    for (const QString &appKey : appKeys) {
        settings.beginGroup(appKey);
        QString displayName = settings.value("DisplayName").toString();
        QString installLocation = settings.value("InstallLocation").toString();
        QString displayIcon = settings.value("DisplayIcon").toString();
        bool isSystemComponent = settings.value("SystemComponent").toBool();
        if (!isSystemComponent && !displayName.isEmpty()) {
            WinApp app;
            app.name = displayName;
            app.iconPath = displayIcon;
            for (auto iteraotr = applianceList.begin(); iteraotr != applianceList.end();
                 ++iteraotr) {
                if (iteraotr->name == displayName)
                    break;
            }
            applianceList.push_back(app);
        }
        settings.endGroup();
    }
    settings.endGroup();
}

void DrapWindowsData::browserBookmarkJsonNode(QJsonObject node)
{
    if (node.contains("name") && node.contains("url")) {
        QString url = node["url"].toString();
        QString title = node["name"].toString();
        QPair<QString, QString> titleAndUrl(title, url);
        insertBrowserBookmarkList(titleAndUrl);
    }

    if (node.contains("children")) {
        QJsonArray children = node["children"].toArray();
        for (const QJsonValue &child : children) {
            browserBookmarkJsonNode(child.toObject());
        }
    }
}

void DrapWindowsData::insertBrowserBookmarkList(const QPair<QString, QString> &titleAndUrl)
{
    auto find = std::find_if(browserBookmarkList.begin(), browserBookmarkList.end(),
                             [&titleAndUrl](const QPair<QString, QString> &mem) {
                                 if (mem.second == titleAndUrl.second) {
                                     return true;
                                 }
                                 return false;
                             });
    if (find == browserBookmarkList.end()) {
        browserBookmarkList.push_back(titleAndUrl);
        // DLOG << titleAndUrl.first << ": " << titleAndUrl.second;
    }
}

QPixmap DrapWindowsData::getAppIcon(const QString &path)
{
    if (path.isEmpty())
       return QPixmap();
    HICON hIcon;
    QString tempStr = path;
    if (ExtractIconExW(tempStr.toStdWString().c_str(), 0, NULL, &hIcon, 1) <= 0) {
        return QPixmap();
    }
   if (hIcon == 0) {
        DestroyIcon(hIcon);
        return QPixmap();
    }

    QPixmap pixmap = QtWin::fromHICON(hIcon);
    DestroyIcon(hIcon);
    if (pixmap.isNull())
        return pixmap;
    return pixmap.scaled(20, 20);
}

bool DrapWindowsData::containsAnyString(const QString &haystack, const QStringList &needles)
{
    for (const QString &needle : needles) {
        if (!haystack.contains(needle, Qt::CaseInsensitive)) {
            return false;
        }
    }
    return true;
}

QMap<QString, QString>
DrapWindowsData::RecommendedInstallationAppList(QMap<QString, QString> &notRecommendedList)
{
    notRecommendedList.clear();

    QList<WinApp> dataStructure;
    QList<WinApp> applist = getApplianceList();
    for (auto value : applist) {
        dataStructure.push_back(value);
    }

    QList<UosApp> MatchFielddata;
    getLinuxApplist(MatchFielddata);

    QMap<QString, QString> resultAPP;
    for (auto iterator = dataStructure.begin(); iterator != dataStructure.end();) {
        bool result;
        QString winApp = (*iterator).name;
        for (UosApp &uosValue : MatchFielddata) {
            QStringList valueB = uosValue.feature;
            result = containsAnyString(winApp, valueB);
            if (result) {
                resultAPP[uosValue.windowsName] = uosValue.UosName;
                iterator = dataStructure.erase(iterator);
                break;
            }
        }
        if (!result) {
            ++iterator;
        }
    }
    for (auto &value : dataStructure) {
        notRecommendedList[value.name] = value.iconPath;
    }
    return resultAPP;
}
