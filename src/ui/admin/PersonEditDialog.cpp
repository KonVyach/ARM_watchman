#include "PersonEditDialog.h"
#include "db/PersonRepository.h"
#include "face/FaceEngine.h"

#include <QApplication>
#include <QBuffer>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace fc {

PersonEditDialog::PersonEditDialog(FaceEngine *engine, PersonRepository *repo, QWidget *parent)
    : QDialog(parent), engine_(engine), repo_(repo)
{
    init();
}

PersonEditDialog::PersonEditDialog(const Person &existing, FaceEngine *engine,
                                   PersonRepository *repo, QWidget *parent)
    : QDialog(parent), result_(existing), engine_(engine), repo_(repo)
{
    init();
    nameEdit_->setText(existing.fullName);
    statusBox_->setCurrentIndex(existing.status == PersonStatus::Blacklisted ? 1 : 0);

    // Основное фото из таблицы persons.
    if (!existing.photo.isEmpty()) {
        PhotoEntry primary;
        primary.dbId      = 0;          // sentinel: основное фото
        primary.embedding = existing.embedding;
        primary.photoBytes = existing.photo;
        primary.thumbnail.loadFromData(existing.photo);
        photoEntries_.push_back(primary);
    }

    // Дополнительные фото из person_photos.
    if (repo_) {
        for (const auto &ep : repo_->listPhotos(existing.id)) {
            PhotoEntry entry;
            entry.dbId      = ep.id;
            entry.embedding = ep.embedding;
            entry.photoBytes = ep.photo;
            entry.thumbnail.loadFromData(ep.photo);
            photoEntries_.push_back(entry);
        }
    }

    refreshGallery();
    updateStatus();
}

// ---------------------------------------------------------------------------

void PersonEditDialog::init() {
    setWindowTitle(tr("Сотрудник"));
    setMinimumWidth(500);

    auto *lay  = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    nameEdit_ = new QLineEdit(this);
    form->addRow(tr("ФИО:"), nameEdit_);

    statusBox_ = new QComboBox(this);
    statusBox_->addItem(tr("Разрешён"),      "allowed");
    statusBox_->addItem(tr("Чёрный список"), "blacklisted");
    form->addRow(tr("Статус:"), statusBox_);
    lay->addLayout(form);

    // Галерея фотографий.
    auto *galleryHdr = new QHBoxLayout;
    galleryHdr->addWidget(new QLabel(
        tr("Фотографии (больше ракурсов → точнее распознавание):"), this));
    auto *addBtn = new QPushButton(tr("+ Добавить фото"), this);
    addBtn->setStyleSheet("background:#1a4a7a;color:white;padding:5px 12px;border-radius:4px");
    galleryHdr->addWidget(addBtn);
    lay->addLayout(galleryHdr);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setFixedHeight(128);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background:#1a1a1a;border:1px solid #444");

    galleryWidget_ = new QWidget;
    galleryWidget_->setStyleSheet("background:#1a1a1a");
    galleryLayout_ = new QHBoxLayout(galleryWidget_);
    galleryLayout_->setContentsMargins(8, 8, 8, 8);
    galleryLayout_->setSpacing(8);
    galleryLayout_->setAlignment(Qt::AlignLeft);
    scrollArea->setWidget(galleryWidget_);
    lay->addWidget(scrollArea);

    statusLabel_ = new QLabel(tr("Нет фото"), this);
    statusLabel_->setStyleSheet("color:#f09050;font-size:12px");
    lay->addWidget(statusLabel_);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    lay->addWidget(btns);

    connect(addBtn, &QPushButton::clicked, this, &PersonEditDialog::addPhoto);

    connect(btns, &QDialogButtonBox::accepted, this, [this] {
        if (nameEdit_->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Укажите ФИО."));
            return;
        }
        if (photoEntries_.isEmpty()) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Добавьте хотя бы одно фото."));
            return;
        }
        result_.fullName = nameEdit_->text().trimmed();
        result_.status   = (statusBox_->currentData().toString() == "blacklisted")
                           ? PersonStatus::Blacklisted : PersonStatus::Allowed;
        result_.embedding  = photoEntries_[0].embedding;
        result_.photo      = photoEntries_[0].photoBytes;
        accept();
    });
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// ---------------------------------------------------------------------------

void PersonEditDialog::addPhoto() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Выберите фото"), {},
        tr("Изображения (*.jpg *.jpeg *.png *.bmp)"));
    if (path.isEmpty()) return;

    QImage img(path);
    if (img.isNull()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось загрузить изображение."));
        return;
    }

    statusLabel_->setText(tr("Вычисляю эмбеддинг…"));
    statusLabel_->setStyleSheet("color:#8af;font-size:12px");
    QApplication::processEvents();   // чтобы надпись обновилась до тяжёлого вычисления

    PhotoEntry entry;
    entry.dbId     = -1;
    entry.thumbnail = QPixmap::fromImage(img);

    if (!computeEmbedding(img, entry)) {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Лицо не обнаружено на фото — попробуйте другое изображение."));
        updateStatus();
        return;
    }

    photoEntries_.push_back(entry);
    refreshGallery();
    updateStatus();
}

void PersonEditDialog::deletePhoto(int idx) {
    if (idx <= 0 || idx >= photoEntries_.size()) return;   // первое фото неудалимо

    const qint64 dbId = photoEntries_[idx].dbId;
    if (dbId > 0) removedPhotoIds_.push_back(dbId);

    photoEntries_.removeAt(idx);
    refreshGallery();
    updateStatus();
}

void PersonEditDialog::refreshGallery() {
    // Удаляем все виджеты из галереи.
    while (galleryLayout_->count() > 0) {
        auto *item = galleryLayout_->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    for (int i = 0; i < photoEntries_.size(); ++i) {
        const auto &entry = photoEntries_[i];

        auto *frame = new QFrame(galleryWidget_);
        frame->setStyleSheet(
            "QFrame{background:#2a2a2a;border-radius:6px;border:1px solid #555}");
        auto *fl = new QVBoxLayout(frame);
        fl->setContentsMargins(4, 4, 4, 4);
        fl->setSpacing(3);

        auto *lbl = new QLabel(frame);
        lbl->setFixedSize(80, 80);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("background:#111;border-radius:4px;border:none");
        if (!entry.thumbnail.isNull())
            lbl->setPixmap(entry.thumbnail.scaled(
                80, 80, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)
                .copy(0, 0, 80, 80));
        fl->addWidget(lbl);

        if (i == 0) {
            // Основное фото — не удаляется.
            auto *tag = new QLabel(tr("основное"), frame);
            tag->setAlignment(Qt::AlignCenter);
            tag->setStyleSheet("color:#666;font-size:9px;border:none");
            fl->addWidget(tag);
        } else {
            auto *delBtn = new QPushButton("✕", frame);
            delBtn->setFixedHeight(18);
            delBtn->setStyleSheet(
                "background:#7a1a1a;color:white;border-radius:3px;"
                "font-size:10px;padding:0;border:none");
            fl->addWidget(delBtn);
            connect(delBtn, &QPushButton::clicked, this, [this, i] { deletePhoto(i); });
        }

        galleryLayout_->addWidget(frame);
    }
}

void PersonEditDialog::updateStatus() {
    const int n = photoEntries_.size();
    if (n == 0) {
        statusLabel_->setText(tr("Нет фото"));
        statusLabel_->setStyleSheet("color:#f09050;font-size:12px");
    } else {
        const int dim = photoEntries_[0].embedding.empty()
                        ? 0 : static_cast<int>(photoEntries_[0].embedding.size());
        statusLabel_->setText(tr("✓  %1 фото  |  эмбеддинг %2-D").arg(n).arg(dim));
        statusLabel_->setStyleSheet("color:#80f080;font-size:12px");
    }
}

bool PersonEditDialog::computeEmbedding(const QImage &img, PhotoEntry &entry) {
    QByteArray buf;
    QBuffer qbuf(&buf);
    qbuf.open(QIODevice::WriteOnly);
    img.save(&qbuf, "JPEG", 90);
    entry.photoBytes = buf;

    if (!engine_ || !engine_->isReady()) {
        entry.embedding.assign(512, 0.0f);
        return true;
    }
    try {
        auto emb = engine_->embedLargestFace(img);
        if (emb.empty()) return false;
        entry.embedding = emb;
        return true;
    } catch (...) { return false; }
}

QVector<PersonEditDialog::ExtraPhoto> PersonEditDialog::extraPhotos() const {
    QVector<ExtraPhoto> out;
    for (int i = 1; i < photoEntries_.size(); ++i) {
        const auto &e = photoEntries_[i];
        if (e.dbId == -1)   // только новые, ещё не в БД
            out.push_back({e.embedding, e.photoBytes});
    }
    return out;
}

}   // namespace fc
