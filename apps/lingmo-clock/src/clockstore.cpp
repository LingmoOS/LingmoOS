// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clockstore.h"
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace Clock {
namespace {
void exec(sqlite3* db, const char* sql) {
    char* error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : "sqlite error";
        sqlite3_free(error);
        throw std::runtime_error(message);
    }
}
struct Statement {
    sqlite3_stmt* p = nullptr;
    Statement(sqlite3* db, const char* sql) {
        if (sqlite3_prepare_v2(db, sql, -1, &p, nullptr) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(db));
    }
    ~Statement() {
        sqlite3_finalize(p);
    }
};
std::string column(sqlite3_stmt* s, int i) {
    const auto* text = sqlite3_column_text(s, i);
    return text ? reinterpret_cast<const char*>(text) : "";
}
} // namespace
Store::Store(std::string path) {
    if (path.empty()) {
        const char* data = std::getenv("XDG_DATA_HOME");
        path = data ? data : "";
        if (path.empty()) {
            const char* home = std::getenv("HOME");
            path = std::string(home ? home : ".") + "/.local/share";
        }
        path += "/lingmo-clock/clock.db";
    }
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    if (sqlite3_open(path.c_str(), &m_db) != SQLITE_OK)
        throw std::runtime_error("Unable to open clock database");
    exec(m_db, "PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON;"
               "CREATE TABLE IF NOT EXISTS world_clocks(id TEXT PRIMARY KEY, city TEXT NOT NULL, zone TEXT NOT NULL);"
               "CREATE TABLE IF NOT EXISTS alarms(id INTEGER PRIMARY KEY, name TEXT NOT NULL, hour INTEGER NOT NULL, "
               "minute INTEGER NOT NULL, weekdays INTEGER NOT NULL, recurring INTEGER NOT NULL DEFAULT 1, enabled "
               "INTEGER NOT NULL, sound TEXT NOT NULL DEFAULT '', next_utc INTEGER NOT NULL DEFAULT 0);");
    try {
        exec(m_db, "ALTER TABLE alarms ADD COLUMN recurring INTEGER NOT NULL DEFAULT 1");
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()).find("duplicate column name") == std::string::npos)
            throw;
    }
}
Store::~Store() {
    if (m_db)
        sqlite3_close(m_db);
}
std::vector<WorldClock> Store::worldClocks() const {
    std::vector<WorldClock> r;
    Statement q(m_db, "SELECT id,city,zone FROM world_clocks ORDER BY rowid");
    while (sqlite3_step(q.p) == SQLITE_ROW)
        r.push_back({column(q.p, 0), column(q.p, 1), column(q.p, 2)});
    return r;
}
void Store::addWorldClock(const WorldClock& c) {
    Statement q(m_db, "INSERT OR REPLACE INTO world_clocks VALUES(?,?,?)");
    sqlite3_bind_text(q.p, 1, c.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(q.p, 2, c.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(q.p, 3, c.zone.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(q.p) != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(m_db));
}
void Store::removeWorldClock(const std::string& id) {
    Statement q(m_db, "DELETE FROM world_clocks WHERE id=?");
    sqlite3_bind_text(q.p, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(q.p);
}
std::vector<Alarm> Store::alarms() const {
    std::vector<Alarm> r;
    Statement q(
        m_db, "SELECT id,name,hour,minute,weekdays,recurring,enabled,sound,next_utc FROM alarms ORDER BY hour,minute");
    while (sqlite3_step(q.p) == SQLITE_ROW)
        r.push_back({sqlite3_column_int64(q.p, 0), column(q.p, 1), sqlite3_column_int(q.p, 2),
                     sqlite3_column_int(q.p, 3), uint8_t(sqlite3_column_int(q.p, 4)), bool(sqlite3_column_int(q.p, 5)),
                     bool(sqlite3_column_int(q.p, 6)), column(q.p, 7), sqlite3_column_int64(q.p, 8)});
    return r;
}
int64_t Store::saveAlarm(const Alarm& a) {
    Statement q(m_db,
                "INSERT INTO alarms(id,name,hour,minute,weekdays,recurring,enabled,sound,next_utc) "
                "VALUES(?,?,?,?,?,?,?,?,?) ON CONFLICT(id) DO UPDATE SET "
                "name=excluded.name,hour=excluded.hour,minute=excluded.minute,weekdays=excluded.weekdays,recurring="
                "excluded.recurring,enabled=excluded.enabled,sound=excluded.sound,next_utc=excluded.next_utc");
    if (a.id)
        sqlite3_bind_int64(q.p, 1, a.id);
    else
        sqlite3_bind_null(q.p, 1);
    sqlite3_bind_text(q.p, 2, a.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(q.p, 3, a.hour);
    sqlite3_bind_int(q.p, 4, a.minute);
    sqlite3_bind_int(q.p, 5, a.weekdays);
    sqlite3_bind_int(q.p, 6, a.recurring);
    sqlite3_bind_int(q.p, 7, a.enabled);
    sqlite3_bind_text(q.p, 8, a.sound.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(q.p, 9, a.nextOccurrenceUtc);
    if (sqlite3_step(q.p) != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(m_db));
    return a.id ? a.id : sqlite3_last_insert_rowid(m_db);
}
void Store::removeAlarm(int64_t id) {
    Statement q(m_db, "DELETE FROM alarms WHERE id=?");
    sqlite3_bind_int64(q.p, 1, id);
    sqlite3_step(q.p);
}
void Store::setAlarmEnabled(int64_t id, bool enabled) {
    Statement q(m_db, "UPDATE alarms SET enabled=? WHERE id=?");
    sqlite3_bind_int(q.p, 1, enabled);
    sqlite3_bind_int64(q.p, 2, id);
    sqlite3_step(q.p);
}
} // namespace Clock
