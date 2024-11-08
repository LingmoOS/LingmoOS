/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <gio/gio.h>
#include <QIcon>
#include <QDebug>
#include <QDateTime>
#include <QDBusReply>
#include <gio/gdesktopappinfo.h>
#include <QDesktopServices>
#include <QMenu>
#include <QDir>
#include <QTranslator>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QThread>
#include <QAbstractListModel>

#include "recent-file-extension.h"
#include "event-track.h"
#include "libkydate.h"

#define LINGMO_APP_MANAGER_NAME            "com.lingmo.ProcessManager"
#define LINGMO_APP_MANAGER_PATH            "/com/lingmo/ProcessManager/AppLauncher"
#define LINGMO_APP_MANAGER_INTERFACE       "com.lingmo.ProcessManager.AppLauncher"
#define FREEDESKTOP_FILEMANAGER_NAME      "org.freedesktop.FileManager1"
#define FREEDESKTOP_FILEMANAGER_PATH      "/org/freedesktop/FileManager1"
#define FREEDESKTOP_FILEMANAGER_INTERFACE "org.freedesktop.FileManager1"

namespace LingmoUIMenu {

class RecentFileProvider : public QObject
{
Q_OBJECT
public:
    explicit RecentFileProvider(QObject *parent = nullptr);
    void dataProcess(QVector<RecentFile> &recentFiles);

public Q_SLOT:
    void getRecentData();
    void openFileByGFile(const QString &fileUrl);

Q_SIGNALS:
    void dataLoadCompleted(QVector<RecentFile> recentFiles);
};

class RecentFilesModel : public QAbstractListModel
{
Q_OBJECT
public:
    enum RoleMessage
    {
        UriRole = Qt::UserRole,
        NameRole,
        IconRole,
        PathRole,
        DateRole
    };

    explicit RecentFilesModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    QString getInfoId(const int &index);
    QStringList getAllInfoId();
    Q_INVOKABLE void updateData();

public Q_SLOTS:
    void updateRecentFiles(QVector<RecentFile> recentFiles);

private Q_SLOTS:
    void setDateFormat(QString formate = "");

private:
    QDBusInterface* m_dateInterface = nullptr;
    QVector<RecentFile> m_recentFileData;
    QString m_dateFormat;

Q_SIGNALS:
    void updateRecentData();
};

// GVFS 最近文件获取工具
class GVFSRecentFileData
{
public:
    static int s_queryFileNum;
    static GFile *s_recentFileRootDir;
    static GFileMonitor *s_recentFileMonitor;
    static void loadRecentFileASync(RecentFileProvider *p_recentFileProvider);
    static void fileMonitor(RecentFileProvider *p_recentFileProvider);
    static void removeRecentFileByInfoId(const QString &infoId);

private:
    static GAsyncReadyCallback enumerateFinish(GFile *file, GAsyncResult *res, RecentFileProvider *p_recentFileProvider);
    static GAsyncReadyCallback parseRecentFiles(GFileEnumerator *enumerator, GAsyncResult *res, RecentFileProvider *p_recentFileProvider);
    static void fileChangedCallback(GFileMonitor *monitor,
                                    GFile *file,
                                    GFile *other_file,
                                    GFileMonitorEvent event_type,
                                    RecentFileProvider *p_recentFileProvider);
};

int GVFSRecentFileData::s_queryFileNum = 100;
GFile *GVFSRecentFileData::s_recentFileRootDir = g_file_new_for_uri("recent:///");
GFileMonitor *GVFSRecentFileData::s_recentFileMonitor = nullptr;

void GVFSRecentFileData::fileMonitor(RecentFileProvider *p_recentFileProvider)
{
    GError *error = nullptr;
    s_recentFileMonitor = g_file_monitor_directory(GVFSRecentFileData::s_recentFileRootDir,
                                                   G_FILE_MONITOR_NONE,
                                                   nullptr,
                                                   &error);

    if (error) {
        qWarning() << "recentFile monitor creat error";
        g_error_free(error);
        return;
    }

    g_signal_connect(s_recentFileMonitor, "changed", G_CALLBACK(fileChangedCallback), p_recentFileProvider);
}

void GVFSRecentFileData::fileChangedCallback(GFileMonitor *monitor,
                                             GFile *file,
                                             GFile *other_file,
                                             GFileMonitorEvent event_type,
                                             RecentFileProvider *p_recentFileProvider)
{
    Q_UNUSED(monitor);
    Q_UNUSED(file);
    Q_UNUSED(other_file);

    switch (event_type) {
        case G_FILE_MONITOR_EVENT_DELETED:
        case G_FILE_MONITOR_EVENT_CREATED: {
            loadRecentFileASync(p_recentFileProvider);
            break;
        }
        default:
            break;
    }
}

void GVFSRecentFileData::removeRecentFileByInfoId(const QString &infoId)
{
    GFile *file = g_file_new_for_uri(infoId.toUtf8().constData());
    GError *err = nullptr;

    g_file_delete(file, nullptr, &err);

    g_object_unref(file);
    if (err) {
        qWarning() << "Recentfile Delete Error";
    }
}

void GVFSRecentFileData::loadRecentFileASync(RecentFileProvider *p_recentFileProvider)
{
    if (!s_recentFileRootDir) {
        qWarning() << "Can not find 'recent:///' dir.";
        return;
    }

    g_file_enumerate_children_async(s_recentFileRootDir,
                                    "*",
                                    G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT,
                                    nullptr, GAsyncReadyCallback(enumerateFinish),
                                    p_recentFileProvider);
}

GAsyncReadyCallback
GVFSRecentFileData::enumerateFinish(GFile *file, GAsyncResult *res, RecentFileProvider *p_recentFileProvider)
{
    GError *error = nullptr;
    GFileEnumerator *enumerator = g_file_enumerate_children_finish(file, res, &error);
    if (error) {
        qWarning() << "GVFSRecentFileData::enumerateFinish Error:" << error->message;
        g_error_free(error);
        return nullptr;
    }

    g_file_enumerator_next_files_async(enumerator, s_queryFileNum, G_PRIORITY_DEFAULT,
                                       nullptr, GAsyncReadyCallback(parseRecentFiles), p_recentFileProvider);

    g_object_unref(enumerator);

    return nullptr;
}

GAsyncReadyCallback
GVFSRecentFileData::parseRecentFiles(GFileEnumerator *enumerator, GAsyncResult *res,
                                     RecentFileProvider *p_recentFileProvider)
{
    GError *error = nullptr;
    GList *fileList = g_file_enumerator_next_files_finish(enumerator, res, &error);
    if (error) {
        qWarning() << "GVFSRecentFileData::parseRecentFiles Error:" << error->message;
        g_error_free(error);
        return nullptr;
    }

    QVector<RecentFile> recentFiles;
    if (!fileList) {
        p_recentFileProvider->dataProcess(recentFiles);
        return nullptr;
    }

    auto listIterator = fileList;
    while (listIterator) {
        RecentFile recentFile;
        GFileInfo *info = static_cast<GFileInfo *>(listIterator->data);
        GFile *file = g_file_enumerator_get_child(enumerator, info);
        recentFile.infoId = g_file_get_uri(file);
        g_object_unref(file);

        char *attribute = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        if (attribute) {
            recentFile.uri = attribute;
            g_free(attribute);
        }

        const char *fileName = g_file_info_get_display_name(info);
        if (fileName) {
            recentFile.name = fileName;
        }

        // in seconds since the UNIX epoch.
        recentFile.accessTime = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_TIME_MODIFIED);

        GIcon *icon = g_file_info_get_icon(info);
        if (icon) {
            const gchar *const *iconNames = g_themed_icon_get_names(G_THEMED_ICON(icon));
            if (iconNames) {
                auto iconNameIterator = iconNames;
                while (*iconNameIterator) {
                    if (QIcon::hasThemeIcon(*iconNameIterator)) {
                        recentFile.icon = QString(*iconNameIterator);
                        break;
                    } else {
                        ++iconNameIterator;
                    }
                }
            }
            g_object_unref(icon);
        }

        if (recentFile.icon.isEmpty()) {
            recentFile.icon = "text-plain";
        }

        recentFiles.append(recentFile);
        info = nullptr;
        listIterator = listIterator->next;
    }
    g_list_free(fileList);

    p_recentFileProvider->dataProcess(recentFiles);

    return nullptr;
}

// RecentFileExtension
RecentFileExtension::RecentFileExtension(QObject *parent) : WidgetExtension(parent)
{
    QString translationFile(QString(RECENT_FILE_TRANSLATION_DIR) + "recent-file_" + QLocale::system().name() + ".qm");
    if (QFile::exists(translationFile)) {
        QTranslator *translator = new QTranslator(this);
        translator->load(translationFile);
        QCoreApplication::installTranslator(translator);
    }

    m_metadata.insert(WidgetMetadata::Id, "recent-file");
    m_metadata.insert(WidgetMetadata::Name, tr("Recent Files"));
    m_metadata.insert(WidgetMetadata::Tooltip, tr("Recent Files"));
    m_metadata.insert(WidgetMetadata::Version, "1.0.0");
    m_metadata.insert(WidgetMetadata::Description, "recent-file");
    m_metadata.insert(WidgetMetadata::Main, "qrc:///extensions/RecentFileExtension.qml");
    m_metadata.insert(WidgetMetadata::Type, WidgetMetadata::Widget);
    m_metadata.insert(WidgetMetadata::Flag, WidgetMetadata::OnlySmallScreen);

    qRegisterMetaType<LingmoUIMenu::RecentFilesModel *>("RecentFilesModel*");
    qRegisterMetaType<QVector<RecentFile> >("QVector<RecentFile>");

    m_recentFilesProviderThread = new QThread(this);
    m_recentFilesModel = new RecentFilesModel(this);
    m_recentFileProvider = new RecentFileProvider();

    if (!m_recentFilesProviderThread || !m_recentFilesModel || !m_recentFileProvider) {
        qWarning() << "recentfile construction error";
        return;
    }

    m_recentFilesProviderThread->start();
    m_recentFileProvider->moveToThread(m_recentFilesProviderThread);

    connect(this, &RecentFileExtension::loadRecentFiles, m_recentFileProvider, &RecentFileProvider::getRecentData);
    connect(m_recentFilesModel, &RecentFilesModel::updateRecentData, m_recentFileProvider,
            &RecentFileProvider::getRecentData);
    connect(m_recentFileProvider, &RecentFileProvider::dataLoadCompleted, m_recentFilesModel,
            &RecentFilesModel::updateRecentFiles);
    connect(this, &RecentFileExtension::openFileASync, m_recentFileProvider, &RecentFileProvider::openFileByGFile);

    m_data.insert("recentFilesModel", QVariant::fromValue(m_recentFilesModel));

    GVFSRecentFileData::fileMonitor(m_recentFileProvider);
    Q_EMIT loadRecentFiles();
}

RecentFileExtension::~RecentFileExtension()
{
    if (m_recentFilesProviderThread) {
        m_recentFilesProviderThread->quit();
        m_recentFilesProviderThread->wait();
    }

    if (m_recentFileProvider) {
        delete m_recentFileProvider;
        m_recentFileProvider = nullptr;
    }

    if (GVFSRecentFileData::s_recentFileRootDir) {
        g_object_unref(GVFSRecentFileData::s_recentFileRootDir);
    }

    if (GVFSRecentFileData::s_recentFileMonitor) {
        g_object_unref(GVFSRecentFileData::s_recentFileMonitor);
    }
}

MetadataMap RecentFileExtension::metadata() const
{
    return m_metadata;
}

int RecentFileExtension::index() const
{
    return 1;
}

QVariantMap RecentFileExtension::data()
{
    return m_data;
}

void RecentFileExtension::receive(const QVariantMap &data)
{
    if (data.value("action").toString() == "contextMenu") {
        QString path = data.value("url").toString();
        int index = data.value("index").toInt();
        creatContextMenu(path, index);
        return;

    }

    if (data.value("action").toString() == "closeMenu") {
        if (m_contextMenu) {
            m_contextMenu.data()->close();
        }
    }

    if (data.value("action").toString() == "emptyAreaContextMenu") {
        creatEmptyAreaContextMenu();
        return;
    }

    if (data.value("action").toString() == "openFile") {
        QString path = data.value("url").toString();
        openFile(path);
    }
}

void RecentFileExtension::openFile(const QString &fileUrl)
{
    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME, LINGMO_APP_MANAGER_PATH, LINGMO_APP_MANAGER_INTERFACE, "LaunchDefaultAppWithUrl");
    message << fileUrl;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [fileUrl, this] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            Q_EMIT openFileASync(fileUrl);
        }

        self->deleteLater();
    });
}

void RecentFileExtension::creatContextMenu(const QString &path, const int &index)
{
    if (m_contextMenu) {
        m_contextMenu.data()->close();
        return;
    }

    QMenu *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *open = new QAction(QIcon::fromTheme("document-open-symbolic"), tr("Open"), menu);
    QAction *remove = new QAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Remove from list"), menu);
    QAction *clear = new QAction(QIcon::fromTheme("edit-delete-symbolic"), tr("Clear list"), menu);
    QAction *directory = new QAction(tr("Open the directory where the file is located"), menu);

    connect(open, &QAction::triggered, this, [this, path]() {
        openFile(path);
    });

    connect(remove, &QAction::triggered, this, [this, index]() {
        GVFSRecentFileData::removeRecentFileByInfoId(m_recentFilesModel->getInfoId(index));
        EventTrack::instance()->sendDefaultEvent("remove_recent_file", "RightClickMenu");
    });

    connect(clear, &QAction::triggered, this, [this]() {
        QStringList infoIdList = m_recentFilesModel->getAllInfoId();
        for (const QString &infoId : infoIdList) {
            GVFSRecentFileData::removeRecentFileByInfoId(infoId);
        }
        EventTrack::instance()->sendDefaultEvent("clear_recent_files", "RightClickMenu");
    });

    connect(directory, &QAction::triggered, this, [this, path]() {
        QDBusMessage message = QDBusMessage::createMethodCall(FREEDESKTOP_FILEMANAGER_NAME,
                                                              FREEDESKTOP_FILEMANAGER_PATH,
                                                              FREEDESKTOP_FILEMANAGER_INTERFACE, "ShowFolders");
        message << QStringList(path) << "lingmo-menu-recent-file";
        QDBusConnection::sessionBus().asyncCall(message);
    });

    menu->addAction(open);
    menu->addSeparator();
    menu->addAction(remove);
    menu->addAction(clear);
    menu->addSeparator();
    menu->addAction(directory);

    m_contextMenu = menu;

    menu->popup(QCursor::pos());
}

void RecentFileExtension::creatEmptyAreaContextMenu()
{
    if (m_contextMenu) {
        m_contextMenu.data()->close();
        return;
    }

    QMenu *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *clear = new QAction(QIcon::fromTheme("edit-delete-symbolic"), tr("Clear list"), menu);

    connect(clear, &QAction::triggered, this, [this]() {
        QStringList infoIdList = m_recentFilesModel->getAllInfoId();
        for (const QString &infoId : infoIdList) {
            GVFSRecentFileData::removeRecentFileByInfoId(infoId);
        }
        EventTrack::instance()->sendDefaultEvent("clear_recent_files", "RightClickMenu");
    });


    menu->addAction(clear);

    m_contextMenu = menu;

    menu->popup(QCursor::pos());
}

RecentFilesModel::RecentFilesModel(QObject *parent) : QAbstractListModel(parent)
{
    QDBusConnection::sessionBus().connect("com.lingmo.lingmosdk.DateServer",
                                          "/com/lingmo/lingmosdk/Date",
                                          "com.lingmo.lingmosdk.DateInterface",
                                          "ShortDateSignal",
                                          this, SLOT(setDateFormat(QString)));
    setDateFormat();
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const
{
    return m_recentFileData.count();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_recentFileData.count()) {
        return {};
    }

    switch (role) {
        case UriRole:
            return m_recentFileData.at(row).uri;
        case NameRole:
            return m_recentFileData.at(row).name;
        case IconRole:
            return m_recentFileData.at(row).icon;
        case PathRole: {
            QUrl baseUrl(m_recentFileData.at(row).uri);
            return baseUrl.adjusted(QUrl::RemoveFilename).path();
        }
        case DateRole: {
            QDateTime time = QDateTime::fromSecsSinceEpoch(m_recentFileData.at(row).accessTime);
            return time.toString(m_dateFormat);
        }
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(UriRole, "uri");
    names.insert(NameRole, "name");
    names.insert(IconRole, "icon");
    names.insert(PathRole, "path");
    names.insert(DateRole, "date");
    return names;
}

QString RecentFilesModel::getInfoId(const int &index)
{
    return m_recentFileData.at(index).infoId;
}

QStringList RecentFilesModel::getAllInfoId()
{
    QStringList infoIdList;
    for (const RecentFile &data : m_recentFileData) {
        infoIdList.append(data.infoId);
    }
    return infoIdList;
}

void RecentFilesModel::updateData()
{
    Q_EMIT updateRecentData();
}

void RecentFilesModel::updateRecentFiles(QVector<RecentFile> recentFiles)
{
    beginResetModel();
    m_recentFileData.swap(recentFiles);
    endResetModel();
}

void RecentFilesModel::setDateFormat(QString formate)
{
    if (formate == "") {
        char* date = kdk_system_get_shortformat();
        m_dateFormat = date;
        free(date);
    } else {
        m_dateFormat = formate;
    }

    QVector<int> vec;
    vec.append(RoleMessage::DateRole);
    Q_EMIT dataChanged(index(0,0), index(m_recentFileData.length() - 1, 0), vec);
}

RecentFileProvider::RecentFileProvider(QObject *parent) : QObject(parent)
{

}

void RecentFileProvider::dataProcess(QVector<RecentFile> &recentFiles)
{
    std::sort(recentFiles.begin(), recentFiles.end(), [](const RecentFile &a, const RecentFile &b) {
        return a.accessTime > b.accessTime;
    });

    Q_EMIT dataLoadCompleted(recentFiles);
}

void RecentFileProvider::getRecentData()
{
    GVFSRecentFileData::loadRecentFileASync(this);
}

void RecentFileProvider::openFileByGFile(const QString &fileUrl)
{
    GFile *file = g_file_new_for_uri(fileUrl.toUtf8().constData());
    if (!file) {
        return;
    }

    GFileInfo *fileInfo = g_file_query_info(file, "standard::*," G_FILE_ATTRIBUTE_ID_FILE, G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    if (!fileInfo) {
        g_object_unref(file);
        return;
    }

    QString mimeType(g_file_info_get_content_type(fileInfo));
    if (mimeType.isEmpty()) {
        if (g_file_info_has_attribute(fileInfo, "standard::fast-content-type")) {
            mimeType = g_file_info_get_attribute_string(fileInfo, "standard::fast-content-type");
        }
    }

    GError *error = NULL;
    GAppInfo *info = NULL;

    QString mimeAppsListPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/mimeapps.list";
    GKeyFile *keyfile = g_key_file_new();
    gboolean ret = g_key_file_load_from_file(keyfile, mimeAppsListPath.toUtf8(), G_KEY_FILE_NONE, &error);

    if (!ret) {
        qWarning() << "load mimeapps list error msg" << error->message;
        info = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
        g_error_free(error);

    } else {
        gchar *desktopApp = g_key_file_get_string(keyfile, "Default Applications", mimeType.toUtf8(), &error);

        if (NULL == desktopApp) {
            info = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);

        } else {
            info = (GAppInfo *) g_desktop_app_info_new(desktopApp);
            g_free(desktopApp);
        }
    }

    g_key_file_free(keyfile);

    bool success = false;
    if (G_IS_APP_INFO(info)) {
        GList *files = g_list_alloc();
        g_list_append(files, file);

        success = g_app_info_launch(info, files, nullptr, nullptr);

        g_list_free(files);
    }

    if (!success) {
        QDesktopServices::openUrl(fileUrl);
    }

    g_object_unref(file);
    g_object_unref(info);
}

RecentFileExtensionPlugin::~RecentFileExtensionPlugin()
{

}

QString RecentFileExtensionPlugin::id()
{
    return "recent-file";
}

WidgetExtension *RecentFileExtensionPlugin::createWidgetExtension()
{
    return new RecentFileExtension;
}

ContextMenuExtension *RecentFileExtensionPlugin::createContextMenuExtension()
{
    return nullptr;
}
} // LingmoUIMenu

#include "recent-file-extension.moc"
