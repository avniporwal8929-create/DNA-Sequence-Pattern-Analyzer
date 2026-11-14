#include <iostream>
#include <cassert>
#include "ThompsonConstruction.h"

int main() {
    std::cout << "Testing NFA and Thompson Construction...\n";

    NFA nfa = ThompsonConstruction::buildFromRegex("(A|T)G");
    assert(nfa.accepts("AG"));
    assert(nfa.accepts("TG"));
    assert(!nfa.accepts("AT"));
    assert(!nfa.accepts("G"));

    NFA starNfa = ThompsonConstruction::buildFromRegex("A*");
    assert(starNfa.accepts(""));
    assert(starNfa.accepts("A"));
    assert(starNfa.accepts("AAA"));
    assert(!starNfa.accepts("C"));

    std::cout << "NFA tests passed!\n";
    return 0;
}
