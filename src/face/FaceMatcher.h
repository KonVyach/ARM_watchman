#pragma once

#include "facecontrol/core/Person.h"

#include <QObject>
#include <QVector>
#include <optional>
#include <vector>

namespace fc {

class PersonRepository;
class ThresholdPolicy;

struct MatchResult {
    std::optional<Person> person;
    double similarity = 0.0;     // cos(angle) ∈ [-1, 1]
};

// Линейный поиск ближайшего соседа по косинусному сходству. Для базы в
// тысячи лиц этого более чем достаточно; ANN-индекс — задел на будущее.
class FaceMatcher : public QObject {
    Q_OBJECT
public:
    FaceMatcher(PersonRepository *repo, ThresholdPolicy *policy, QObject *parent = nullptr);

    // Найти ближайшее лицо. Возвращает кандидата, если cos_sim >= порога.
    // В любом случае similarity заполняется (для UI / диагностики).
    MatchResult findBest(const std::vector<float> &embedding) const;

    // Тот же поиск, но возвращает топ-K кандидатов независимо от порога —
    // используется в диалоге охранника, чтобы он мог выбрать кого-то из базы.
    QVector<MatchResult> topK(const std::vector<float> &embedding, int k) const;

public:
    static double cosine(const std::vector<float> &a, const std::vector<float> &b);
private:

    PersonRepository *repo_;
    ThresholdPolicy  *policy_;
};

}   // namespace fc
