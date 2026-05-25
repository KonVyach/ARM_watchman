#pragma once

#include <QDialog>

class QLineEdit;

namespace fc {

class AdminLoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit AdminLoginDialog(QWidget *parent = nullptr);

private:
    QLineEdit *pwEdit_ = nullptr;

    static bool checkPassword(const QString &pw);
};

}   // namespace fc
