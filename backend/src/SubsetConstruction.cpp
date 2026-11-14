#include "SubsetConstruction.h"
#include <queue>
#include <sstream>

nlohmann::json SymbolTransitionStep::toJson() const {
    nlohmann::json j;
    j["symbol"] = std::string(1, symbol);
    j["moveSet"] = moveSet;
    j["moveSetString"] = moveSetString;
    j["closureSet"] = closureSet;
    j["closureSetString"] = closureSetString;
    j["targetStateId"] = targetStateId;
    j["targetStateLabel"] = targetStateLabel;
    j["isNewState"] = isNewState;
    j["explanation"] = explanation;
    return j;
}

nlohmann::json SubsetStep::toJson() const {
    nlohmann::json j;
    j["stepIndex"] = stepIndex;
    j["dfaStateId"] = dfaStateId;
    j["dfaStateLabel"] = dfaStateLabel;
    j["nfaSubset"] = nfaSubset;
    j["nfaSubsetString"] = nfaSubsetString;
    j["isStart"] = isStart;
    j["isAccept"] = isAccept;

    nlohmann::json transJson = nlohmann::json::array();
    for (const auto& t : transitions) {
        transJson.push_back(t.toJson());
    }
    j["transitions"] = transJson;
    return j;
}

nlohmann::json SubsetConversionReport::toJson() const {
    nlohmann::json j;
    j["dfa"] = dfa.toJson();

    nlohmann::json stepsJson = nlohmann::json::array();
    for (const auto& step : steps) {
        stepsJson.push_back(step.toJson());
    }
    j["steps"] = stepsJson;
    j["conversionTable"] = conversionTable;
    j["theoreticalExplanation"] = theoreticalExplanation;
    return j;
}

std::string SubsetConstruction::formatSet(const std::set<int>& s, const NFA& nfa) {
    if (s.empty()) return "∅";
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (int state : s) {
        if (!first) oss << ", ";
        oss << nfa.getStateLabel(state);
        first = false;
    }
    oss << "}";
    return oss.str();
}

std::string SubsetConstruction::generateDfaLabel(int index) {
    return "S" + std::to_string(index);
}

SubsetConversionReport SubsetConstruction::convert(const NFA& nfa) {
    SubsetConversionReport report;
    DFA dfa;

    std::map<std::set<int>, int> subsetToDfaState;
    std::vector<std::set<int>> dfaStateToSubset;
    std::queue<int> unmarkedQueue;

    // Step 1: Compute epsilon-closure of NFA start state
    std::set<int> startClosure = nfa.epsilonClosure(nfa.getStartState());

    int startDfaId = 0;
    std::string startLabel = generateDfaLabel(startDfaId);
    subsetToDfaState[startClosure] = startDfaId;
    dfaStateToSubset.push_back(startClosure);

    dfa.addState(startDfaId, startLabel);
    dfa.setStartState(startDfaId);
    dfa.setStateSubset(startDfaId, startClosure, formatSet(startClosure, nfa));

    // Check if start state is accept state
    bool startIsAccept = false;
    for (int s : startClosure) {
        if (nfa.isAcceptState(s)) {
            startIsAccept = true;
            break;
        }
    }
    if (startIsAccept) {
        dfa.addAcceptState(startDfaId);
    }

    unmarkedQueue.push(startDfaId);

    const std::vector<char> dnaAlphabet = {'A', 'T', 'G', 'C'};
    int stepCounter = 1;

    nlohmann::json conversionTableRows = nlohmann::json::array();

    // Subset Construction Main Loop
    while (!unmarkedQueue.empty()) {
        int currentDfaId = unmarkedQueue.front();
        unmarkedQueue.pop();

        std::set<int> currentSubset = dfaStateToSubset[currentDfaId];
        std::string currentLabel = dfa.getStateLabel(currentDfaId);
        std::string currentSubstr = formatSet(currentSubset, nfa);
        bool currentIsAccept = dfa.isAcceptState(currentDfaId);

        SubsetStep step;
        step.stepIndex = stepCounter++;
        step.dfaStateId = currentDfaId;
        step.dfaStateLabel = currentLabel;
        step.nfaSubset = currentSubset;
        step.nfaSubsetString = currentSubstr;
        step.isStart = (currentDfaId == startDfaId);
        step.isAccept = currentIsAccept;

        nlohmann::json tableRow;
        tableRow["dfaState"] = currentLabel;
        tableRow["nfaSubset"] = currentSubstr;
        tableRow["isStart"] = step.isStart;
        tableRow["isAccept"] = currentIsAccept;
        nlohmann::json transMap;

        for (char sym : dnaAlphabet) {
            std::string symStr(1, sym);
            std::set<int> moveResult = nfa.move(currentSubset, sym);
            std::string moveStr = formatSet(moveResult, nfa);

            std::set<int> closureResult = nfa.epsilonClosure(moveResult);
            std::string closureStr = formatSet(closureResult, nfa);

            SymbolTransitionStep symStep;
            symStep.symbol = sym;
            symStep.moveSet = moveResult;
            symStep.moveSetString = moveStr;
            symStep.closureSet = closureResult;
            symStep.closureSetString = closureStr;

            if (closureResult.empty()) {
                symStep.targetStateId = -1;
                symStep.targetStateLabel = "∅";
                symStep.isNewState = false;
                symStep.explanation = "Move(" + currentSubstr + ", " + symStr + ") = ∅. No transition defined.";
                transMap[symStr] = "∅";
            } else {
                auto it = subsetToDfaState.find(closureResult);
                if (it == subsetToDfaState.end()) {
                    // New DFA state discovered
                    int newDfaId = static_cast<int>(dfaStateToSubset.size());
                    std::string newLabel = generateDfaLabel(newDfaId);

                    subsetToDfaState[closureResult] = newDfaId;
                    dfaStateToSubset.push_back(closureResult);

                    dfa.addState(newDfaId, newLabel);
                    dfa.setStateSubset(newDfaId, closureResult, closureStr);

                    // Check if new state contains an NFA accept state
                    bool isNewAccept = false;
                    for (int s : closureResult) {
                        if (nfa.isAcceptState(s)) {
                            isNewAccept = true;
                            break;
                        }
                    }
                    if (isNewAccept) {
                        dfa.addAcceptState(newDfaId);
                    }

                    unmarkedQueue.push(newDfaId);

                    symStep.targetStateId = newDfaId;
                    symStep.targetStateLabel = newLabel;
                    symStep.isNewState = true;
                    symStep.explanation = "Move(" + currentSubstr + ", " + symStr + ") = " + moveStr +
                                          ", ε-closure = " + closureStr +
                                          " → New DFA state " + newLabel + " created and enqueued.";
                    transMap[symStr] = newLabel;
                } else {
                    int existingId = it->second;
                    std::string existingLabel = dfa.getStateLabel(existingId);

                    symStep.targetStateId = existingId;
                    symStep.targetStateLabel = existingLabel;
                    symStep.isNewState = false;
                    symStep.explanation = "Move(" + currentSubstr + ", " + symStr + ") = " + moveStr +
                                          ", ε-closure = " + closureStr +
                                          " → Maps to existing DFA state " + existingLabel + ".";
                    transMap[symStr] = existingLabel;
                }

                dfa.addTransition(currentDfaId, sym, symStep.targetStateId);
            }

            step.transitions.push_back(symStep);
        }

        tableRow["transitions"] = transMap;
        conversionTableRows.push_back(tableRow);
        report.steps.push_back(step);
    }

    report.dfa = dfa;
    report.conversionTable = conversionTableRows;

    report.theoreticalExplanation =
        "The Subset Construction (Rabin-Scott Powersett Construction) converts a Non-Deterministic Finite "
        "Automaton (NFA) with N states into an equivalent Deterministic Finite Automaton (DFA). "
        "Each DFA state corresponds to a subset of NFA states. "
        "1. Initial State: The DFA initial state S0 is the ε-closure of the NFA start state. "
        "2. Transitions: For each DFA state U and nucleotide symbol a ∈ {A, T, G, C}, the target DFA state "
        "is δ_DFA(U, a) = ε-closure(Move(U, a)), where Move(U, a) = ⋃_{s ∈ U} δ_NFA(s, a). "
        "3. Final States: A DFA state is accepting if and only if its state subset contains at least one NFA accepting state. "
        "This mathematically guarantees that the language recognized by the DFA, L(DFA), is identical to L(NFA).";

    return report;
}
