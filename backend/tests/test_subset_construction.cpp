#include <iostream>
#include <cassert>
#include "ThompsonConstruction.h"
#include "SubsetConstruction.h"

int main() {
    std::cout << "Testing Subset Construction...\n";

    NFA nfa = ThompsonConstruction::buildFromRegex("(A|T)G");
    SubsetConversionReport report = SubsetConstruction::convert(nfa);
    DFA dfa = report.dfa;

    assert(!dfa.getStates().empty());
    assert(dfa.accepts("AG"));
    assert(dfa.accepts("TG"));
    assert(!dfa.accepts("AT"));
    assert(!dfa.accepts("G"));
    assert(!dfa.accepts("AGG"));

    // Check that DFA states have subsets mapped
    for (int s : dfa.getStates()) {
        assert(!dfa.getSubsetString(s).empty());
    }

    std::cout << "Subset Construction tests passed!\n";
    return 0;
}
