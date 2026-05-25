#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <vector>

namespace fc {

enum class PersonStatus {
    Allowed,        // в базе и допуск разрешён
    Blacklisted,    // в базе, но заблокирован
};

struct Person {
    qint64 id = -1;
    QString fullName;
    PersonStatus status = PersonStatus::Allowed;
    std::vector<float> embedding;   // 512-D ArcFace, L2-нормированный
    QByteArray photo;               // jpeg blob (опционально, для UI)
    QDateTime createdAt;
};

inline QString to_string(PersonStatus s) {
    switch (s) {
        case PersonStatus::Allowed:     return QStringLiteral("allowed");
        case PersonStatus::Blacklisted: return QStringLiteral("blacklisted");
    }
    return {};
}

inline PersonStatus person_status_from(const QString &s) {
    if (s == QLatin1String("blacklisted")) return PersonStatus::Blacklisted;
    return PersonStatus::Allowed;
}

}   // namespace fc
