#pragma once

#include "facecontrol/core/Person.h"

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QSqlQuery>
#include <QVector>
#include <optional>
#include <vector>

namespace fc {

// Одна дополнительная фотография сотрудника (таблица person_photos).
struct PersonPhoto {
    qint64             id        = -1;
    qint64             personId  = -1;
    std::vector<float> embedding;
    QByteArray         photo;
    QDateTime          createdAt;
};

class PersonRepository : public QObject {
    Q_OBJECT
public:
    explicit PersonRepository(QObject *parent = nullptr);

    // CRUD persons.
    std::optional<qint64> insert(const Person &p);
    bool update(const Person &p);
    bool remove(qint64 id);

    std::optional<Person> findById(qint64 id) const;
    QVector<Person> listAll() const;
    QVector<Person> search(const QString &substring) const;
    QVector<Person> loadAllWithEmbeddings() const;

    // Дополнительные фотографии / эмбеддинги для одного сотрудника.
    bool addPhoto(qint64 personId, const std::vector<float> &emb, const QByteArray &photo);
    bool removePhoto(qint64 photoId);
    QVector<PersonPhoto> listPhotos(qint64 personId) const;
    QVector<PersonPhoto> loadAllPhotos() const;   // для матчера

signals:
    void changed();

private:
    static QByteArray encodeEmbedding(const std::vector<float> &v);
    static std::vector<float> decodeEmbedding(const QByteArray &b);
    static Person       rowToPerson(const QSqlQuery &q);
    static PersonPhoto  rowToPhoto(const QSqlQuery &q);
};

}   // namespace fc
