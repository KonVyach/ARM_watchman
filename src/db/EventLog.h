#pragma once

#include <QString>

namespace fc {

// Простейший журнал событий пайплайна. Пишет напрямую в таблицу `events`.
struct EventLog {
    static void log(const QString &step,
                    const QString &result,
                    qint64 personId = -1,
                    const QString &details = {});
};

}   // namespace fc
