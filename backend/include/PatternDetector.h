#ifndef PATTERN_DETECTOR_H
#define PATTERN_DETECTOR_H

#include <string>
#include <vector>
#include "NFA.h"
#include "DFA.h"
#include "SubsetConstruction.h"
#include "DNAValidator.h"
#include "json.hpp"

struct PatternMatchOccurrence {
    int matchNumber;
    int startIndex; // 1-based
    int endIndex;   // 1-based
    std::string matchedText;

    nlohmann::json toJson() const;
};

struct PatternDetectionResult {
    bool success = false;
    std::string errorMessage;
    std::string dnaSequence;
    std::string pattern;
    bool patternFound = false;
    int totalOccurrences = 0;
    std::vector<int> startingPositions; // 1-based
    std::vector<PatternMatchOccurrence> occurrences;
    std::vector<bool> matchMask; // True for each character index in dnaSequence that is part of a match

    // Automata used
    NFA matcherNFA;
    DFA matcherDFA;
    SubsetConversionReport conversionReport;

    nlohmann::json toJson() const;
};

class PatternDetector {
public:
    static PatternDetectionResult detect(const std::string& dnaInput, const std::string& patternInput);
};

#endif // PATTERN_DETECTOR_H
