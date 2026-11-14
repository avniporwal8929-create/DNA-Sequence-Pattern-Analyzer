#include "PatternDetector.h"
#include "ThompsonConstruction.h"

nlohmann::json PatternMatchOccurrence::toJson() const {
    nlohmann::json j;
    j["matchNumber"] = matchNumber;
    j["startIndex"] = startIndex;
    j["endIndex"] = endIndex;
    j["matchedText"] = matchedText;
    return j;
}

nlohmann::json PatternDetectionResult::toJson() const {
    nlohmann::json j;
    j["success"] = success;
    j["errorMessage"] = errorMessage;
    j["dnaSequence"] = dnaSequence;
    j["pattern"] = pattern;
    j["patternFound"] = patternFound;
    j["totalOccurrences"] = totalOccurrences;
    j["startingPositions"] = startingPositions;

    nlohmann::json occs = nlohmann::json::array();
    for (const auto& o : occurrences) {
        occs.push_back(o.toJson());
    }
    j["occurrences"] = occs;

    // Convert boolean mask to array of ints
    std::vector<int> maskInts;
    maskInts.reserve(matchMask.size());
    for (bool b : matchMask) maskInts.push_back(b ? 1 : 0);
    j["matchMask"] = maskInts;

    j["matcherNFA"] = matcherNFA.toJson();
    j["matcherDFA"] = matcherDFA.toJson();
    j["conversionSteps"] = conversionReport.toJson()["steps"];
    j["conversionTable"] = conversionReport.conversionTable;

    return j;
}

PatternDetectionResult PatternDetector::detect(const std::string& dnaInput, const std::string& patternInput) {
    PatternDetectionResult result;

    // 1. Validate DNA Sequence
    ValidationResult dnaVal = DNAValidator::validate(dnaInput);
    if (!dnaVal.isValid) {
        result.success = false;
        result.errorMessage = "DNA sequence validation failed: " + dnaVal.errorMessage;
        return result;
    }

    // 2. Validate Pattern
    ValidationResult patVal = DNAValidator::validate(patternInput);
    if (!patVal.isValid) {
        result.success = false;
        result.errorMessage = "Pattern validation failed: " + patVal.errorMessage;
        return result;
    }

    result.dnaSequence = dnaVal.sanitizedSequence;
    result.pattern = patVal.sanitizedSequence;
    result.matchMask.assign(result.dnaSequence.size(), false);

    if (result.pattern.size() > result.dnaSequence.size()) {
        result.success = true;
        result.patternFound = false;
        result.totalOccurrences = 0;
        result.matcherNFA = ThompsonConstruction::buildSubstringMatcher(result.pattern);
        result.conversionReport = SubsetConstruction::convert(result.matcherNFA);
        result.matcherDFA = result.conversionReport.dfa;
        return result;
    }

    // 3. Build Substring Pattern-Matching NFA:
    // State 0 loops on {A, T, G, C}, and transitions sequentially along pattern characters to final state
    result.matcherNFA = ThompsonConstruction::buildSubstringMatcher(result.pattern);

    // 4. Convert NFA to DFA via Subset Construction
    result.conversionReport = SubsetConstruction::convert(result.matcherNFA);
    result.matcherDFA = result.conversionReport.dfa;

    // 5. Run the DFA on the DNA sequence to detect all occurrences
    int currState = result.matcherDFA.getStartState();
    size_t patternLen = result.pattern.size();
    int matchCounter = 1;

    for (size_t i = 0; i < result.dnaSequence.size(); ++i) {
        char symbol = result.dnaSequence[i];
        int nextState = result.matcherDFA.getTransition(currState, symbol);

        if (nextState == -1) {
            currState = result.matcherDFA.getStartState();
        } else {
            currState = nextState;
        }

        // Check if the current DFA state is an accepting state
        if (result.matcherDFA.isAcceptState(currState)) {
            // Occurrence detected!
            // i is 0-based index of the last character of the pattern
            int startIdx0 = static_cast<int>(i - patternLen + 1);
            int startIdx1 = startIdx0 + 1; // 1-based indexing
            int endIdx1 = static_cast<int>(i + 1);

            result.startingPositions.push_back(startIdx1);

            PatternMatchOccurrence occ;
            occ.matchNumber = matchCounter++;
            occ.startIndex = startIdx1;
            occ.endIndex = endIdx1;
            occ.matchedText = result.dnaSequence.substr(startIdx0, patternLen);
            result.occurrences.push_back(occ);

            // Mark characters in mask
            for (int k = startIdx0; k <= static_cast<int>(i); ++k) {
                if (k >= 0 && k < static_cast<int>(result.matchMask.size())) {
                    result.matchMask[k] = true;
                }
            }
        }
    }

    result.totalOccurrences = static_cast<int>(result.startingPositions.size());
    result.patternFound = (result.totalOccurrences > 0);
    result.success = true;

    return result;
}
