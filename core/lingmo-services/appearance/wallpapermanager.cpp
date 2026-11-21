#include "wallpapermanager.h"

#include <algorithm>
#include <filesystem>
#include <set>

#include "base/file.h"
#include "base/hash.h"
#include "base/paths.h"
#include "base/strings.h"

using namespace Lingmo::Base;
using namespace Lingmo::Base::strings;
namespace fs = std::filesystem;

// An absolute path, "." and ".." resolved by name, no trailing separator.
static std::string absolutePath(const std::string &path)
{
    std::error_code error;
    std::string result = fs::absolute(path, error).lexically_normal().string();
    while (result.size() > 1 && result.back() == '/')
        result.pop_back();
    return result;
}

static bool isFile(const std::string &path)
{
    std::error_code error;
    return fs::is_regular_file(path, error);
}

static bool isDirectory(const std::string &path)
{
    std::error_code error;
    return fs::is_directory(path, error);
}

static bool exists(const std::string &path)
{
    std::error_code error;
    return fs::exists(path, error);
}

static void removeRecursively(const std::string &path)
{
    std::error_code error;
    fs::remove_all(path, error);
}

static bool contains(const std::vector<std::string> &list, const std::string &value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

static void removeAll(std::vector<std::string> &list, const std::string &value)
{
    std::erase(list, value);
}

static std::vector<std::string> systemWallpaperDirectories()
{
    std::vector<std::string> directories;
    for (const std::string &dataPath : paths::dataDirectories())
        directories.push_back(absolutePath(dataPath) + "/backgrounds/lingmoos");
    return directories;
}

static bool isSystemWallpaper(const std::string &path)
{
    const std::string wallpaperPath = absolutePath(path);
    for (const std::string &directory : systemWallpaperDirectories()) {
        if (startsWith(wallpaperPath, directory + '/'))
            return true;
    }

    return false;
}

static std::string userWallpaperDirectory()
{
    return paths::dataHome() + "/backgrounds/lingmo/user";
}

static bool isManagedUserWallpaper(const std::string &path)
{
    return startsWith(absolutePath(path), userWallpaperDirectory() + '/');
}

// A folder the user added is mirrored here, one directory per folder: the
// originals may be moved or deleted, and the wallpapers have to survive that.
static std::string wallpaperFolderRoot()
{
    return paths::dataHome() + "/backgrounds/lingmo/folders";
}

static std::string wallpaperFolderStore(const std::string &folder)
{
    const std::string folderHash =
            CryptographicHash::hashHex(absolutePath(folder), CryptographicHash::Algorithm::Sha256).substr(0, 16);
    return wallpaperFolderRoot() + '/' + folderHash;
}

static bool isManagedFolderWallpaper(const std::string &path)
{
    return startsWith(absolutePath(path), wallpaperFolderRoot() + '/');
}

static std::vector<std::string> imagesIn(const std::string &directory)
{
    static const char *const suffixes[] = {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".tif", ".tiff", ".avif"};

    std::vector<std::string> result;
    std::error_code error;
    for (fs::recursive_directory_iterator it(directory, error), end; !error && it != end; it.increment(error)) {
        std::error_code typeError;
        if (!it->is_regular_file(typeError))
            continue;
        const std::string name = it->path().filename().string();
        const bool image = std::any_of(std::begin(suffixes), std::end(suffixes), [&name](const char *suffix) {
            return name.size() > std::string_view(suffix).size()
                    && endsWith(name, suffix, CaseSensitivity::Insensitive);
        });
        if (image)
            result.push_back(it->path().string());
    }

    return result;
}

// The copy's name carries a hash of the source path, so the same source always
// maps to the same copy and a renamed one to a new copy.
static std::string mirroredFileName(const std::string &source)
{
    const std::string sourcePath = absolutePath(source);
    const std::string pathHash =
            CryptographicHash::hashHex(sourcePath, CryptographicHash::Algorithm::Sha256).substr(0, 12);
    const std::string name = fs::path(sourcePath).filename().string();
    const size_t dot = name.rfind('.');
    const std::string baseName = dot == std::string::npos ? name : name.substr(0, dot);
    const std::string suffix = dot == std::string::npos ? std::string() : name.substr(dot + 1);
    std::string fileName = baseName + '-' + pathHash;
    if (!suffix.empty())
        fileName += '.' + toAsciiLower(suffix);
    return fileName;
}

static std::string copyWallpaperInto(const std::string &path, const std::string &destinationDirectory)
{
    if (!isFile(path))
        return path;

    const std::string sourcePath = absolutePath(path);
    if (startsWith(sourcePath, destinationDirectory + '/'))
        return sourcePath;

    std::error_code error;
    fs::create_directories(destinationDirectory, error);
    if (!isDirectory(destinationDirectory))
        return path;

    const std::string destinationPath = destinationDirectory + '/' + mirroredFileName(sourcePath);
    if (exists(destinationPath))
        return destinationPath;

    const std::optional<std::string> contents = file::read(sourcePath);
    if (!contents)
        return path;

    return file::writeAtomically(destinationPath, *contents) ? destinationPath : path;
}

// A wallpaper that ships with the system is already where it belongs.
static std::string importUserWallpaper(const std::string &path)
{
    if (isSystemWallpaper(path))
        return path;

    return copyWallpaperInto(path, userWallpaperDirectory());
}

WallpaperManager::WallpaperManager(Settings &settings)
    : m_settings(settings)
{
    m_wallpaperPath = m_settings.stringValue("Appearance/Wallpaper", "/usr/share/backgrounds/lingmoos/default.jpg");
    // A folder stays in the list even when its source is gone: what the list
    // shows is the copy taken when the folder was added.
    for (const std::string &folder : m_settings.stringListValue("Appearance/WallpaperFolders")) {
        if (folder.empty())
            continue;
        const std::string folderPath = absolutePath(folder);
        if (!folderPath.empty() && !contains(m_wallpaperFolders, folderPath))
            m_wallpaperFolders.push_back(folderPath);
    }
    m_settings.setValue("Appearance/WallpaperFolders", m_wallpaperFolders);

    for (const std::string &path : m_settings.stringListValue("Appearance/UserWallpapers")) {
        if (!exists(path))
            continue;

        const std::string importedPath = importUserWallpaper(path);
        if (isManagedUserWallpaper(importedPath))
            m_userWallpapers.push_back(importedPath);
    }
    if (exists(m_wallpaperPath) && !isSystemWallpaper(m_wallpaperPath)
        && !isManagedFolderWallpaper(m_wallpaperPath)) {
        m_wallpaperPath = importUserWallpaper(m_wallpaperPath);
        m_settings.setValue("Appearance/Wallpaper", m_wallpaperPath);
        if (isManagedUserWallpaper(m_wallpaperPath) && !contains(m_userWallpapers, m_wallpaperPath))
            m_userWallpapers.insert(m_userWallpapers.begin(), m_wallpaperPath);
    }
    std::vector<std::string> unique;
    for (const std::string &path : m_userWallpapers) {
        if (!contains(unique, path))
            unique.push_back(path);
    }
    m_userWallpapers = std::move(unique);
    m_settings.setValue("Appearance/UserWallpapers", m_userWallpapers);
    m_settings.sync();
    m_backgroundType = int(m_settings.intValue("Appearance/BackgroundType", 0));
    m_backgroundColor = m_settings.stringValue("Appearance/BackgroundColor", "#2B8ADA");
}

WallpaperManager::~WallpaperManager() = default;

template<typename T>
void WallpaperManager::write(const std::string &key, const T &value)
{
    m_settings.setValue("Appearance/" + key, value);
    m_settings.sync();
}

std::string WallpaperManager::wallpaper() const
{
    return m_wallpaperPath;
}

void WallpaperManager::setWallpaper(const std::string &path)
{
    const std::string wallpaperPath =
            isManagedFolderWallpaper(path) ? absolutePath(path) : importUserWallpaper(path);
    if (exists(wallpaperPath) && isManagedUserWallpaper(wallpaperPath)
        && !contains(m_userWallpapers, wallpaperPath)) {
        m_userWallpapers.insert(m_userWallpapers.begin(), wallpaperPath);
        write("UserWallpapers", m_userWallpapers);
        userWallpapersChanged.emit();
    }

    if (m_wallpaperPath != wallpaperPath) {
        m_wallpaperPath = wallpaperPath;
        write("Wallpaper", wallpaperPath);
        wallpaperChanged.emit(wallpaperPath);
    }
}

std::vector<std::string> WallpaperManager::userWallpapers() const
{
    return m_userWallpapers;
}

void WallpaperManager::removeUserWallpaper(const std::string &path)
{
    if (!isManagedUserWallpaper(path) || !contains(m_userWallpapers, path))
        return;

    std::error_code error;
    if (exists(path) && !fs::remove(path, error))
        return;

    removeAll(m_userWallpapers, path);
    write("UserWallpapers", m_userWallpapers);

    if (path != m_wallpaperPath) {
        userWallpapersChanged.emit();
        return;
    }

    std::string replacement;
    for (const std::string &candidate : m_userWallpapers) {
        if (exists(candidate)) {
            replacement = candidate;
            break;
        }
    }

    if (replacement.empty()) {
        for (const std::string &directory : systemWallpaperDirectories()) {
            const std::vector<std::string> wallpapers = imagesIn(directory);
            if (!wallpapers.empty()) {
                replacement = wallpapers.front();
                break;
            }
        }
    }

    m_wallpaperPath = replacement;
    write("Wallpaper", replacement);
    wallpaperChanged.emit(replacement);

    if (replacement.empty())
        setBackgroundType(1);

    userWallpapersChanged.emit();
}

std::vector<std::string> WallpaperManager::wallpaperFolders() const
{
    return m_wallpaperFolders;
}

void WallpaperManager::addWallpaperFolder(const std::string &path)
{
    const std::string folderPath = absolutePath(path);
    if (!isDirectory(folderPath) || contains(m_wallpaperFolders, folderPath))
        return;

    m_wallpaperFolders.push_back(folderPath);
    write("WallpaperFolders", m_wallpaperFolders);
    syncWallpaperFolder(folderPath);
    wallpaperFoldersChanged.emit();
}

void WallpaperManager::removeWallpaperFolder(const std::string &path)
{
    const std::string folderPath = absolutePath(path);
    if (!contains(m_wallpaperFolders, folderPath))
        return;

    // The folder's copies go with it, so a wallpaper still in use is kept by
    // moving it over to the user's own wallpapers first.
    const std::string store = wallpaperFolderStore(folderPath);
    if (startsWith(m_wallpaperPath, store + '/')) {
        const std::string importedPath = importUserWallpaper(m_wallpaperPath);
        if (isManagedUserWallpaper(importedPath)) {
            m_wallpaperPath = importedPath;
            write("Wallpaper", importedPath);
            if (!contains(m_userWallpapers, importedPath)) {
                m_userWallpapers.insert(m_userWallpapers.begin(), importedPath);
                write("UserWallpapers", m_userWallpapers);
                userWallpapersChanged.emit();
            }
            wallpaperChanged.emit(importedPath);
        }
    }

    removeRecursively(store);
    removeAll(m_wallpaperFolders, folderPath);
    write("WallpaperFolders", m_wallpaperFolders);
    wallpaperFoldersChanged.emit();
}

// The copies are what the folder shows; the source is read again on every
// query so that images added to it later turn up here too. Copying a folder
// takes long enough to be worth a thread: the caller is told to ask again
// once the copies are on disk.
void WallpaperManager::syncWallpaperFolder(const std::string &folder)
{
    if (m_syncingFolders.contains(folder))
        return;

    const std::string wallpaperInUse = m_wallpaperPath;
    m_syncingFolders[folder] = runInThread(
            [folder, wallpaperInUse] {
                const std::string store = wallpaperFolderStore(folder);
                std::vector<std::string> before = imagesIn(store);
                std::sort(before.begin(), before.end());
                std::set<std::string> expected;
                for (const std::string &source : imagesIn(folder)) {
                    copyWallpaperInto(source, store);
                    expected.insert(mirroredFileName(source));
                }
                // A copy whose source was renamed, replaced or removed has no use, unless it is the
                // wallpaper in use; a source folder that is gone as a whole keeps its copies.
                if (isDirectory(folder)) {
                    for (const std::string &copy : imagesIn(store)) {
                        std::error_code error;
                        if (copy != wallpaperInUse && !expected.contains(fs::path(copy).filename().string()))
                            fs::remove(copy, error);
                    }
                }
                std::vector<std::string> after = imagesIn(store);
                std::sort(after.begin(), after.end());
                return after != before;
            },
            [this, folder](bool copied) {
                onWallpaperFolderSynced(folder, copied);
            });
}

void WallpaperManager::onWallpaperFolderSynced(const std::string &folder, bool copied)
{
    m_syncingFolders.erase(folder);

    // The folder was removed while its copies were still being made.
    if (!contains(m_wallpaperFolders, folder)) {
        removeRecursively(wallpaperFolderStore(folder));
        return;
    }

    if (copied)
        wallpaperFolderImagesChanged.emit(folder);
}

std::vector<std::string> WallpaperManager::wallpaperFolderImages(const std::string &path)
{
    const std::string folderPath = absolutePath(path);
    if (!contains(m_wallpaperFolders, folderPath))
        return {};

    syncWallpaperFolder(folderPath);
    return imagesIn(wallpaperFolderStore(folderPath));
}

int WallpaperManager::backgroundType() const
{
    return m_backgroundType;
}

void WallpaperManager::setBackgroundType(int type)
{
    if (m_backgroundType != type) {
        m_backgroundType = type;
        write("BackgroundType", m_backgroundType);
        backgroundTypeChanged.emit();
    }
}

std::string WallpaperManager::backgroundColor() const
{
    return m_backgroundColor;
}

void WallpaperManager::setBackgroundColor(const std::string &color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        write("BackgroundColor", m_backgroundColor);
        backgroundColorChanged.emit();
    }
}
