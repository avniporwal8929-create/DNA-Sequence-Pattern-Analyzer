#ifndef SUBSET_CONSTRUCTION_H
#define SUBSET_CONSTRUCTION_H

#include "NFA.h"
#include "DFA.h"
#include "json.hpp"
#include <vector>
#include <string>
#include <set>
#include <map>

struct SymbolTransitionStep {
    char symbol;
    std::set<int> moveSet;
    std::string moveSetString;
    std::set<int> closureSet;
    std::string closureSetString;
    int targetStateId;
    std::string targetStateLabel;
    bool isNewState;
    std::string explanation;

    nlohmann::json toJson() const;
};

struct SubsetStep {
    int stepIndex;
    int dfaStateId;
    std::string dfaStateLabel;
    std::set<int> nfaSubset;
    std::string nfaSubsetString;
    bool isStart;
    bool isAccept;
    std::vector<SymbolTransitionStep> transitions;

    nlohmann::json toJson() const;
};

struct SubsetConversionReport {
    DFA dfa;
    std::vector<SubsetStep> steps;
    nlohmann::json conversionTable;
    std::string theoreticalExplanation;

    nlohmann::json toJson() const;
};

class SubsetConstruction {
public:
    static SubsetConversionReport convert(const NFA& nfa);

private:
    static std::string formatSet(const std::set<int>& s, const NFA& nfa);
    static std::string generateDfaLabel(int index);
};

#endif // SUBSET_CONSTRUCTION_H
