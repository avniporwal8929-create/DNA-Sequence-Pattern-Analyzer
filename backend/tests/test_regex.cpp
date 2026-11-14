#include <iostream>
#include <cassert>
#include "RegexParser.h"

int main() {
    std::cout << "Testing RegexParser...\n";

    RegexParseResult r1 = RegexParser::parse("ATG");
    assert(r1.isValid && r1.postfixRegex == "AT.G.");

    RegexParseResult r2 = RegexParser::parse("A|T");
    assert(r2.isValid && r2.postfixRegex == "AT|");

    RegexParseResult r3 = RegexParser::parse("AT*");
    assert(r3.isValid && r3.postfixRegex == "AT*.");

    RegexParseResult r4 = RegexParser::parse("(A|T)G");
    assert(r4.isValid && r4.postfixRegex == "AT|G.");

    RegexParseResult r5 = RegexParser::parse("*ATG");
    assert(!r5.isValid);

    std::cout << "RegexParser tests passed!\n";
    return 0;
}
