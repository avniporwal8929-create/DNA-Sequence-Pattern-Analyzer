#include "ThompsonConstruction.h"
#include <stack>
#include <stdexcept>
#include <map>

NFA ThompsonConstruction::buildFromPostfix(const std::string& postfix) {
    NFA rawNfa;
    std::stack<NFAFragment> fragmentStack;
    int stateCounter = 0;

    auto newState = [&]() -> int {
        int id = stateCounter++;
        rawNfa.addState(id);
        return id;
    };

    if (postfix.empty()) {
        int s = newState();
        rawNfa.setStartState(s);
        rawNfa.addAcceptState(s);
        return rawNfa;
    }

    for (char c : postfix) {
        if (c == 'A' || c == 'T' || c == 'G' || c == 'C') {
            int s0 = newState();
            int s1 = newState();
            rawNfa.addTransition(s0, c, s1);
            fragmentStack.push({s0, s1});
        } else if (c == '.') {
            // Concatenation: pop N2, then N1
            if (fragmentStack.size() < 2) {
                throw std::runtime_error("Malformed postfix regex: insufficient operands for concatenation.");
            }
            NFAFragment n2 = fragmentStack.top(); fragmentStack.pop();
            NFAFragment n1 = fragmentStack.top(); fragmentStack.pop();

            // Connect n1.accept to n2.start via epsilon transition
            rawNfa.addEpsilonTransition(n1.accept, n2.start);

            fragmentStack.push({n1.start, n2.accept});
        } else if (c == '|') {
            // Union: pop N2, then N1
            if (fragmentStack.size() < 2) {
                throw std::runtime_error("Malformed postfix regex: insufficient operands for union.");
            }
            NFAFragment n2 = fragmentStack.top(); fragmentStack.pop();
            NFAFragment n1 = fragmentStack.top(); fragmentStack.pop();

            int newStart = newState();
            int newAccept = newState();

            rawNfa.addEpsilonTransition(newStart, n1.start);
            rawNfa.addEpsilonTransition(newStart, n2.start);
            rawNfa.addEpsilonTransition(n1.accept, newAccept);
            rawNfa.addEpsilonTransition(n2.accept, newAccept);

            fragmentStack.push({newStart, newAccept});
        } else if (c == '*') {
            // Kleene Star: pop N
            if (fragmentStack.empty()) {
                throw std::runtime_error("Malformed postfix regex: insufficient operands for Kleene star.");
            }
            NFAFragment n = fragmentStack.top(); fragmentStack.pop();

            int newStart = newState();
            int newAccept = newState();

            rawNfa.addEpsilonTransition(newStart, n.start);
            rawNfa.addEpsilonTransition(newStart, newAccept);
            rawNfa.addEpsilonTransition(n.accept, n.start);
            rawNfa.addEpsilonTransition(n.accept, newAccept);

            fragmentStack.push({newStart, newAccept});
        }
    }

    if (fragmentStack.size() != 1) {
        throw std::runtime_error("Malformed postfix regex: evaluation did not reduce to a single fragment.");
    }

    NFAFragment finalFrag = fragmentStack.top();
    rawNfa.setStartState(finalFrag.start);
    rawNfa.addAcceptState(finalFrag.accept);

    // Normalize state IDs starting from 0, with start state = 0
    NFA cleanNfa;
    std::map<int, int> stateMap;
    int nextId = 0;

    // Start state gets id 0
    stateMap[finalFrag.start] = nextId++;

    for (int s : rawNfa.getStates()) {
        if (stateMap.find(s) == stateMap.end()) {
            stateMap[s] = nextId++;
        }
    }

    for (const auto& [oldId, newId] : stateMap) {
        cleanNfa.addState(newId, "q" + std::to_string(newId));
    }

    cleanNfa.setStartState(stateMap[finalFrag.start]);
    cleanNfa.addAcceptState(stateMap[finalFrag.accept]);

    for (const auto& [key, targets] : rawNfa.getAllTransitions()) {
        int from = stateMap[key.first];
        char sym = key.second;
        for (int t : targets) {
            int to = stateMap[t];
            cleanNfa.addTransition(from, sym, to);
        }
    }

    return cleanNfa;
}

NFA ThompsonConstruction::buildFromRegex(const std::string& regexStr) {
    RegexParseResult parsed = RegexParser::parse(regexStr);
    if (!parsed.isValid) {
        throw std::runtime_error(parsed.errorMessage);
    }
    return buildFromPostfix(parsed.postfixRegex);
}

NFA ThompsonConstruction::buildFromLiteral(const std::string& literal) {
    NFA nfa;
    if (literal.empty()) {
        nfa.setStartState(0);
        nfa.addAcceptState(0);
        nfa.setStateLabel(0, "q0");
        return nfa;
    }

    nfa.setStartState(0);
    nfa.setStateLabel(0, "q0");

    for (size_t i = 0; i < literal.size(); ++i) {
        char c = literal[i];
        int next = static_cast<int>(i + 1);
        nfa.addState(next, "q" + std::to_string(next));
        nfa.addTransition(static_cast<int>(i), c, next);
    }

    nfa.addAcceptState(static_cast<int>(literal.size()));
    return nfa;
}

NFA ThompsonConstruction::buildSubstringMatcher(const std::string& pattern) {
    NFA nfa = buildFromLiteral(pattern);

    // Self-loop on start state for all DNA symbols: allows matching pattern starting at any position
    for (char sym : {'A', 'T', 'G', 'C'}) {
        nfa.addTransition(0, sym, 0);
    }

    return nfa;
}
