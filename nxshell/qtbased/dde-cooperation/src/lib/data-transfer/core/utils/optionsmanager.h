#ifndef OPTIONSMANAGER_H
#define OPTIONSMANAGER_H

#include <QMap>
#include <QObject>

class OptionsManager : public QObject
{
    Q_OBJECT

public:
    OptionsManager();
    ~OptionsManager();

    static OptionsManager *instance();

    QMap<QString, QStringList> getUserOptions() const;
    QStringList getUserOption(const QString &option) const;

    void setUserOptions(const QMap<QString, QStringList> &value);
    void addUserOption(const QString &option, const QStringList &value);
    void clear();
private:
    QMap<QString, QStringList> userOptions;
};

namespace Options {
inline constexpr char kUser[]{ "user" };
inline constexpr char kFile[]{ "file" };
inline constexpr char kApp[]{ "app" };
inline constexpr char KSelectFileSize[]{ "selectFileSize" };
inline constexpr char kBrowserBookmarks[]{ "browserbookmarks" };
inline constexpr char kConfig[]{ "config" };
inline constexpr char kBackupFileSavePath[]{ "backupFileSavePath" };
inline constexpr char kBackupFileName[]{ "backupFileName" };
inline constexpr char kTransferMethod[]{ "transferMethod" };
inline constexpr char KUserDataInfoJsonPath[]{ "userDataInfoJsonPath" };
inline constexpr char KWallpaperPath[]{ "wallpaperPath" };
inline constexpr char KBookmarksJsonPath[]{ "bookmarksJsonPath" };
inline constexpr char KBackupFileSize[] {"backupFileSize"};
inline constexpr char kTransferFileList[]{"transferFileList"};
} // namespace Options

namespace TransferMethod {
inline constexpr char kLocalExport[]{ "LocalExport" };
inline constexpr char kNetworkTransmission[]{ "NetworkTransmission" };
} // namespace TransferMethod
#endif
