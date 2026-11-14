#include <iostream>
#include <cassert>
#include "DNAValidator.h"

int main() {
    std::cout << "Testing DNAValidator...\n";

    ValidationResult r1 = DNAValidator::validate("ATGCGATG");
    assert(r1.isValid && r1.length == 8 && r1.sanitizedSequence == "ATGCGATG");

    ValidationResult r2 = DNAValidator::validate("atgcgatg");
    assert(r2.isValid && r2.sanitizedSequence == "ATGCGATG");

    ValidationResult r3 = DNAValidator::validate("ATGCXZTG");
    assert(!r3.isValid && r3.invalidChar == 'X' && r3.errorPosition == 5);

    ValidationResult r4 = DNAValidator::validate("");
    assert(!r4.isValid);

    std::cout << "DNAValidator tests passed!\n";
    return 0;
}
