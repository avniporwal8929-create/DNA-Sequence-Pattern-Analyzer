#include "NFA.h"
#include <queue>
#include <sstream>

NFA::NFA() : Automaton() {
    alphabet = {'A', 'T', 'G', 'C'};
}

void NFA::addTransition(int from, char symbol, int to) {
    states.insert(from);
    states.insert(to);
    if (symbol != EPSILON) {
        alphabet.insert(symbol);
    }
    transitions[{from, symbol}].insert(to);
}

void NFA::addEpsilonTransition(int from, int to) {
    addTransition(from, EPSILON, to);
}

std::set<int> NFA::getTransitions(int from, char symbol) const {
    auto it = transitions.find({from, symbol});
    if (it != transitions.end()) {
        return it->second;
    }
    return {};
}

std::set<int> NFA::getEpsilonTransitions(int from) const {
    return getTransitions(from, EPSILON);
}

// Epsilon closure of a single state: set of states reachable via 0 or more epsilon transitions
std::set<int> NFA::epsilonClosure(int state) const {
    std::set<int> closure;
    std::queue<int> q;

    closure.insert(state);
    q.push(state);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        for (int next : getEpsilonTransitions(curr)) {
            if (closure.find(next) == closure.end()) {
                closure.insert(next);
                q.push(next);
            }
        }
    }

    return closure;
}

// Epsilon closure of a set of states
std::set<int> NFA::epsilonClosure(const std::set<int>& stateSet) const {
    std::set<int> totalClosure;
    for (int s : stateSet) {
        std::set<int> single = epsilonClosure(s);
        totalClosure.insert(single.begin(), single.end());
    }
    return totalClosure;
}

// Move(T, a) = union of transitions from each state in T on input symbol a
std::set<int> NFA::move(const std::set<int>& stateSet, char symbol) const {
    std::set<int> result;
    for (int s : stateSet) {
        std::set<int> nextStates = getTransitions(s, symbol);
        result.insert(nextStates.begin(), nextStates.end());
    }
    return result;
}

bool NFA::accepts(const std::string& input) const {
    std::set<int> current = epsilonClosure(startState);

    for (char c : input) {
        std::set<int> moved = move(current, c);
        current = epsilonClosure(moved);
        if (current.empty()) {
            return false;
        }
    }

    for (int s : current) {
        if (isAcceptState(s)) {
            return true;
        }
    }
    return false;
}

std::vector<NFASimulationStep> NFA::simulateTrace(const std::string& input) const {
    std::vector<NFASimulationStep> trace;
    std::set<int> current = epsilonClosure(startState);

    NFASimulationStep initStep;
    initStep.stepNumber = 0;
    initStep.inputSymbol = '\0';
    initStep.currentStates = current;
    initStep.nextStatesBeforeEpsilon = current;
    initStep.nextStatesAfterEpsilon = current;
    initStep.isAccepted = false;
    for (int s : current) {
        if (isAcceptState(s)) {
            initStep.isAccepted = true;
            break;
        }
    }
    trace.push_back(initStep);

    int stepCount = 1;
    for (char c : input) {
        std::set<int> moved = move(current, c);
        std::set<int> nextClosure = epsilonClosure(moved);

        NFASimulationStep step;
        step.stepNumber = stepCount++;
        step.inputSymbol = c;
        step.currentStates = current;
        step.nextStatesBeforeEpsilon = moved;
        step.nextStatesAfterEpsilon = nextClosure;
        step.isAccepted = false;
        for (int s : nextClosure) {
            if (isAcceptState(s)) {
                step.isAccepted = true;
                break;
            }
        }
        trace.push_back(step);
        current = nextClosure;
    }

    return trace;
}

nlohmann::json NFA::getTransitionTableJson() const {
    nlohmann::json rows = nlohmann::json::array();

    for (int state : states) {
        nlohmann::json row;
        row["state"] = state;
        row["stateLabel"] = getStateLabel(state);
        row["isStart"] = (state == startState);
        row["isAccept"] = isAcceptState(state);

        nlohmann::json transMap;
        for (char sym : {'A', 'T', 'G', 'C'}) {
            std::string key(1, sym);
            std::set<int> targets = getTransitions(state, sym);
            std::vector<std::string> targetLabels;
            for (int t : targets) {
                targetLabels.push_back(getStateLabel(t));
            }
            transMap[key] = targetLabels;
        }

        // Epsilon transitions
        std::set<int> epsTargets = getEpsilonTransitions(state);
        std::vector<std::string> epsLabels;
        for (int t : epsTargets) {
            epsLabels.push_back(getStateLabel(t));
        }
        transMap["ε"] = epsLabels;

        row["transitions"] = transMap;
        rows.push_back(row);
    }
    return rows;
}

nlohmann::json NFA::toJson() const {
    nlohmann::json j;
    j["type"] = "NFA";
    j["startState"] = startState;
    j["startStateLabel"] = getStateLabel(startState);

    nlohmann::json statesJson = nlohmann::json::array();
    for (int s : states) {
        nlohmann::json node;
        node["id"] = s;
        node["label"] = getStateLabel(s);
        node["isStart"] = (s == startState);
        node["isAccept"] = isAcceptState(s);
        statesJson.push_back(node);
    }
    j["states"] = statesJson;

    nlohmann::json acceptsJson = nlohmann::json::array();
    for (int s : acceptStates) {
        acceptsJson.push_back({{"id", s}, {"label", getStateLabel(s)}});
    }
    j["acceptStates"] = acceptsJson;

    nlohmann::json edgesJson = nlohmann::json::array();
    for (const auto& [key, targets] : transitions) {
        int from = key.first;
        char sym = key.second;
        bool isEps = (sym == EPSILON);
        std::string symStr = isEps ? "ε" : std::string(1, sym);

        for (int to : targets) {
            nlohmann::json edge;
            edge["from"] = from;
            edge["to"] = to;
            edge["symbol"] = symStr;
            edge["isEpsilon"] = isEps;
            edgesJson.push_back(edge);
        }
    }
    j["edges"] = edgesJson;
    j["alphabet"] = std::vector<std::string>{"A", "T", "G", "C"};
    j["transitionTable"] = getTransitionTableJson();

    return j;
}
