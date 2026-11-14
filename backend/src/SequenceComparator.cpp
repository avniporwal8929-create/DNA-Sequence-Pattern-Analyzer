#include "SequenceComparator.h"
#include "DNAValidator.h"
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

nlohmann::json AlignmentColumn::toJson() const {
    nlohmann::json j;
    j["alignmentPosition"] = alignmentPosition;
    j["seq1Pos"] = seq1Pos;
    j["seq2Pos"] = seq2Pos;
    j["char1"] = std::string(1, char1);
    j["char2"] = std::string(1, char2);
    j["type"] = type;
    j["explanation"] = explanation;
    return j;
}

nlohmann::json ComparisonResult::toJson() const {
    nlohmann::json j;
    j["success"] = success;
    j["errorMessage"] = errorMessage;
    j["originalSeq1"] = originalSeq1;
    j["originalSeq2"] = originalSeq2;
    j["length1"] = length1;
    j["length2"] = length2;
    j["differenceFound"] = differenceFound;
    j["totalMatches"] = totalMatches;
    j["totalSubstitutions"] = totalSubstitutions;
    j["totalInsertions"] = totalInsertions;
    j["totalDeletions"] = totalDeletions;
    j["similarityPercentage"] = similarityPercentage;
    j["alignedSeq1"] = alignedSeq1;
    j["matchSymbols"] = matchSymbols;
    j["alignedSeq2"] = alignedSeq2;

    nlohmann::json diffCols = nlohmann::json::array();
    for (const auto& col : differenceColumns) {
        diffCols.push_back(col.toJson());
    }
    j["differences"] = diffCols;

    nlohmann::json allCols = nlohmann::json::array();
    for (const auto& col : alignmentColumns) {
        allCols.push_back(col.toJson());
    }
    j["allColumns"] = allCols;
    j["educationalDisclaimer"] = educationalDisclaimer;

    return j;
}

ComparisonResult SequenceComparator::compare(const std::string& input1, const std::string& input2) {
    ComparisonResult result;

    ValidationResult val1 = DNAValidator::validate(input1);
    if (!val1.isValid) {
        result.success = false;
        result.errorMessage = "Sequence 1 validation failed: " + val1.errorMessage;
        return result;
    }

    ValidationResult val2 = DNAValidator::validate(input2);
    if (!val2.isValid) {
        result.success = false;
        result.errorMessage = "Sequence 2 validation failed: " + val2.errorMessage;
        return result;
    }

    const std::string& s1 = val1.sanitizedSequence;
    const std::string& s2 = val2.sanitizedSequence;

    result.originalSeq1 = s1;
    result.originalSeq2 = s2;
    result.length1 = s1.size();
    result.length2 = s2.size();

    // Needleman-Wunsch Global Alignment Parameters
    const int MATCH_SCORE = 2;
    const int MISMATCH_PENALTY = -1;
    const int GAP_PENALTY = -2;

    size_t n = s1.size();
    size_t m = s2.size();

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (size_t i = 0; i <= n; ++i) dp[i][0] = static_cast<int>(i) * GAP_PENALTY;
    for (size_t j = 0; j <= m; ++j) dp[0][j] = static_cast<int>(j) * GAP_PENALTY;

    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            int scoreDiag = dp[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? MATCH_SCORE : MISMATCH_PENALTY);
            int scoreUp = dp[i - 1][j] + GAP_PENALTY;
            int scoreLeft = dp[i][j - 1] + GAP_PENALTY;

            dp[i][j] = std::max({scoreDiag, scoreUp, scoreLeft});
        }
    }

    // Backtrack to reconstruct the optimal alignment
    std::string align1, align2;
    size_t i = n, j = m;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && dp[i][j] == dp[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? MATCH_SCORE : MISMATCH_PENALTY)) {
            align1.push_back(s1[i - 1]);
            align2.push_back(s2[j - 1]);
            --i;
            --j;
        } else if (i > 0 && dp[i][j] == dp[i - 1][j] + GAP_PENALTY) {
            align1.push_back(s1[i - 1]);
            align2.push_back('-');
            --i;
        } else {
            align1.push_back('-');
            align2.push_back(s2[j - 1]);
            --j;
        }
    }

    std::reverse(align1.begin(), align1.end());
    std::reverse(align2.begin(), align2.end());

    result.alignedSeq1 = align1;
    result.alignedSeq2 = align2;

    std::string matchSymbols;
    matchSymbols.reserve(align1.size());

    int p1 = 0;
    int p2 = 0;

    for (size_t k = 0; k < align1.size(); ++k) {
        char c1 = align1[k];
        char c2 = align2[k];

        std::string pos1Str = "-";
        std::string pos2Str = "-";

        if (c1 != '-') {
            ++p1;
            pos1Str = std::to_string(p1);
        }
        if (c2 != '-') {
            ++p2;
            pos2Str = std::to_string(p2);
        }

        AlignmentColumn col;
        col.alignmentPosition = static_cast<int>(k + 1);
        col.seq1Pos = pos1Str;
        col.seq2Pos = pos2Str;
        col.char1 = c1;
        col.char2 = c2;

        if (c1 == c2) {
            col.type = "Match";
            col.explanation = "Identical nucleotide '" + std::string(1, c1) + "' at Seq1[" + pos1Str + "] and Seq2[" + pos2Str + "].";
            matchSymbols.push_back('|');
            result.totalMatches++;
        } else if (c1 != '-' && c2 != '-') {
            col.type = "Substitution";
            col.explanation = "Nucleotide substitution: '" + std::string(1, c1) + "' at Seq1[" + pos1Str + "] replaced by '" + std::string(1, c2) + "' in Seq2[" + pos2Str + "].";
            matchSymbols.push_back('*');
            result.totalSubstitutions++;
            result.differenceColumns.push_back(col);
        } else if (c1 == '-' && c2 != '-') {
            col.type = "Insertion";
            col.explanation = "Insertion: nucleotide '" + std::string(1, c2) + "' inserted in Seq2 at position " + pos2Str + " (absent in Seq1).";
            matchSymbols.push_back('+');
            result.totalInsertions++;
            result.differenceColumns.push_back(col);
        } else {
            col.type = "Deletion";
            col.explanation = "Deletion: nucleotide '" + std::string(1, c1) + "' at Seq1[" + pos1Str + "] deleted in Seq2.";
            matchSymbols.push_back('-');
            result.totalDeletions++;
            result.differenceColumns.push_back(col);
        }

        result.alignmentColumns.push_back(col);
    }

    result.matchSymbols = matchSymbols;
    result.differenceFound = (result.totalSubstitutions > 0 || result.totalInsertions > 0 || result.totalDeletions > 0);

    if (!align1.empty()) {
        result.similarityPercentage = (static_cast<double>(result.totalMatches) / static_cast<double>(align1.size())) * 100.0;
    } else {
        result.similarityPercentage = 0.0;
    }

    result.success = true;
    return result;
}
