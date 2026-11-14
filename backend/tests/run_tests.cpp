#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "DNAValidator.h"
#include "RegexParser.h"
#include "ThompsonConstruction.h"
#include "SubsetConstruction.h"
#include "PatternDetector.h"
#include "SequenceComparator.h"

int totalTests = 0;
int passedTests = 0;

void recordTest(const std::string& name, bool condition) {
    totalTests++;
    if (condition) {
        passedTests++;
        std::cout << "  [PASS] " << name << "\n";
    } else {
        std::cerr << "  [FAIL] " << name << "\n";
    }
}

void testDNAValidator() {
    std::cout << "\n=== 1. Testing DNA Sequence Validator ===\n";

    // Valid sequence
    ValidationResult r1 = DNAValidator::validate("ATGCGATG");
    recordTest("Valid DNA sequence accepted", r1.isValid && r1.length == 8 && r1.sanitizedSequence == "ATGCGATG");

    // Lowercase conversion
    ValidationResult r2 = DNAValidator::validate("atgcgatg");
    recordTest("Lowercase input converted to uppercase", r2.isValid && r2.sanitizedSequence == "ATGCGATG");

    // Invalid symbol
    ValidationResult r3 = DNAValidator::validate("ATGCXZTG");
    recordTest("Invalid nucleotide rejected with correct position", !r3.isValid && r3.invalidChar == 'X' && r3.errorPosition == 5);

    // Empty sequence
    ValidationResult r4 = DNAValidator::validate("");
    recordTest("Empty sequence handled", !r4.isValid);

    // Whitespace only
    ValidationResult r5 = DNAValidator::validate("   \n\t ");
    recordTest("Whitespace-only input rejected", !r5.isValid);
}

void testRegexParser() {
    std::cout << "\n=== 2. Testing Regular Expression Parser ===\n";

    // Simple concatenation
    RegexParseResult r1 = RegexParser::parse("ATG");
    recordTest("Simple concatenation: ATG", r1.isValid && r1.postfixRegex == "AT.G.");

    // Union
    RegexParseResult r2 = RegexParser::parse("A|T");
    recordTest("Union operator: A|T", r2.isValid && r2.postfixRegex == "AT|");

    // Kleene star
    RegexParseResult r3 = RegexParser::parse("AT*");
    recordTest("Kleene star operator: AT*", r3.isValid && r3.postfixRegex == "AT*.");

    // Parentheses
    RegexParseResult r4 = RegexParser::parse("(A|T)G");
    recordTest("Parentheses: (A|T)G", r4.isValid && r4.postfixRegex == "AT|G.");

    // Complex combination
    RegexParseResult r5 = RegexParser::parse("(A|T)*(G|C)");
    recordTest("Complex regex: (A|T)*(G|C)", r5.isValid && r5.postfixRegex == "AT|*GC|.");

    // Syntax error: mismatched paren
    RegexParseResult r6 = RegexParser::parse("(A|T");
    recordTest("Syntax error detected: unclosed '('", !r6.isValid);

    // Syntax error: empty paren
    RegexParseResult r7 = RegexParser::parse("A()T");
    recordTest("Syntax error detected: empty '()'", !r7.isValid);

    // Syntax error: starting with star
    RegexParseResult r8 = RegexParser::parse("*ATG");
    recordTest("Syntax error detected: leading '*'", !r8.isValid);
}

void testThompsonAndSubsetConstruction() {
    std::cout << "\n=== 3. Testing Thompson Construction & Subset Construction ===\n";

    // Test 1: NFA for (A|T)G
    NFA nfa = ThompsonConstruction::buildFromRegex("(A|T)G");
    recordTest("NFA recognizes AG", nfa.accepts("AG"));
    recordTest("NFA recognizes TG", nfa.accepts("TG"));
    recordTest("NFA rejects AT", !nfa.accepts("AT"));
    recordTest("NFA rejects G", !nfa.accepts("G"));

    // Subset Construction
    SubsetConversionReport report = SubsetConstruction::convert(nfa);
    DFA dfa = report.dfa;

    recordTest("Subset construction produced valid DFA", !dfa.getStates().empty());
    recordTest("DFA recognizes AG", dfa.accepts("AG"));
    recordTest("DFA recognizes TG", dfa.accepts("TG"));
    recordTest("DFA rejects AT", !dfa.accepts("AT"));
    recordTest("DFA rejects G", !dfa.accepts("G"));
    recordTest("DFA rejects AGG", !dfa.accepts("AGG"));

    // Test 2: Kleene star A*
    NFA nfaStar = ThompsonConstruction::buildFromRegex("A*");
    DFA dfaStar = SubsetConstruction::convert(nfaStar).dfa;
    recordTest("A* accepts empty string", dfaStar.accepts(""));
    recordTest("A* accepts A", dfaStar.accepts("A"));
    recordTest("A* accepts AAAA", dfaStar.accepts("AAAA"));
    recordTest("A* rejects T", !dfaStar.accepts("T"));
}

void testPatternDetector() {
    std::cout << "\n=== 4. Testing Automata-Based Pattern Detector ===\n";

    // Test overlapping pattern occurrences as explicitly requested
    // DNA: ATATAT, Pattern: ATAT -> Expected starting positions: 1, 3
    PatternDetectionResult r1 = PatternDetector::detect("ATATAT", "ATAT");
    recordTest("Overlapping pattern detection success", r1.success && r1.patternFound);
    recordTest("Overlapping occurrences count == 2", r1.totalOccurrences == 2);
    bool pos1Correct = (r1.startingPositions.size() >= 2 && r1.startingPositions[0] == 1 && r1.startingPositions[1] == 3);
    recordTest("Overlapping starting positions 1 and 3", pos1Correct);

    // Standard pattern occurrence
    PatternDetectionResult r2 = PatternDetector::detect("ATGCGATGATG", "ATG");
    recordTest("Multiple occurrences found (3 times)", r2.totalOccurrences == 3);
    bool pos2Correct = (r2.startingPositions.size() == 3 && r2.startingPositions[0] == 1 && r2.startingPositions[1] == 6 && r2.startingPositions[2] == 9);
    recordTest("Starting positions 1, 6, 9", pos2Correct);

    // Pattern at beginning
    PatternDetectionResult r3 = PatternDetector::detect("GCAT", "GC");
    recordTest("Pattern at beginning", r3.patternFound && r3.startingPositions.size() == 1 && r3.startingPositions[0] == 1);

    // Pattern at end
    PatternDetectionResult r4 = PatternDetector::detect("ATGC", "GC");
    recordTest("Pattern at end", r4.patternFound && r4.startingPositions.size() == 1 && r4.startingPositions[0] == 3);

    // Pattern not found
    PatternDetectionResult r5 = PatternDetector::detect("AAAAAA", "TT");
    recordTest("Pattern not found handled", !r5.patternFound && r5.totalOccurrences == 0);

    // Pattern longer than sequence
    PatternDetectionResult r6 = PatternDetector::detect("ATG", "ATGCGATG");
    recordTest("Pattern longer than sequence handled", !r6.patternFound && r6.totalOccurrences == 0);

    // Multi-pattern search
    MultiPatternResult r7 = PatternDetector::detectMultiple("ATGCGATGATG", {"ATG", "CGA"});
    recordTest("Multi-pattern detection succeeds", r7.success && r7.totalOccurrencesCombined == 4);
    recordTest("Multi-pattern searched 2 patterns", r7.totalPatternsSearched == 2);
}

void testSequenceComparator() {
    std::cout << "\n=== 5. Testing Sequence Comparator (Mutation Detection) ===\n";

    // Identical sequences
    ComparisonResult r1 = SequenceComparator::compare("ATGCGT", "ATGCGT");
    recordTest("Identical sequences have 0 differences", r1.success && !r1.differenceFound && r1.totalMatches == 6);
    recordTest("Identical sequences similarity is 100%", r1.similarityPercentage == 100.0);

    // Single nucleotide substitution (SNP)
    ComparisonResult r2 = SequenceComparator::compare("ATGCGT", "ATGAGT");
    recordTest("Substitution detected", r2.success && r2.differenceFound && r2.totalSubstitutions == 1);
    bool subPosCorrect = (!r2.differenceColumns.empty() && r2.differenceColumns[0].seq1Pos == "4" && r2.differenceColumns[0].char1 == 'C' && r2.differenceColumns[0].char2 == 'A');
    recordTest("Substitution position 4 (C -> A)", subPosCorrect);

    // Insertion
    ComparisonResult r3 = SequenceComparator::compare("ATGT", "ATGCT");
    recordTest("Insertion detected", r3.success && r3.differenceFound && r3.totalInsertions >= 1);

    // Deletion
    ComparisonResult r4 = SequenceComparator::compare("ATGCT", "ATGT");
    recordTest("Deletion detected", r4.success && r4.differenceFound && r4.totalDeletions >= 1);

    // Different lengths
    ComparisonResult r5 = SequenceComparator::compare("ATGCGATCG", "ATG");
    recordTest("Different sequence lengths handled", r5.success && r5.differenceFound);
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  THEORY OF COMPUTATION - DNA ANALYZER C++ UNIT TESTS  \n";
    std::cout << "========================================================\n";

    testDNAValidator();
    testRegexParser();
    testThompsonAndSubsetConstruction();
    testPatternDetector();
    testSequenceComparator();

    std::cout << "\n--------------------------------------------------------\n";
    std::cout << "Test Summary: " << passedTests << " / " << totalTests << " passed.\n";
    std::cout << "--------------------------------------------------------\n";

    if (passedTests == totalTests) {
        std::cout << ">>> ALL TESTS PASSED SUCCESSFULLY! <<<\n";
        return 0;
    } else {
        std::cerr << ">>> SOME TESTS FAILED! <<<\n";
        return 1;
    }
}
