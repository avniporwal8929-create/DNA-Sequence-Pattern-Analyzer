#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "json.hpp"

// Constant for epsilon symbol
constexpr char EPSILON = '\0';
const std::string EPSILON_STR = "ε";

struct VisualNode {
    int id;
    std::string label;
    bool isStart;
    bool isAccept;
    std::string subsetLabel; // For DFA states representing NFA subsets, e.g. "{q0, q1}"
};

struct VisualEdge {
    int from;
    int to;
    std::string symbol;
    bool isEpsilon;
};

class Automaton {
protected:
    std::set<int> states;
    std::set<char> alphabet;
    int startState;
    std::set<int> acceptStates;
    std::map<int, std::string> stateLabels;

public:
    Automaton() : startState(0), alphabet({'A', 'T', 'G', 'C'}) {}
    virtual ~Automaton() = default;

    const std::set<int>& getStates() const { return states; }
    const std::set<char>& getAlphabet() const { return alphabet; }
    int getStartState() const { return startState; }
    const std::set<int>& getAcceptStates() const { return acceptStates; }

    void setStartState(int s) {
        startState = s;
        states.insert(s);
    }

    void addState(int s, const std::string& label = "") {
        states.insert(s);
        if (!label.empty()) {
            stateLabels[s] = label;
        }
    }

    void addAcceptState(int s) {
        states.insert(s);
        acceptStates.insert(s);
    }

    bool isAcceptState(int s) const {
        return acceptStates.find(s) != acceptStates.end();
    }

    std::string getStateLabel(int s) const {
        auto it = stateLabels.find(s);
        if (it != stateLabels.end()) return it->second;
        return "q" + std::to_string(s);
    }

    void setStateLabel(int s, const std::string& label) {
        stateLabels[s] = label;
    }

    virtual nlohmann::json toJson() const = 0;
};

#endif // AUTOMATON_H
