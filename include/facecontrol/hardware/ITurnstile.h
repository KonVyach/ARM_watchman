#pragma once

#include <QObject>

namespace fc {

enum class TurnstileState {
    Closed,
    Opened,
};

class ITurnstile : public QObject {
    Q_OBJECT
public:
    explicit ITurnstile(QObject *parent = nullptr) : QObject(parent) {}
    ~ITurnstile() override;

    virtual TurnstileState state() const = 0;

    virtual void open() = 0;
    virtual void close() = 0;

    // Внешний триггер, что человек прошёл (в моке — кнопка в UI).
    virtual void notifyPassed() = 0;

signals:
    void stateChanged(TurnstileState state);
    void personPassed();
};

}   // namespace fc
