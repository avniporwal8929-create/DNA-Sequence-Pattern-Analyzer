#ifndef THOMPSON_CONSTRUCTION_H
#define THOMPSON_CONSTRUCTION_H

#include "NFA.h"
#include "RegexParser.h"
#include <string>

struct NFAFragment {
    int start;
    int accept;
};

class ThompsonConstruction {
public:
    // Build NFA from parsed postfix regex (exact language recognition)
    static NFA buildFromPostfix(const std::string& postfix);

    // Build NFA directly from regex string (handles parsing and validation)
    static NFA buildFromRegex(const std::string& regexStr);

    // Build NFA for exact literal pattern: c1 c2 ... cm
    static NFA buildFromLiteral(const std::string& literal);

    // Build substring pattern-matching NFA (adds Sigma* self-loop at start state)
    // Used for automata-based search/detection across entire DNA sequence
    static NFA buildSubstringMatcher(const std::string& pattern);
};

#endif // THOMPSON_CONSTRUCTION_H
