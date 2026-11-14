#ifndef DNA_VALIDATOR_H
#define DNA_VALIDATOR_H

#include <string>
#include <vector>
#include "json.hpp"

struct ValidationResult {
    bool isValid = false;
    std::string sanitizedSequence;
    size_t length = 0;
    std::string errorMessage;
    int errorPosition = -1; // 1-based index
    char invalidChar = '\0';
    std::vector<std::string> alphabet = {"A", "T", "G", "C"};

    nlohmann::json toJson() const;
};

class DNAValidator {
public:
    static ValidationResult validate(const std::string& input);
    static bool isDNASymbol(char c);
};

#endif // DNA_VALIDATOR_H
