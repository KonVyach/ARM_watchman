#include "FaceMatcher.h"
#include "ThresholdPolicy.h"
#include "db/PersonRepository.h"

#include <QHash>
#include <algorithm>

namespace fc {

FaceMatcher::FaceMatcher(PersonRepository *repo, ThresholdPolicy *policy, QObject *parent)
    : QObject(parent), repo_(repo), policy_(policy) {}

double FaceMatcher::cosine(const std::vector<float> &a, const std::vector<float> &b) {
    if (a.size() != b.size() || a.empty()) return -1.0;
    double dot = 0.0;
    for (size_t i = 0; i < a.size(); ++i) dot += static_cast<double>(a[i]) * b[i];
    return dot;
}

// Наполняет bestSim и personMap данными из обеих таблиц.
static void buildMaps(PersonRepository *repo,
                      const std::vector<float> &embedding,
                      QHash<qint64, double>  &bestSim,
                      QHash<qint64, Person>  &personMap) {
    // Основной эмбеддинг из таблицы persons.
    for (const auto &p : repo->loadAllWithEmbeddings()) {
        if (p.embedding.empty()) continue;
        personMap[p.id] = p;
        bestSim[p.id]   = FaceMatcher::cosine(embedding, p.embedding);
    }
    // Дополнительные эмбеддинги из person_photos.
    for (const auto &ph : repo->loadAllPhotos()) {
        if (ph.embedding.empty()) continue;
        double s = FaceMatcher::cosine(embedding, ph.embedding);
        if (!bestSim.contains(ph.personId) || s > bestSim[ph.personId])
            bestSim[ph.personId] = s;
    }
}

MatchResult FaceMatcher::findBest(const std::vector<float> &embedding) const {
    QHash<qint64, double> bestSim;
    QHash<qint64, Person> personMap;
    buildMaps(repo_, embedding, bestSim, personMap);

    MatchResult best;
    best.similarity = -1.0;
    for (auto it = bestSim.constBegin(); it != bestSim.constEnd(); ++it) {
        if (it.value() > best.similarity && personMap.contains(it.key())) {
            best.similarity = it.value();
            best.person     = personMap[it.key()];
        }
    }

    if (best.person && best.similarity < policy_->current())
        best.person.reset();

    return best;
}

QVector<MatchResult> FaceMatcher::topK(const std::vector<float> &embedding, int k) const {
    QHash<qint64, double> bestSim;
    QHash<qint64, Person> personMap;
    buildMaps(repo_, embedding, bestSim, personMap);

    QVector<MatchResult> out;
    for (auto it = bestSim.constBegin(); it != bestSim.constEnd(); ++it) {
        if (!personMap.contains(it.key())) continue;
        MatchResult r;
        r.person     = personMap[it.key()];
        r.similarity = it.value();
        out.push_back(r);
    }

    std::sort(out.begin(), out.end(),
              [](const MatchResult &a, const MatchResult &b) {
                  return a.similarity > b.similarity;
              });
    if (out.size() > k) out.resize(k);
    return out;
}

}   // namespace fc
