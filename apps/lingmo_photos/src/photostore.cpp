// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "photostore.h"

#include <base/paths.h>
#include <sqlite3.h>

#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace Photos {
namespace {
struct Statement {
    sqlite3_stmt* value = nullptr;
    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &value, nullptr) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(db));
    }
    ~Statement() { sqlite3_finalize(value); }
};
std::string text(sqlite3_stmt* statement, int column) {
    const auto* value = sqlite3_column_text(statement, column);
    return value ? reinterpret_cast<const char*>(value) : std::string();
}
void execute(sqlite3* db, const char* sql) {
    char* error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : "SQLite error";
        sqlite3_free(error);
        throw std::runtime_error(message);
    }
}
} // namespace

struct PhotoStore::Private { sqlite3* db = nullptr; };

PhotoStore::PhotoStore(std::string path)
    : m_private(new Private)
{
    if (path.empty()) {
        const char* data = std::getenv("XDG_DATA_HOME");
        path = data ? data : std::string();
        if (path.empty()) {
            const char* home = std::getenv("HOME");
            path = std::string(home ? home : ".") + "/.local/share";
        }
        path += "/lingmo-photos/library.db";
    }
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    if (sqlite3_open(path.c_str(), &m_private->db) != SQLITE_OK)
        throw std::runtime_error("Unable to open photo database");
    execute(m_private->db,
            "PRAGMA journal_mode=WAL;"
            "CREATE TABLE IF NOT EXISTS photos(id INTEGER PRIMARY KEY, path TEXT UNIQUE NOT NULL, name TEXT NOT NULL,"
            "folder TEXT NOT NULL, captured TEXT NOT NULL, format TEXT NOT NULL, width INTEGER NOT NULL DEFAULT 0,"
            "height INTEGER NOT NULL DEFAULT 0, bytes INTEGER NOT NULL DEFAULT 0, favorite INTEGER NOT NULL DEFAULT 0,"
            "trashed INTEGER NOT NULL DEFAULT 0, indexed_at INTEGER NOT NULL DEFAULT (unixepoch()));"
            "CREATE TABLE IF NOT EXISTS albums(id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, favorite INTEGER NOT NULL DEFAULT 0);"
            "CREATE TABLE IF NOT EXISTS album_photos(album_id INTEGER NOT NULL, photo_id INTEGER NOT NULL, PRIMARY KEY(album_id,photo_id), FOREIGN KEY(album_id) REFERENCES albums(id) ON DELETE CASCADE, FOREIGN KEY(photo_id) REFERENCES photos(id) ON DELETE CASCADE);"
            "CREATE INDEX IF NOT EXISTS photos_captured ON photos(captured);"
            "CREATE INDEX IF NOT EXISTS photos_folder ON photos(folder);");
}

PhotoStore::~PhotoStore() {
    if (m_private->db)
        sqlite3_close(m_private->db);
    delete m_private;
}

void PhotoStore::upsert(const Photo& photo) {
    Statement statement(m_private->db,
                        "INSERT INTO photos(path,name,folder,captured,format,width,height,bytes,favorite,trashed)"
                        " VALUES(?,?,?,?,?,?,?,?,?,?) ON CONFLICT(path) DO UPDATE SET name=excluded.name,"
                        "folder=excluded.folder,captured=excluded.captured,format=excluded.format,width=excluded.width,"
                        "height=excluded.height,bytes=excluded.bytes,indexed_at=unixepoch()");
    sqlite3_bind_text(statement.value, 1, photo.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement.value, 2, photo.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement.value, 3, photo.folder.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement.value, 4, photo.captured.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement.value, 5, photo.format.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement.value, 6, photo.width);
    sqlite3_bind_int(statement.value, 7, photo.height);
    sqlite3_bind_int64(statement.value, 8, photo.bytes);
    sqlite3_bind_int(statement.value, 9, photo.favorite);
    sqlite3_bind_int(statement.value, 10, photo.trashed);
    if (sqlite3_step(statement.value) != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(m_private->db));
}

std::vector<Photo> PhotoStore::photos(bool favorites, bool trash) const {
    std::vector<Photo> result;
    const char* sql = favorites
        ? "SELECT id,path,name,folder,captured,format,width,height,bytes,favorite,trashed FROM photos WHERE favorite=1 AND trashed=0 ORDER BY captured DESC,name"
        : trash
        ? "SELECT id,path,name,folder,captured,format,width,height,bytes,favorite,trashed FROM photos WHERE trashed=1 ORDER BY captured DESC,name"
        : "SELECT id,path,name,folder,captured,format,width,height,bytes,favorite,trashed FROM photos WHERE trashed=0 ORDER BY captured DESC,name";
    Statement statement(m_private->db, sql);
    while (sqlite3_step(statement.value) == SQLITE_ROW)
        result.push_back({sqlite3_column_int64(statement.value, 0), text(statement.value, 1), text(statement.value, 2),
                          text(statement.value, 3), text(statement.value, 4), text(statement.value, 5),
                          sqlite3_column_int(statement.value, 6), sqlite3_column_int(statement.value, 7),
                          sqlite3_column_int64(statement.value, 8), bool(sqlite3_column_int(statement.value, 9)),
                          bool(sqlite3_column_int(statement.value, 10))});
    return result;
}

std::vector<Photo> PhotoStore::search(const std::string& query) const {
    std::vector<Photo> result;
    Statement statement(m_private->db,
                        "SELECT id,path,name,folder,captured,format,width,height,bytes,favorite,trashed FROM photos "
                        "WHERE trashed=0 AND (name LIKE ? OR folder LIKE ? OR captured LIKE ?) ORDER BY captured DESC,name");
    const std::string pattern = "%" + query + "%";
    for (int column = 1; column <= 3; ++column)
        sqlite3_bind_text(statement.value, column, pattern.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(statement.value) == SQLITE_ROW)
        result.push_back({sqlite3_column_int64(statement.value, 0), text(statement.value, 1), text(statement.value, 2),
                          text(statement.value, 3), text(statement.value, 4), text(statement.value, 5),
                          sqlite3_column_int(statement.value, 6), sqlite3_column_int(statement.value, 7),
                          sqlite3_column_int64(statement.value, 8), bool(sqlite3_column_int(statement.value, 9)),
                          bool(sqlite3_column_int(statement.value, 10))});
    return result;
}

void PhotoStore::setFavorite(int64_t id, bool favorite) {
    Statement statement(m_private->db, "UPDATE photos SET favorite=? WHERE id=?");
    sqlite3_bind_int(statement.value, 1, favorite);
    sqlite3_bind_int64(statement.value, 2, id);
    sqlite3_step(statement.value);
}
void PhotoStore::setTrashed(int64_t id, bool trashed) {
    Statement statement(m_private->db, "UPDATE photos SET trashed=? WHERE id=?");
    sqlite3_bind_int(statement.value, 1, trashed);
    sqlite3_bind_int64(statement.value, 2, id);
    sqlite3_step(statement.value);
}
void PhotoStore::purge(int64_t id) {
    Statement statement(m_private->db, "DELETE FROM photos WHERE id=?");
    sqlite3_bind_int64(statement.value, 1, id);
    sqlite3_step(statement.value);
}
std::vector<Photo> PhotoStore::folder(const std::string& path) const {
    std::vector<Photo> result;
    Statement statement(m_private->db, "SELECT id,path,name,folder,captured,format,width,height,bytes,favorite,trashed FROM photos WHERE folder=? AND trashed=0 ORDER BY captured DESC,name");
    sqlite3_bind_text(statement.value, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(statement.value) == SQLITE_ROW)
        result.push_back({sqlite3_column_int64(statement.value, 0), text(statement.value, 1), text(statement.value, 2), text(statement.value, 3), text(statement.value, 4), text(statement.value, 5), sqlite3_column_int(statement.value, 6), sqlite3_column_int(statement.value, 7), sqlite3_column_int64(statement.value, 8), bool(sqlite3_column_int(statement.value, 9)), bool(sqlite3_column_int(statement.value, 10))});
    return result;
}
std::vector<Album> PhotoStore::albums() const {
    std::vector<Album> result; Statement statement(m_private->db, "SELECT id,name,favorite FROM albums ORDER BY name");
    while (sqlite3_step(statement.value) == SQLITE_ROW) result.push_back({sqlite3_column_int64(statement.value, 0), text(statement.value, 1), bool(sqlite3_column_int(statement.value, 2))});
    return result;
}
int64_t PhotoStore::createAlbum(const std::string& name) {
    Statement statement(m_private->db, "INSERT INTO albums(name) VALUES(?)"); sqlite3_bind_text(statement.value, 1, name.c_str(), -1, SQLITE_TRANSIENT); if (sqlite3_step(statement.value) != SQLITE_DONE) return 0; return sqlite3_last_insert_rowid(m_private->db);
}
void PhotoStore::renameAlbum(int64_t id, const std::string& name) { Statement statement(m_private->db, "UPDATE albums SET name=? WHERE id=?"); sqlite3_bind_text(statement.value, 1, name.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int64(statement.value, 2, id); sqlite3_step(statement.value); }
void PhotoStore::deleteAlbum(int64_t id) { Statement statement(m_private->db, "DELETE FROM albums WHERE id=?"); sqlite3_bind_int64(statement.value, 1, id); sqlite3_step(statement.value); }
void PhotoStore::addToAlbum(int64_t albumId, int64_t photoId) { Statement statement(m_private->db, "INSERT OR IGNORE INTO album_photos(album_id,photo_id) VALUES(?,?)"); sqlite3_bind_int64(statement.value, 1, albumId); sqlite3_bind_int64(statement.value, 2, photoId); sqlite3_step(statement.value); }
std::vector<Photo> PhotoStore::albumPhotos(int64_t id) const {
    std::vector<Photo> result; Statement statement(m_private->db, "SELECT p.id,p.path,p.name,p.folder,p.captured,p.format,p.width,p.height,p.bytes,p.favorite,p.trashed FROM photos p JOIN album_photos ap ON ap.photo_id=p.id WHERE ap.album_id=? AND p.trashed=0 ORDER BY p.captured DESC,p.name"); sqlite3_bind_int64(statement.value, 1, id);
    while (sqlite3_step(statement.value) == SQLITE_ROW) result.push_back({sqlite3_column_int64(statement.value, 0), text(statement.value, 1), text(statement.value, 2), text(statement.value, 3), text(statement.value, 4), text(statement.value, 5), sqlite3_column_int(statement.value, 6), sqlite3_column_int(statement.value, 7), sqlite3_column_int64(statement.value, 8), bool(sqlite3_column_int(statement.value, 9)), bool(sqlite3_column_int(statement.value, 10))}); return result;
}
size_t PhotoStore::count(bool trash) const {
    Statement statement(m_private->db, trash ? "SELECT count(*) FROM photos WHERE trashed=1" : "SELECT count(*) FROM photos WHERE trashed=0");
    return sqlite3_step(statement.value) == SQLITE_ROW ? size_t(sqlite3_column_int64(statement.value, 0)) : 0;
}
} // namespace Photos
