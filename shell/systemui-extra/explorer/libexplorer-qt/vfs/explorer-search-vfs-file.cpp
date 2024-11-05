/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "explorer-search-vfs-file.h"
#include "explorer-search-vfs-file-enumerator.h"
#include "file-enumerator.h"
#include "file-utils.h"
#include "search-vfs-manager.h"
#include <QString>
#include <QDebug>
#include <QThread>

/* -- GFileIface -- */
static void explorer_search_vfs_file_g_file_iface_init(GFileIface *iface);

static GFile *explorer_search_vfs_file_dup(GFile *file);
static guint explorer_search_vfs_file_hash(GFile *file);//unused
static gboolean explorer_search_vfs_file_equal(GFile *file1, GFile *file2);//unused
static gboolean explorer_search_vfs_file_is_native(GFile *file);//unused
static gboolean explorer_search_vfs_file_has_uri_scheme(GFile *file, const char *uri_scheme);//unused
static char *explorer_search_vfs_file_get_uri_sheceme(GFile *file);
static char *explorer_search_vfs_file_get_basename(GFile *file);//unused
static char *explorer_search_vfs_file_get_path(GFile *file);//unused
static char *explorer_search_vfs_file_get_uri(GFile *file);
static char *explorer_search_vfs_file_get_parse_name(GFile *file);//unused
static GFile *explorer_search_vfs_file_get_parent(GFile *file);//unused
static gboolean explorer_search_vfs_file_prefix_matches(GFile *prefix, GFile *file);//unused
static char *explorer_search_vfs_file_get_relative_path(GFile *file, const char *relative_path);//unused
static GFile *explorer_search_vfs_file_resolve_relative_path(GFile *file,
        const char *relative_path);
static GFile *explorer_search_vfs_file_get_child_for_display_name (GFile *file,
        const char *display_name,
        GError **err);//unused

static GFileInfo *explorer_search_vfs_file_query_info(GFile *file,
        const char *attributes,
        GFileQueryInfoFlags flags,
        GCancellable *cancellable,
        GError **error);

/* -- Implement Iface -- */
G_DEFINE_TYPE_EXTENDED(PeonySearchVFSFile,
                       explorer_search_vfs_file,
                       G_TYPE_OBJECT,
                       0,
                       G_ADD_PRIVATE(PeonySearchVFSFile)
                       G_IMPLEMENT_INTERFACE(G_TYPE_FILE, explorer_search_vfs_file_g_file_iface_init));

static void file_dispose(GObject *object)
{
    auto vfs_file = PEONY_SEARCH_VFS_FILE(object);
    if (vfs_file->priv->uri) {
        g_free(vfs_file->priv->uri);
    }
}

static void explorer_search_vfs_file_class_init (PeonySearchVFSFileClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = file_dispose;
}

GFileEnumerator *explorer_search_vfs_file_enumerate_children(GFile *file,
        const char *attribute,
        GFileQueryInfoFlags flags,
        GCancellable *cancellable,
        GError **error)
{
    qDebug() << "explorer_search_vfs_file_enumerate_children";
    auto search_vfs_file = PEONY_SEARCH_VFS_FILE(file);

    return explorer_search_vfs_file_enumerate_children_internal(file, attribute, flags, cancellable, error);
}

GFileInfo *explorer_search_vfs_file_query_info(GFile *file,
        const char *attributes,
        GFileQueryInfoFlags flags,
        GCancellable *cancellable,
        GError **error)
{
    auto vfs_file = PEONY_SEARCH_VFS_FILE(file);
    qDebug()<<vfs_file->priv->uri;
    GFileInfo *info = g_file_info_new();
    g_file_info_set_name(info, vfs_file->priv->uri);
    auto icon = g_themed_icon_new("search");
    g_file_info_set_icon(info, icon);
    g_object_unref(icon);
    g_file_info_set_display_name(info, " ");
    g_file_info_set_file_type(info, G_FILE_TYPE_DIRECTORY);
    return info;
}


static void explorer_search_vfs_file_init(PeonySearchVFSFile *self)
{
    PeonySearchVFSFilePrivate *priv = (PeonySearchVFSFilePrivate*)explorer_search_vfs_file_get_instance_private(self);
    self->priv = priv;
    priv->uri = nullptr;
}

char *explorer_search_vfs_file_get_uri(GFile *file)
{
    auto vfs_file = PEONY_SEARCH_VFS_FILE(file);
    return g_strdup(vfs_file->priv->uri);
}

GFile *explorer_search_vfs_file_get_parent(GFile *file)
{
    Q_UNUSED(file);
    return nullptr;
}

gboolean explorer_search_vfs_file_is_native(GFile *file)
{
    return false;
}

static char *explorer_search_vfs_file_get_uri_sheceme(GFile *file)
{
    return g_strdup("search");
}

static void explorer_search_vfs_file_g_file_iface_init(GFileIface *iface)
{
    qDebug() << "explorer_search_vfs_file_g_file_iface_init";
    iface->dup = explorer_search_vfs_file_dup;
    iface->get_parent = explorer_search_vfs_file_get_parent;
    iface->is_native = explorer_search_vfs_file_is_native;
    iface->enumerate_children = explorer_search_vfs_file_enumerate_children;
    iface->query_info = explorer_search_vfs_file_query_info;
    iface->get_uri = explorer_search_vfs_file_get_uri;
    iface->get_path = explorer_search_vfs_file_get_path;
    iface->resolve_relative_path = explorer_search_vfs_file_resolve_relative_path;
    iface->get_uri_scheme = explorer_search_vfs_file_get_uri_sheceme;
}

/// dup, get_parent and is_native(and so on) is used in explorer-qt,
/// such as FileEnumerator and FileInfo, so i must need
/// implement these search vfs file's functions.
GFile *explorer_search_vfs_file_dup(GFile *file)
{
    if (!PEONY_IS_SEARCH_VFS_FILE(file)) {
        //this should not be happened.
        return g_file_new_for_uri("search:///");
    }
    auto vfs_file = PEONY_SEARCH_VFS_FILE(file);
    auto dup = PEONY_SEARCH_VFS_FILE(g_object_new(PEONY_TYPE_SEARCH_VFS_FILE, nullptr));
    dup->priv->uri = g_strdup(vfs_file->priv->uri);
    return G_FILE(dup);
}

GFile *explorer_search_vfs_file_resolve_relative_path(GFile *file, const char *relative_path)
{
    Q_UNUSED(file);
    //FIXME: maybe i should put the resolve to vfs look up method?
    QString tmp = relative_path;
    if (tmp.contains("real-uri:")) {
        tmp = tmp.remove("real-uri:");
        return g_file_new_for_uri(tmp.toUtf8().constData());
    }
    return g_file_new_for_uri("serarch:///");
}

GFile *explorer_search_vfs_file_new_for_uri(const char *uri)
{
    auto search_vfs_file = PEONY_SEARCH_VFS_FILE(g_object_new(PEONY_TYPE_SEARCH_VFS_FILE, nullptr));
    search_vfs_file->priv->uri = g_strdup(uri);

    return G_FILE(search_vfs_file);
}

char *explorer_search_vfs_file_get_path(GFile *file)
{
    return nullptr;
}

void explorer_search_vfs_file_enumerator_parse_uri(PeonySearchVFSFileEnumerator *enumerator,
        const char *uri)
{
    PeonySearchVFSFileEnumeratorPrivate *details = enumerator->priv;

    *details->search_vfs_directory_uri = uri;

    auto manager = Peony::SearchVFSManager::getInstance();
    if (manager->hasHistory(uri)) {
        auto uris = manager->getHistroyResults(uri);
        for (auto uri: uris) {
            details->enumerate_queue->enqueue(uri);
        }
        //do not parse uri, not neccersary
        return;
    }

    QStringList args = details->search_vfs_directory_uri->split("&", QString::SkipEmptyParts);
    QStringList paths;
    QStringList keyWords;

    if (args.at(1).contains("name_regexp=") && 12 == args.at(1).size()
            && !details->search_vfs_directory_uri->contains("search_hidden=")
            && !details->search_vfs_directory_uri->contains("use_regexp=")
            && !details->search_vfs_directory_uri->contains("case_sensitive=")
            && !details->search_vfs_directory_uri->contains("extend_regexp=")
            && !details->search_vfs_directory_uri->contains("save=")) {
        int indexOne = details->search_vfs_directory_uri->indexOf("name_regexp=");
        int indexSecond = details->search_vfs_directory_uri->indexOf("&recursive=");
        if (indexSecond > indexOne) {
            QString nameRegExp = details->search_vfs_directory_uri->mid(indexOne, indexSecond - indexOne);
            args.replace(1, nameRegExp);
        }
    }
#ifdef LINGMO_LINGMO_SEARCH
    details->search_engine = args.contains("search_engine=1");
#endif

    //we should judge case sensitive, then we confirm the regexp when
    //we match file in file enumeration.
    for (auto arg: args) {
        //qDebug()<<arg;
        if (arg.contains("search_hidden=")) {
            if (arg.endsWith("1")) {
                details->search_hidden = true;
            }
            continue;
        }
        if (arg.contains("use_regexp=")) {
            if (arg.endsWith("0")) {
                details->use_regexp = false;
            }
            continue;
        }

        if (arg.contains("case_sensitive=")) {
            if (arg.endsWith("1")) {
                details->case_sensitive = false;
            }
            continue;
        }

        if (arg.contains("name_regexp=")) {
            if (arg == "name_regexp=") {
                continue;
            }
            QString tmp = arg;
            tmp = tmp.remove("name_regexp=");
            details->name_regexp = new QRegExp(tmp);
#ifdef LINGMO_LINGMO_SEARCH
            //Add search keys
            if (details->search_engine) {
                keyWords.clear();
                keyWords.append(tmp);
            }
#endif
            continue;
        }

        if (arg.contains("extend_regexp="))
        {
            if (arg == "extend_regexp=")
                continue;

            QString tmp = arg;
            tmp = tmp.remove("extend_regexp=");
            QStringList keys = tmp.split(",", QString::SkipEmptyParts);
            for(auto key : keys)
            {
                details->name_regexp_extend_list->append(new QRegExp(key));
            }
            continue;
        }

        if (arg.contains("content_regexp=")) {
            if (arg == "content_regexp=") {
                continue;
            }
            QString tmp = arg;
            tmp = tmp.remove("content_regexp=");
            details->content_regexp = new QRegExp(tmp);
            continue;
        }

        if (arg.contains("save=")) {
            if (arg.endsWith("1")) {
                details->save_result = true;
            }
            continue;
        }

        if (arg.contains("recursive=")) {
            if (arg.endsWith("1")) {
                details->recursive = true;
            }
            else
                details->save_result = false;
            continue;
        }

        if (arg.contains("search_uris=")) {
            QString tmp = arg;
            QString tmpPath;
            tmp.remove("search:///");
            tmp.remove("search_uris=");
#ifdef LINGMO_LINGMO_SEARCH
            //Add search path
            if (details->search_engine) {
                tmpPath = tmp;
                tmpPath.remove("file://");
                paths.clear();
                paths.append(tmpPath);
            }
#endif
            QStringList uris = tmp.split(",", QString::SkipEmptyParts);
            for (auto uri: uris) {
                //NOTE: we should enumerate the search uris and add
                //the children into queue first. otherwise we could
                //not judge wether we should search recursively.
                Peony::FileEnumerator e;
                e.setEnumerateDirectory(uri);
                e.enumerateSync();
                auto uris1 = e.getChildrenUris();
                for (auto uri1 : uris1) {
                    details->enumerate_queue->enqueue(uri1);
                }
            }
        }
    }

    Qt::CaseSensitivity sensitivity = details->case_sensitive? Qt::CaseSensitive: Qt::CaseInsensitive;

    if (!details->name_regexp) {
        //details->name_regexp = new QRegExp;
    } else {
        details->name_regexp->setCaseSensitivity(sensitivity);
    }

    if (!details->content_regexp) {
        //details->content_regexp = new QRegExp;
    } else {
        details->content_regexp->setCaseSensitivity(sensitivity);
    }

    if (details->name_regexp_extend_list->count() >0)
    {
        for(int i=0; i<details->name_regexp_extend_list->count(); i++)
        {
            details->name_regexp_extend_list->at(i)->setCaseSensitivity(sensitivity);
        }
    }

#ifdef LINGMO_LINGMO_SEARCH
    if (nullptr != details->m_search && details->search_engine) {
        details->m_search->clearAllConditions();
        details->m_queue->clear();

        for (QString &dir : paths) {
            dir = Peony::FileUtils::urlDecode(dir);
            details->m_search->addSearchDir(dir);
        }

        for (QString &key : keyWords) {
            details->m_search->addKeyword(key);
        }

        details->m_search->initSearchPlugin(LingmoUISearch::SearchProperty::SearchType::File);
        details->m_search->initSearchPlugin(LingmoUISearch::SearchProperty::SearchType::FileContent);
        details->m_queue = details->m_search->init();
        details->m_search->setMaxResultNum(9999999);

        qDebug() << "thraed:" << QThread::currentThreadId();

//        QEventLoop *l = new QEventLoop;
//        details->m_search->connect(details->m_search, &LingmoUISearch::LingmoUISearchTask::searchFinished, [l]{
//            qDebug()<<"search finished";
//            l->exit(0);
//        });
//        auto searchId = details->m_search->startSearch(LingmoUISearch::SearchProperty::SearchType::File);
//        l->exec();
//        l->deleteLater();
    }
#endif
}

GFileEnumerator *explorer_search_vfs_file_enumerate_children_internal(GFile *file,
        const char *attributes,
        GFileQueryInfoFlags flags,
        GCancellable *cancellable,
        GError **error)
{
    auto vfs_file = PEONY_SEARCH_VFS_FILE(file);

    //we should add enumerator container when search vfs enumerator created.
    //otherwise g_enumerator_get_child will went error.
    auto enumerator = PEONY_SEARCH_VFS_FILE_ENUMERATOR(g_object_new(PEONY_TYPE_SEARCH_VFS_FILE_ENUMERATOR,
                      "container", file,
                      nullptr));
    explorer_search_vfs_file_enumerator_parse_uri(enumerator, vfs_file->priv->uri);
    //parse uri, add top folder uri to queue;
    return G_FILE_ENUMERATOR(enumerator);
}
