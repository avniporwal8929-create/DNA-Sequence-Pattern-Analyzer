#ifndef DFA_H
#define DFA_H

#include "Automaton.h"

struct DFASimulationStep {
    int stepNumber;
    int currentState;
    std::string currentStateLabel;
    std::string currentSubset;
    char inputSymbol;
    int nextState;
    std::string nextStateLabel;
    std::string nextSubset;
    std::string transitionDescription;
    bool isAccepted;
};

class DFA : public Automaton {
private:
    // transitions: (from, symbol) -> to
    std::map<std::pair<int, char>, int> transitions;

    // Subset mapping: DFA state id -> set of NFA state ids
    std::map<int, std::set<int>> stateSubsets;
    std::map<int, std::string> subsetStrings;

public:
    DFA();

    void addTransition(int from, char symbol, int to);
    int getTransition(int from, char symbol) const;
    bool hasTransition(int from, char symbol) const;

    void setStateSubset(int dfaState, const std::set<int>& nfaSubset, const std::string& subsetStr = "");
    const std::set<int>& getStateSubset(int dfaState) const;
    std::string getSubsetString(int dfaState) const;

    const std::map<std::pair<int, char>, int>& getAllTransitions() const { return transitions; }
    const std::map<int, std::set<int>>& getAllStateSubsets() const { return stateSubsets; }

    // Simulation
    bool accepts(const std::string& input) const;
    std::vector<DFASimulationStep> simulateTrace(const std::string& input) const;

    nlohmann::json toJson() const override;
    nlohmann::json getTransitionTableJson() const;
};

#endif // DFA_H
