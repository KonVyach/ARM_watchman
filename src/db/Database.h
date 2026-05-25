#pragma once

#include <QSqlDatabase>
#include <QString>

namespace fc {

class Database {
public:
    // Открывает БД по указанному пути. Создаёт схему при отсутствии.
    static bool open(const QString &path);

    // Получить дескриптор (Qt SQL-соединение с именем "fc").
    static QSqlDatabase get();

    // Чтение/запись ключ-значение в `settings`.
    static QString readSetting(const QString &key, const QString &fallback = {});
    static void writeSetting(const QString &key, const QString &value);

private:
    static bool createSchema();
};

}   // namespace fc
