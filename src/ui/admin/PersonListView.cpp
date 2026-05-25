#include "PersonListView.h"
#include "PersonEditDialog.h"
#include "db/PersonRepository.h"
#include "face/FaceEngine.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace fc {

PersonListView::PersonListView(PersonRepository *repo, FaceEngine *engine, QWidget *parent)
    : QWidget(parent), repo_(repo), engine_(engine) {

    auto *lay = new QVBoxLayout(this);

    // Поиск.
    auto *searchRow = new QHBoxLayout;
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText(tr("Поиск по ФИО…"));
    searchRow->addWidget(new QLabel(tr("Поиск:"), this));
    searchRow->addWidget(searchEdit_);
    lay->addLayout(searchRow);

    // Таблица.
    table_ = new QTableWidget(0, 4, this);
    table_->setHorizontalHeaderLabels({tr("ID"), tr("ФИО"), tr("Статус"), tr("Добавлен")});
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSortingEnabled(true);
    lay->addWidget(table_);

    // Кнопки.
    auto *btnsRow = new QHBoxLayout;
    addBtn_    = new QPushButton(tr("+ Добавить"), this);
    editBtn_   = new QPushButton(tr("✎ Изменить"), this);
    removeBtn_ = new QPushButton(tr("✗ Удалить"), this);
    addBtn_->setStyleSheet("background:#1a4a7a;color:white;padding:6px 14px;border-radius:4px");
    editBtn_->setStyleSheet("background:#4a4a1a;color:white;padding:6px 14px;border-radius:4px");
    removeBtn_->setStyleSheet("background:#7a1a1a;color:white;padding:6px 14px;border-radius:4px");
    btnsRow->addStretch();
    btnsRow->addWidget(addBtn_);
    btnsRow->addWidget(editBtn_);
    btnsRow->addWidget(removeBtn_);
    lay->addLayout(btnsRow);

    connect(searchEdit_, &QLineEdit::textChanged, this, &PersonListView::reload);
    connect(addBtn_,    &QPushButton::clicked, this, &PersonListView::addPerson);
    connect(editBtn_,   &QPushButton::clicked, this, &PersonListView::editSelected);
    connect(removeBtn_, &QPushButton::clicked, this, &PersonListView::removeSelected);
    connect(repo_, &PersonRepository::changed, this, &PersonListView::reload);
    connect(table_, &QTableWidget::doubleClicked, this, &PersonListView::editSelected);

    reload();
}

void PersonListView::reload() {
    const QString query = searchEdit_->text().trimmed();
    persons_ = query.isEmpty() ? repo_->listAll() : repo_->search(query);

    // Отключаем сортировку на время заполнения — иначе строки
    // перемешиваются прямо во время вставки и данные попадают не в те ячейки.
    table_->setSortingEnabled(false);
    table_->setRowCount(0);
    table_->setRowCount(persons_.size());
    for (int i = 0; i < persons_.size(); ++i) fillRow(i, persons_[i]);
    table_->setSortingEnabled(true);
}

void PersonListView::fillRow(int row, const Person &p) {
    auto item = [](const QString &s) {
        auto *it = new QTableWidgetItem(s);
        it->setTextAlignment(Qt::AlignCenter);
        return it;
    };
    table_->setItem(row, 0, item(QString::number(p.id)));
    table_->setItem(row, 1, new QTableWidgetItem(p.fullName));
    table_->setItem(row, 2, item(p.status == PersonStatus::Blacklisted
                                    ? tr("Чёрный список") : tr("Разрешён")));
    table_->setItem(row, 3, item(p.createdAt.toString("dd.MM.yyyy HH:mm")));

    if (p.status == PersonStatus::Blacklisted) {
        for (int c = 0; c < 4; ++c) {
            if (auto *it = table_->item(row, c)) {
                it->setForeground(QColor(200, 80, 80));
            }
        }
    }
}

void PersonListView::addPerson() {
    PersonEditDialog dlg(engine_, repo_, this);
    if (dlg.exec() == QDialog::Accepted) {
        auto id = repo_->insert(dlg.result());
        if (id) {
            for (const auto &ep : dlg.extraPhotos())
                repo_->addPhoto(*id, ep.embedding, ep.photoBytes);
        }
    }
}

// Получить ID выбранной строки прямо из таблицы (не из persons_[row],
// который может не совпасть после пользовательской сортировки).
static qint64 selectedId(QTableWidget *table) {
    const int row = table->currentRow();
    if (row < 0) return -1;
    auto *item = table->item(row, 0);
    if (!item) return -1;
    return item->text().toLongLong();
}

void PersonListView::editSelected() {
    const qint64 id = selectedId(table_);
    if (id < 0) return;

    auto opt = repo_->findById(id);
    if (!opt) return;

    PersonEditDialog dlg(*opt, engine_, repo_, this);
    if (dlg.exec() == QDialog::Accepted) {
        Person updated = dlg.result();
        updated.id        = opt->id;
        updated.createdAt = opt->createdAt;
        repo_->update(updated);

        for (qint64 pid : dlg.removedPhotoIds())
            repo_->removePhoto(pid);
        for (const auto &ep : dlg.extraPhotos())
            repo_->addPhoto(updated.id, ep.embedding, ep.photoBytes);
    }
}

void PersonListView::removeSelected() {
    const qint64 id = selectedId(table_);
    if (id < 0) return;

    auto opt = repo_->findById(id);
    if (!opt) return;

    const auto ans = QMessageBox::question(
        this, tr("Удалить"),
        tr("Удалить запись «%1»?").arg(opt->fullName));
    if (ans == QMessageBox::Yes) repo_->remove(id);
}

}   // namespace fc
