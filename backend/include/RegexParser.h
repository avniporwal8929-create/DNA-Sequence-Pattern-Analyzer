#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include <string>
#include <vector>
#include "json.hpp"

struct RegexParseResult {
    bool isValid = false;
    std::string originalRegex;
    std::string sanitizedRegex;
    std::string explicitConcatRegex;
    std::string postfixRegex;
    std::string errorMessage;
    int errorPosition = -1; // 1-based index

    nlohmann::json toJson() const;
};

class RegexParser {
public:
    static RegexParseResult parse(const std::string& inputRegex);

private:
    static bool isDNASymbol(char c);
    static int getPrecedence(char op);
    static std::string insertExplicitConcatenation(const std::string& regex, std::string& errorMsg, int& errPos);
    static std::string toPostfix(const std::string& explicitRegex, std::string& errorMsg, int& errPos);
};

#endif // REGEX_PARSER_H
