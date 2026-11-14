#ifndef SEQUENCE_COMPARATOR_H
#define SEQUENCE_COMPARATOR_H

#include <string>
#include <vector>
#include "json.hpp"

enum class MutationType {
    MATCH,
    SUBSTITUTION,
    INSERTION,
    DELETION
};

struct AlignmentColumn {
    int alignmentPosition; // 1-based column in aligned representation
    std::string seq1Pos;   // 1-based index or "-"
    std::string seq2Pos;   // 1-based index or "-"
    char char1;            // Nucleotide or '-'
    char char2;            // Nucleotide or '-'
    std::string type;      // "Match", "Substitution", "Insertion", "Deletion"
    std::string explanation;

    nlohmann::json toJson() const;
};

struct ComparisonResult {
    bool success = false;
    std::string errorMessage;
    std::string originalSeq1;
    std::string originalSeq2;
    size_t length1 = 0;
    size_t length2 = 0;
    bool differenceFound = false;

    int totalMatches = 0;
    int totalSubstitutions = 0;
    int totalInsertions = 0;
    int totalDeletions = 0;
    double similarityPercentage = 0.0;

    std::string alignedSeq1;
    std::string matchSymbols; // '|' for match, '*' for substitution, ' ' for indel
    std::string alignedSeq2;

    std::vector<AlignmentColumn> alignmentColumns;
    std::vector<AlignmentColumn> differenceColumns; // Only substitutions, insertions, deletions

    std::string educationalDisclaimer =
        "Note: This tool uses theoretical sequence comparison algorithms (Needleman-Wunsch Dynamic Programming) "
        "strictly for educational and academic demonstration in Theory of Computation. It is not intended for medical, "
        "diagnostic, or clinical use.";

    nlohmann::json toJson() const;
};

class SequenceComparator {
public:
    static ComparisonResult compare(const std::string& input1, const std::string& input2);
};

#endif // SEQUENCE_COMPARATOR_H
