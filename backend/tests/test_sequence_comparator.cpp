#include <iostream>
#include <cassert>
#include "SequenceComparator.h"

int main() {
    std::cout << "Testing Sequence Comparator...\n";

    // Substitution: ATGCGT vs ATGAGT -> Position 4 (C -> A)
    ComparisonResult r1 = SequenceComparator::compare("ATGCGT", "ATGAGT");
    assert(r1.success && r1.differenceFound);
    assert(r1.totalSubstitutions == 1);
    assert(r1.differenceColumns.size() == 1);
    assert(r1.differenceColumns[0].seq1Pos == "4");
    assert(r1.differenceColumns[0].char1 == 'C');
    assert(r1.differenceColumns[0].char2 == 'A');

    // Identical
    ComparisonResult r2 = SequenceComparator::compare("ATGC", "ATGC");
    assert(r2.success && !r2.differenceFound && r2.totalMatches == 4);
    assert(r2.similarityPercentage == 100.0);

    std::cout << "Sequence Comparator tests passed!\n";
    return 0;
}
