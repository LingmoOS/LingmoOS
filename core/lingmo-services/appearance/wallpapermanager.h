
#include <map>
#include <string>
#pragma once
#include <vector>

#include "base/settings.h"
#include "base/signal.h"
#include "base/thread_pool.h"

/**
 * The desktop background: the wallpaper in use, the wallpapers the user
 * brought in, and the folders they added as libraries. ThemeManager owns the
 * D-Bus interface and hands this part of it over here.
 *
 * Every wallpaper the user picks is copied into the session's own data
 * directory, so moving or deleting the original leaves the desktop alone.
 */
class WallpaperManager
{
public:
    explicit WallpaperManager(Lingmo::Base::Settings &settings);
    ~WallpaperManager();

    WallpaperManager(const WallpaperManager &) = delete;
    WallpaperManager &operator=(const WallpaperManager &) = delete;

    std::string wallpaper() const;
    void setWallpaper(const std::string &path);

    std::vector<std::string> userWallpapers() const;
    void removeUserWallpaper(const std::string &path);

    std::vector<std::string> wallpaperFolders() const;
    void addWallpaperFolder(const std::string &path);
    void removeWallpaperFolder(const std::string &path);
    std::vector<std::string> wallpaperFolderImages(const std::string &path);

    int backgroundType() const;
    void setBackgroundType(int type);

    std::string backgroundColor() const;
    void setBackgroundColor(const std::string &color);

    Lingmo::Base::Signal<std::string> wallpaperChanged;
    Lingmo::Base::Signal<> userWallpapersChanged;
    Lingmo::Base::Signal<> wallpaperFoldersChanged;
    Lingmo::Base::Signal<std::string> wallpaperFolderImagesChanged;
    Lingmo::Base::Signal<> backgroundTypeChanged;
    Lingmo::Base::Signal<> backgroundColorChanged;

private:
    template<typename T>
    void write(const std::string &key, const T &value);
    void syncWallpaperFolder(const std::string &folder);
    void onWallpaperFolderSynced(const std::string &folder, bool copied);

    Lingmo::Base::Settings &m_settings;

    std::string m_wallpaperPath;
    std::vector<std::string> m_userWallpapers;
    std::vector<std::string> m_wallpaperFolders;
    // The copies being made, by folder.
    std::map<std::string, Lingmo::Base::Job> m_syncingFolders;

    int m_backgroundType = 0;
    std::string m_backgroundColor;
};

