#pragma once

#include <QString>

namespace fc {

enum class CheckpointState {
    Idle,
    FaceCapture,
    FaceMatch,
    GuardVerifyIdentity,
    MetalCheck,
    GuardMetal,
    TempCheck,
    MedicTemp,
    AlcoholCheck,
    MedicAlcohol,
    Granted,
    Denied,
};

enum class DenyReason {
    None,
    NotInDatabase,
    Blacklisted,
    ProhibitedItems,
    HighTemperature,
    Intoxicated,
    OperatorRejected,
};

inline QString state_label(CheckpointState s) {
    switch (s) {
        case CheckpointState::Idle:                 return QStringLiteral("Ожидание");
        case CheckpointState::FaceCapture:          return QStringLiteral("Захват изображения");
        case CheckpointState::FaceMatch:            return QStringLiteral("Поиск в базе");
        case CheckpointState::GuardVerifyIdentity:  return QStringLiteral("Проверка охранником");
        case CheckpointState::MetalCheck:           return QStringLiteral("Металлодетектор");
        case CheckpointState::GuardMetal:           return QStringLiteral("Досмотр охранника");
        case CheckpointState::TempCheck:            return QStringLiteral("Температура");
        case CheckpointState::MedicTemp:            return QStringLiteral("Перепроверка медиком (t°)");
        case CheckpointState::AlcoholCheck:         return QStringLiteral("Опьянение");
        case CheckpointState::MedicAlcohol:         return QStringLiteral("Перепроверка медиком (алкоголь)");
        case CheckpointState::Granted:              return QStringLiteral("Допуск разрешён");
        case CheckpointState::Denied:               return QStringLiteral("Допуск запрещён");
    }
    return {};
}

inline QString deny_reason_label(DenyReason r) {
    switch (r) {
        case DenyReason::None:              return {};
        case DenyReason::NotInDatabase:     return QStringLiteral("Не найден в базе");
        case DenyReason::Blacklisted:       return QStringLiteral("В чёрном списке");
        case DenyReason::ProhibitedItems:   return QStringLiteral("Запрещённые предметы");
        case DenyReason::HighTemperature:   return QStringLiteral("Повышенная температура");
        case DenyReason::Intoxicated:       return QStringLiteral("Опьянение");
        case DenyReason::OperatorRejected:  return QStringLiteral("Решение оператора");
    }
    return {};
}

}   // namespace fc
