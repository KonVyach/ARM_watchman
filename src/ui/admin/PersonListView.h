#pragma once

#include "facecontrol/core/Person.h"

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QPushButton;

namespace fc {

class PersonRepository;
class FaceEngine;

class PersonListView : public QWidget {
    Q_OBJECT
public:
    PersonListView(PersonRepository *repo, FaceEngine *engine, QWidget *parent = nullptr);

private slots:
    void reload();
    void addPerson();
    void editSelected();
    void removeSelected();

private:
    void fillRow(int row, const Person &p);

    PersonRepository  *repo_;
    FaceEngine        *engine_;
    QVector<Person>    persons_;

    QLineEdit         *searchEdit_ = nullptr;
    QTableWidget      *table_      = nullptr;
    QPushButton       *addBtn_     = nullptr;
    QPushButton       *editBtn_    = nullptr;
    QPushButton       *removeBtn_  = nullptr;
};

}   // namespace fc
