#include "EventLog.h"
#include "Database.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

namespace fc {

void EventLog::log(const QString &step,
                   const QString &result,
                   qint64 personId,
                   const QString &details) {
    QSqlQuery q(Database::get());
    q.prepare("INSERT INTO events(ts, person_id, step, result, details) "
              "VALUES(?, ?, ?, ?, ?)");
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    q.addBindValue(personId >= 0 ? QVariant(personId) : QVariant());
    q.addBindValue(step);
    q.addBindValue(result);
    q.addBindValue(details);
    if (!q.exec()) qWarning() << "EventLog::log:" << q.lastError().text();
}

}   // namespace fc
