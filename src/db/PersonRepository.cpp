#include "PersonRepository.h"
#include "Database.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include <cstring>

namespace fc {

PersonRepository::PersonRepository(QObject *parent) : QObject(parent) {}

QByteArray PersonRepository::encodeEmbedding(const std::vector<float> &v) {
    QByteArray b;
    b.resize(static_cast<int>(v.size() * sizeof(float)));
    if (!v.empty()) std::memcpy(b.data(), v.data(), b.size());
    return b;
}

std::vector<float> PersonRepository::decodeEmbedding(const QByteArray &b) {
    std::vector<float> v(b.size() / sizeof(float));
    if (!v.empty()) std::memcpy(v.data(), b.constData(), v.size() * sizeof(float));
    return v;
}

Person PersonRepository::rowToPerson(const QSqlQuery &q) {
    Person p;
    p.id        = q.value("id").toLongLong();
    p.fullName  = q.value("full_name").toString();
    p.status    = person_status_from(q.value("status").toString());
    p.embedding = decodeEmbedding(q.value("embedding").toByteArray());
    p.photo     = q.value("photo").toByteArray();
    p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    return p;
}

std::optional<qint64> PersonRepository::insert(const Person &p) {
    QSqlQuery q(Database::get());
    q.prepare("INSERT INTO persons(full_name, status, embedding, photo, created_at) "
              "VALUES(?, ?, ?, ?, ?)");
    q.addBindValue(p.fullName);
    q.addBindValue(to_string(p.status));
    q.addBindValue(encodeEmbedding(p.embedding));
    q.addBindValue(p.photo);
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!q.exec()) {
        qWarning() << "PersonRepository::insert:" << q.lastError().text();
        return std::nullopt;
    }
    emit const_cast<PersonRepository*>(this)->changed();
    return q.lastInsertId().toLongLong();
}

bool PersonRepository::update(const Person &p) {
    QSqlQuery q(Database::get());
    q.prepare("UPDATE persons SET full_name = ?, status = ?, embedding = ?, photo = ? WHERE id = ?");
    q.addBindValue(p.fullName);
    q.addBindValue(to_string(p.status));
    q.addBindValue(encodeEmbedding(p.embedding));
    q.addBindValue(p.photo);
    q.addBindValue(p.id);
    bool ok = q.exec();
    if (ok) emit changed();
    else qWarning() << "PersonRepository::update:" << q.lastError().text();
    return ok;
}

bool PersonRepository::remove(qint64 id) {
    QSqlQuery q(Database::get());
    q.prepare("DELETE FROM persons WHERE id = ?");
    q.addBindValue(id);
    bool ok = q.exec();
    if (ok) emit changed();
    return ok;
}

std::optional<Person> PersonRepository::findById(qint64 id) const {
    QSqlQuery q(Database::get());
    q.prepare("SELECT * FROM persons WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) return rowToPerson(q);
    return std::nullopt;
}

QVector<Person> PersonRepository::listAll() const {
    QVector<Person> out;
    QSqlQuery q(Database::get());
    if (q.exec("SELECT * FROM persons ORDER BY full_name")) {
        while (q.next()) out.push_back(rowToPerson(q));
    }
    return out;
}

QVector<Person> PersonRepository::search(const QString &substring) const {
    QVector<Person> out;
    QSqlQuery q(Database::get());
    q.prepare("SELECT * FROM persons WHERE full_name LIKE ? ORDER BY full_name");
    q.addBindValue("%" + substring + "%");
    if (q.exec()) {
        while (q.next()) out.push_back(rowToPerson(q));
    }
    return out;
}

QVector<Person> PersonRepository::loadAllWithEmbeddings() const {
    return listAll();
}

// ---------------------------------------------------------------------------
// person_photos

PersonPhoto PersonRepository::rowToPhoto(const QSqlQuery &q) {
    PersonPhoto p;
    p.id        = q.value("id").toLongLong();
    p.personId  = q.value("person_id").toLongLong();
    p.embedding = decodeEmbedding(q.value("embedding").toByteArray());
    p.photo     = q.value("photo").toByteArray();
    p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    return p;
}

bool PersonRepository::addPhoto(qint64 personId,
                                const std::vector<float> &emb,
                                const QByteArray &photo) {
    QSqlQuery q(Database::get());
    q.prepare("INSERT INTO person_photos(person_id, embedding, photo, created_at) "
              "VALUES(?, ?, ?, ?)");
    q.addBindValue(personId);
    q.addBindValue(encodeEmbedding(emb));
    q.addBindValue(photo.isEmpty() ? QVariant() : QVariant(photo));
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!q.exec()) {
        qWarning() << "PersonRepository::addPhoto:" << q.lastError().text();
        return false;
    }
    return true;
}

bool PersonRepository::removePhoto(qint64 photoId) {
    QSqlQuery q(Database::get());
    q.prepare("DELETE FROM person_photos WHERE id = ?");
    q.addBindValue(photoId);
    return q.exec();
}

QVector<PersonPhoto> PersonRepository::listPhotos(qint64 personId) const {
    QVector<PersonPhoto> out;
    QSqlQuery q(Database::get());
    q.prepare("SELECT * FROM person_photos WHERE person_id = ? ORDER BY created_at");
    q.addBindValue(personId);
    if (q.exec())
        while (q.next()) out.push_back(rowToPhoto(q));
    return out;
}

QVector<PersonPhoto> PersonRepository::loadAllPhotos() const {
    QVector<PersonPhoto> out;
    QSqlQuery q(Database::get());
    if (q.exec("SELECT * FROM person_photos"))
        while (q.next()) out.push_back(rowToPhoto(q));
    return out;
}

}   // namespace fc
