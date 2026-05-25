#pragma once

#include "facecontrol/core/Person.h"

#include <QDialog>
#include <QPixmap>
#include <QVector>
#include <vector>

class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QWidget;

namespace fc {

class FaceEngine;
class PersonRepository;

class PersonEditDialog : public QDialog {
    Q_OBJECT
public:
    // Создание нового сотрудника.
    PersonEditDialog(FaceEngine *engine, PersonRepository *repo, QWidget *parent = nullptr);
    // Редактирование существующего (загружает все его фото из БД).
    PersonEditDialog(const Person &existing, FaceEngine *engine, PersonRepository *repo,
                     QWidget *parent = nullptr);

    Person result() const { return result_; }

    // Новые фото (не первое), ещё не в БД — вызывающий код вставляет их сам.
    struct ExtraPhoto { std::vector<float> embedding; QByteArray photoBytes; };
    QVector<ExtraPhoto>    extraPhotos()     const;
    const QVector<qint64> &removedPhotoIds() const { return removedPhotoIds_; }

private:
    struct PhotoEntry {
        qint64             dbId = -1;  // -1=новое, 0=основное(persons), >0=person_photos.id
        std::vector<float> embedding;
        QByteArray         photoBytes;
        QPixmap            thumbnail;
    };

    void init();
    void addPhoto();
    void deletePhoto(int idx);
    void refreshGallery();
    void updateStatus();
    bool computeEmbedding(const QImage &img, PhotoEntry &entry);

    Person               result_;
    FaceEngine          *engine_  = nullptr;
    PersonRepository    *repo_    = nullptr;
    QVector<PhotoEntry>  photoEntries_;
    QVector<qint64>      removedPhotoIds_;

    QLineEdit   *nameEdit_      = nullptr;
    QComboBox   *statusBox_     = nullptr;
    QWidget     *galleryWidget_ = nullptr;
    QHBoxLayout *galleryLayout_ = nullptr;
    QLabel      *statusLabel_   = nullptr;
};

}   // namespace fc
