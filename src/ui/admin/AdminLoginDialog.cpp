#include "AdminLoginDialog.h"
#include "db/Database.h"

#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace fc {

namespace {
// SHA-256 hash of the default password "admin". Replace via DB settings.
constexpr auto kDefaultHash =
    "8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918";
}

AdminLoginDialog::AdminLoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Вход в режим администратора"));
    setWindowModality(Qt::ApplicationModal);
    setFixedWidth(320);

    auto *lay = new QVBoxLayout(this);
    auto *info = new QLabel(tr("Введите пароль администратора:"), this);
    lay->addWidget(info);

    auto *form = new QFormLayout;
    pwEdit_ = new QLineEdit(this);
    pwEdit_->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Пароль:"), pwEdit_);
    lay->addLayout(form);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    lay->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, [this] {
        if (checkPassword(pwEdit_->text())) {
            accept();
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Неверный пароль."));
            pwEdit_->clear();
        }
    });
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(pwEdit_, &QLineEdit::returnPressed,
            btns->button(QDialogButtonBox::Ok), &QPushButton::click);
}

bool AdminLoginDialog::checkPassword(const QString &pw) {
    const QByteArray hash = QCryptographicHash::hash(
        pw.toUtf8(), QCryptographicHash::Sha256).toHex();
    const QString stored = Database::readSetting("admin.pw_hash",
                                                  QLatin1String(kDefaultHash));
    return hash == stored.toLatin1();
}

}   // namespace fc
