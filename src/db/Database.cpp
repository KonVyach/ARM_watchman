#include "Database.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace fc {

namespace {
constexpr auto kConnectionName = "fc";
}

QSqlDatabase Database::get() {
    return QSqlDatabase::database(kConnectionName);
}

bool Database::open(const QString &path) {
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());

    auto db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        qWarning() << "Database::open failed:" << db.lastError().text();
        return false;
    }
    QSqlQuery q(db);
    q.exec("PRAGMA foreign_keys = ON");
    return createSchema();
}

bool Database::createSchema() {
    QSqlQuery q(get());
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS persons ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  full_name   TEXT NOT NULL,"
        "  status      TEXT NOT NULL DEFAULT 'allowed',"
        "  embedding   BLOB NOT NULL,"
        "  photo       BLOB,"
        "  created_at  TEXT NOT NULL"
        ")")) {
        qWarning() << "create persons:" << q.lastError().text();
        return false;
    }
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS settings ("
        "  key   TEXT PRIMARY KEY,"
        "  value TEXT NOT NULL"
        ")")) {
        qWarning() << "create settings:" << q.lastError().text();
        return false;
    }
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS events ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ts          TEXT NOT NULL,"
        "  person_id   INTEGER,"
        "  step        TEXT NOT NULL,"
        "  result      TEXT NOT NULL,"
        "  details     TEXT"
        ")")) {
        qWarning() << "create events:" << q.lastError().text();
        return false;
    }
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS person_photos ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id   INTEGER NOT NULL REFERENCES persons(id) ON DELETE CASCADE,"
        "  embedding   BLOB NOT NULL,"
        "  photo       BLOB,"
        "  created_at  TEXT NOT NULL"
        ")")) {
        qWarning() << "create person_photos:" << q.lastError().text();
        return false;
    }
    return true;
}

QString Database::readSetting(const QString &key, const QString &fallback) {
    QSqlQuery q(get());
    q.prepare("SELECT value FROM settings WHERE key = ?");
    q.addBindValue(key);
    if (q.exec() && q.next()) return q.value(0).toString();
    return fallback;
}

void Database::writeSetting(const QString &key, const QString &value) {
    QSqlQuery q(get());
    q.prepare("INSERT INTO settings(key, value) VALUES(?, ?) "
              "ON CONFLICT(key) DO UPDATE SET value = excluded.value");
    q.addBindValue(key);
    q.addBindValue(value);
    if (!q.exec()) qWarning() << "writeSetting:" << q.lastError().text();
}

}   // namespace fc
