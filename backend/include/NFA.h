#ifndef NFA_H
#define NFA_H

#include "Automaton.h"

struct NFASimulationStep {
    int stepNumber;
    char inputSymbol;
    std::set<int> currentStates;
    std::set<int> nextStatesBeforeEpsilon;
    std::set<int> nextStatesAfterEpsilon;
    bool isAccepted;
};

class NFA : public Automaton {
private:
    // transitions: (from, symbol) -> set of target states
    // symbol == EPSILON represents an epsilon transition
    std::map<std::pair<int, char>, std::set<int>> transitions;

public:
    NFA();

    void addTransition(int from, char symbol, int to);
    void addEpsilonTransition(int from, int to);

    std::set<int> getTransitions(int from, char symbol) const;
    std::set<int> getEpsilonTransitions(int from) const;
    const std::map<std::pair<int, char>, std::set<int>>& getAllTransitions() const { return transitions; }

    // TOC core algorithms
    std::set<int> epsilonClosure(int state) const;
    std::set<int> epsilonClosure(const std::set<int>& stateSet) const;
    std::set<int> move(const std::set<int>& stateSet, char symbol) const;

    // Simulation
    bool accepts(const std::string& input) const;
    std::vector<NFASimulationStep> simulateTrace(const std::string& input) const;

    nlohmann::json toJson() const override;
    nlohmann::json getTransitionTableJson() const;
};

#endif // NFA_H
