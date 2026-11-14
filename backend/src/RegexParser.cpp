#include "RegexParser.h"
#include <stack>
#include <cctype>

nlohmann::json RegexParseResult::toJson() const {
    nlohmann::json j;
    j["isValid"] = isValid;
    j["originalRegex"] = originalRegex;
    j["sanitizedRegex"] = sanitizedRegex;
    j["explicitConcatRegex"] = explicitConcatRegex;
    j["postfixRegex"] = postfixRegex;
    j["errorMessage"] = errorMessage;
    j["errorPosition"] = errorPosition;
    return j;
}

bool RegexParser::isDNASymbol(char c) {
    return (c == 'A' || c == 'T' || c == 'G' || c == 'C');
}

int RegexParser::getPrecedence(char op) {
    switch (op) {
        case '*': return 3;
        case '.': return 2;
        case '|': return 1;
        default: return 0;
    }
}

std::string RegexParser::insertExplicitConcatenation(const std::string& regex, std::string& errorMsg, int& errPos) {
    std::string result;
    result.reserve(regex.size() * 2);

    for (size_t i = 0; i < regex.size(); ++i) {
        char c1 = regex[i];
        result.push_back(c1);

        if (i + 1 < regex.size()) {
            char c2 = regex[i + 1];

            // Condition for inserting concatenation operator '.':
            // c1 can be: nucleotide, ')', or '*'
            // c2 can be: nucleotide, or '('
            bool c1CanPrecedeConcat = (isDNASymbol(c1) || c1 == ')' || c1 == '*');
            bool c2CanFollowConcat = (isDNASymbol(c2) || c2 == '(');

            if (c1CanPrecedeConcat && c2CanFollowConcat) {
                result.push_back('.');
            }
        }
    }

    return result;
}

std::string RegexParser::toPostfix(const std::string& explicitRegex, std::string& errorMsg, int& errPos) {
    std::string postfix;
    std::stack<char> opStack;

    for (size_t i = 0; i < explicitRegex.size(); ++i) {
        char c = explicitRegex[i];

        if (isDNASymbol(c)) {
            postfix.push_back(c);
        } else if (c == '(') {
            opStack.push(c);
        } else if (c == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            if (opStack.empty()) {
                errorMsg = "Mismatched closing parenthesis ')' at character position " + std::to_string(i + 1);
                errPos = static_cast<int>(i + 1);
                return "";
            }
            opStack.pop(); // Pop '('
        } else if (c == '*' || c == '.' || c == '|') {
            // Operator
            int prec = getPrecedence(c);
            while (!opStack.empty() && opStack.top() != '(' && getPrecedence(opStack.top()) >= prec) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(c);
        } else {
            errorMsg = "Unrecognized symbol '" + std::string(1, c) + "' in regular expression.";
            errPos = static_cast<int>(i + 1);
            return "";
        }
    }

    while (!opStack.empty()) {
        if (opStack.top() == '(') {
            errorMsg = "Mismatched opening parenthesis '(' without closing partner.";
            errPos = 1;
            return "";
        }
        postfix.push_back(opStack.top());
        opStack.pop();
    }

    return postfix;
}

RegexParseResult RegexParser::parse(const std::string& inputRegex) {
    RegexParseResult res;
    res.originalRegex = inputRegex;

    if (inputRegex.empty()) {
        res.isValid = false;
        res.errorMessage = "Regular expression cannot be empty.";
        res.errorPosition = 0;
        return res;
    }

    // 1. Sanitize: remove whitespace, convert nucleotides to uppercase
    std::string sanitized;
    sanitized.reserve(inputRegex.size());

    for (size_t i = 0; i < inputRegex.size(); ++i) {
        char c = inputRegex[i];
        if (std::isspace(static_cast<unsigned char>(c))) continue;

        char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        if (isDNASymbol(upper) || upper == '|' || upper == '*' || upper == '(' || upper == ')') {
            sanitized.push_back(upper);
        } else {
            res.isValid = false;
            res.errorPosition = static_cast<int>(i + 1);
            res.errorMessage = "Unsupported character '" + std::string(1, c) + "' at position " +
                               std::to_string(res.errorPosition) + ". Supported: DNA alphabet {A,T,G,C} and operators {'|', '*', '(', ')'}.";
            return res;
        }
    }

    if (sanitized.empty()) {
        res.isValid = false;
        res.errorMessage = "Regular expression contains only whitespace.";
        res.errorPosition = 0;
        return res;
    }

    res.sanitizedRegex = sanitized;

    // 2. Syntax validation checks
    // Check initial/terminal operators
    if (sanitized[0] == '*' || sanitized[0] == '|') {
        res.isValid = false;
        res.errorPosition = 1;
        res.errorMessage = "Regular expression cannot begin with operator '" + std::string(1, sanitized[0]) + "'.";
        return res;
    }
    if (sanitized.back() == '|') {
        res.isValid = false;
        res.errorPosition = static_cast<int>(sanitized.size());
        res.errorMessage = "Regular expression cannot end with union operator '|'.";
        return res;
    }

    // Check adjacent invalid operator sequences
    for (size_t i = 0; i < sanitized.size(); ++i) {
        char c = sanitized[i];
        if (c == '(' && i + 1 < sanitized.size() && sanitized[i + 1] == ')') {
            res.isValid = false;
            res.errorPosition = static_cast<int>(i + 1);
            res.errorMessage = "Empty parentheses '()' are not allowed.";
            return res;
        }
        if (c == '(' && i + 1 < sanitized.size() && (sanitized[i + 1] == '*' || sanitized[i + 1] == '|')) {
            res.isValid = false;
            res.errorPosition = static_cast<int>(i + 2);
            res.errorMessage = "Operator '" + std::string(1, sanitized[i + 1]) + "' cannot immediately follow '('.";
            return res;
        }
        if (c == '|' && i + 1 < sanitized.size() && (sanitized[i + 1] == '|' || sanitized[i + 1] == '*')) {
            res.isValid = false;
            res.errorPosition = static_cast<int>(i + 2);
            res.errorMessage = "Operator '" + std::string(1, sanitized[i + 1]) + "' cannot immediately follow '|'.";
            return res;
        }
    }

    // 3. Insert explicit concatenation
    std::string errorMsg;
    int errPos = -1;
    std::string explicitConcat = insertExplicitConcatenation(sanitized, errorMsg, errPos);
    if (!errorMsg.empty()) {
        res.isValid = false;
        res.errorMessage = errorMsg;
        res.errorPosition = errPos;
        return res;
    }
    res.explicitConcatRegex = explicitConcat;

    // 4. Convert to postfix notation via Shunting-Yard
    std::string postfix = toPostfix(explicitConcat, errorMsg, errPos);
    if (!errorMsg.empty()) {
        res.isValid = false;
        res.errorMessage = errorMsg;
        res.errorPosition = errPos;
        return res;
    }

    res.postfixRegex = postfix;
    res.isValid = true;
    res.errorMessage = "";
    return res;
}
