#include "DFA.h"
#include <sstream>

DFA::DFA() : Automaton() {
    alphabet = {'A', 'T', 'G', 'C'};
}

void DFA::addTransition(int from, char symbol, int to) {
    states.insert(from);
    states.insert(to);
    alphabet.insert(symbol);
    transitions[{from, symbol}] = to;
}

int DFA::getTransition(int from, char symbol) const {
    auto it = transitions.find({from, symbol});
    if (it != transitions.end()) {
        return it->second;
    }
    return -1; // -1 denotes no transition / dead
}

bool DFA::hasTransition(int from, char symbol) const {
    return transitions.find({from, symbol}) != transitions.end();
}

void DFA::setStateSubset(int dfaState, const std::set<int>& nfaSubset, const std::string& subsetStr) {
    states.insert(dfaState);
    stateSubsets[dfaState] = nfaSubset;
    if (!subsetStr.empty()) {
        subsetStrings[dfaState] = subsetStr;
    } else {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (int s : nfaSubset) {
            if (!first) oss << ", ";
            oss << "q" << s;
            first = false;
        }
        oss << "}";
        subsetStrings[dfaState] = oss.str();
    }
}

const std::set<int>& DFA::getStateSubset(int dfaState) const {
    static const std::set<int> emptySet;
    auto it = stateSubsets.find(dfaState);
    if (it != stateSubsets.end()) {
        return it->second;
    }
    return emptySet;
}

std::string DFA::getSubsetString(int dfaState) const {
    auto it = subsetStrings.find(dfaState);
    if (it != subsetStrings.end()) {
        return it->second;
    }
    return "{}";
}

bool DFA::accepts(const std::string& input) const {
    int curr = startState;
    for (char c : input) {
        int next = getTransition(curr, c);
        if (next == -1) {
            return false;
        }
        curr = next;
    }
    return isAcceptState(curr);
}

std::vector<DFASimulationStep> DFA::simulateTrace(const std::string& input) const {
    std::vector<DFASimulationStep> trace;

    int curr = startState;
    DFASimulationStep initStep;
    initStep.stepNumber = 0;
    initStep.currentState = curr;
    initStep.currentStateLabel = getStateLabel(curr);
    initStep.currentSubset = getSubsetString(curr);
    initStep.inputSymbol = '\0';
    initStep.nextState = curr;
    initStep.nextStateLabel = getStateLabel(curr);
    initStep.nextSubset = getSubsetString(curr);
    initStep.transitionDescription = "Initial State: " + getStateLabel(curr);
    initStep.isAccepted = isAcceptState(curr);
    trace.push_back(initStep);

    int stepNum = 1;
    for (char c : input) {
        int next = getTransition(curr, c);
        DFASimulationStep step;
        step.stepNumber = stepNum++;
        step.currentState = curr;
        step.currentStateLabel = getStateLabel(curr);
        step.currentSubset = getSubsetString(curr);
        step.inputSymbol = c;
        step.nextState = next;

        if (next != -1) {
            step.nextStateLabel = getStateLabel(next);
            step.nextSubset = getSubsetString(next);
            step.transitionDescription = "δ(" + getStateLabel(curr) + ", " + std::string(1, c) + ") = " + getStateLabel(next);
            step.isAccepted = isAcceptState(next);
            trace.push_back(step);
            curr = next;
        } else {
            step.nextStateLabel = "TRAP / REJECT";
            step.nextSubset = "∅";
            step.transitionDescription = "δ(" + getStateLabel(curr) + ", " + std::string(1, c) + ") = ∅ (Undefined / Rejected)";
            step.isAccepted = false;
            trace.push_back(step);
            break; // Machine halts in reject state
        }
    }

    return trace;
}

nlohmann::json DFA::getTransitionTableJson() const {
    nlohmann::json rows = nlohmann::json::array();
    for (int state : states) {
        nlohmann::json row;
        row["state"] = state;
        row["stateLabel"] = getStateLabel(state);
        row["subset"] = getSubsetString(state);
        row["isStart"] = (state == startState);
        row["isAccept"] = isAcceptState(state);

        nlohmann::json transMap;
        for (char sym : {'A', 'T', 'G', 'C'}) {
            std::string key(1, sym);
            int next = getTransition(state, sym);
            if (next != -1) {
                transMap[key] = getStateLabel(next);
            } else {
                transMap[key] = "∅";
            }
        }
        row["transitions"] = transMap;
        rows.push_back(row);
    }
    return rows;
}

nlohmann::json DFA::toJson() const {
    nlohmann::json j;
    j["type"] = "DFA";
    j["startState"] = startState;
    j["startStateLabel"] = getStateLabel(startState);

    nlohmann::json statesJson = nlohmann::json::array();
    for (int s : states) {
        nlohmann::json node;
        node["id"] = s;
        node["label"] = getStateLabel(s);
        node["subset"] = getSubsetString(s);
        node["isStart"] = (s == startState);
        node["isAccept"] = isAcceptState(s);
        statesJson.push_back(node);
    }
    j["states"] = statesJson;

    nlohmann::json acceptsJson = nlohmann::json::array();
    for (int s : acceptStates) {
        acceptsJson.push_back({
            {"id", s},
            {"label", getStateLabel(s)},
            {"subset", getSubsetString(s)}
        });
    }
    j["acceptStates"] = acceptsJson;

    // Group edges between same from and to for cleaner diagrams
    std::map<std::pair<int, int>, std::string> combinedEdges;
    for (const auto& [key, to] : transitions) {
        int from = key.first;
        char sym = key.second;
        auto edgePair = std::make_pair(from, to);
        if (combinedEdges.find(edgePair) == combinedEdges.end()) {
            combinedEdges[edgePair] = std::string(1, sym);
        } else {
            combinedEdges[edgePair] += ", " + std::string(1, sym);
        }
    }

    nlohmann::json edgesJson = nlohmann::json::array();
    for (const auto& [pair, symbolsStr] : combinedEdges) {
        nlohmann::json edge;
        edge["from"] = pair.first;
        edge["to"] = pair.second;
        edge["symbol"] = symbolsStr;
        edge["isEpsilon"] = false;
        edgesJson.push_back(edge);
    }
    j["edges"] = edgesJson;
    j["alphabet"] = std::vector<std::string>{"A", "T", "G", "C"};
    j["transitionTable"] = getTransitionTableJson();

    return j;
}
