#include <iostream>
#include <cassert>
#include "PatternDetector.h"

int main() {
    std::cout << "Testing Pattern Detector...\n";

    // DNA: ATATAT, Pattern: ATAT -> 1, 3
    PatternDetectionResult r1 = PatternDetector::detect("ATATAT", "ATAT");
    assert(r1.success && r1.patternFound);
    assert(r1.totalOccurrences == 2);
    assert(r1.startingPositions.size() == 2);
    assert(r1.startingPositions[0] == 1 && r1.startingPositions[1] == 3);

    // Multiple occurrences
    PatternDetectionResult r2 = PatternDetector::detect("ATGCGATGATG", "ATG");
    assert(r2.totalOccurrences == 3);
    assert(r2.startingPositions.size() == 3);
    assert(r2.startingPositions[0] == 1 && r2.startingPositions[1] == 6 && r2.startingPositions[2] == 9);

    std::cout << "Pattern Detector tests passed!\n";
    return 0;
}
