#include "PatternDetector.h"
#include "ThompsonConstruction.h"

nlohmann::json PatternMatchOccurrence::toJson() const {
    nlohmann::json j;
    j["matchNumber"] = matchNumber;
    j["startIndex"] = startIndex;
    j["endIndex"] = endIndex;
    j["matchedText"] = matchedText;
    j["patternTag"] = patternTag;
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

nlohmann::json MultiPatternResult::toJson() const {
    nlohmann::json j;
    j["success"] = success;
    j["errorMessage"] = errorMessage;
    j["dnaSequence"] = dnaSequence;
    j["totalPatternsSearched"] = totalPatternsSearched;
    j["totalOccurrencesCombined"] = totalOccurrencesCombined;

    nlohmann::json arr = nlohmann::json::array();
    for (const auto& pr : patternResults) {
        arr.push_back(pr.toJson());
    }
    j["patternResults"] = arr;

    std::vector<int> maskInts;
    maskInts.reserve(combinedMatchMask.size());
    for (bool b : combinedMatchMask) maskInts.push_back(b ? 1 : 0);
    j["combinedMatchMask"] = maskInts;

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
            int startIdx0 = static_cast<int>(i - patternLen + 1);
            int startIdx1 = startIdx0 + 1; // 1-based indexing
            int endIdx1 = static_cast<int>(i + 1);

            result.startingPositions.push_back(startIdx1);

            PatternMatchOccurrence occ;
            occ.matchNumber = matchCounter++;
            occ.startIndex = startIdx1;
            occ.endIndex = endIdx1;
            occ.matchedText = result.dnaSequence.substr(startIdx0, patternLen);
            occ.patternTag = result.pattern;
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

MultiPatternResult PatternDetector::detectMultiple(const std::string& dnaInput, const std::vector<std::string>& patterns) {
    MultiPatternResult multiRes;

    ValidationResult dnaVal = DNAValidator::validate(dnaInput);
    if (!dnaVal.isValid) {
        multiRes.success = false;
        multiRes.errorMessage = "DNA sequence validation failed: " + dnaVal.errorMessage;
        return multiRes;
    }

    multiRes.dnaSequence = dnaVal.sanitizedSequence;
    multiRes.combinedMatchMask.assign(multiRes.dnaSequence.size(), false);
    multiRes.totalPatternsSearched = static_cast<int>(patterns.size());

    int combinedCount = 0;
    for (const auto& rawPat : patterns) {
        if (rawPat.empty()) continue;
        PatternDetectionResult singleRes = detect(multiRes.dnaSequence, rawPat);
        if (singleRes.success) {
            combinedCount += singleRes.totalOccurrences;
            for (size_t k = 0; k < singleRes.matchMask.size(); ++k) {
                if (singleRes.matchMask[k]) {
                    multiRes.combinedMatchMask[k] = true;
                }
            }
            multiRes.patternResults.push_back(singleRes);
        }
    }

    multiRes.totalOccurrencesCombined = combinedCount;
    multiRes.success = true;
    return multiRes;
}
