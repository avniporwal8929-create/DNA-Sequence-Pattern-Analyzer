#include "DNAValidator.h"
#include <cctype>

nlohmann::json ValidationResult::toJson() const {
    nlohmann::json j;
    j["isValid"] = isValid;
    j["sanitizedSequence"] = sanitizedSequence;
    j["length"] = length;
    j["errorMessage"] = errorMessage;
    j["errorPosition"] = errorPosition;
    if (invalidChar != '\0') {
        j["invalidChar"] = std::string(1, invalidChar);
    } else {
        j["invalidChar"] = "";
    }
    j["alphabet"] = alphabet;
    return j;
}

bool DNAValidator::isDNASymbol(char c) {
    return (c == 'A' || c == 'T' || c == 'G' || c == 'C');
}

ValidationResult DNAValidator::validate(const std::string& input) {
    ValidationResult res;

    if (input.empty()) {
        res.isValid = false;
        res.errorMessage = "Input sequence is empty. Please enter a DNA sequence consisting of {A, T, G, C}.";
        res.errorPosition = 0;
        return res;
    }

    std::string sanitized;
    sanitized.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue; // Skip whitespace if user pasted formatted fasta/blocks
        }
        char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        if (!isDNASymbol(upper)) {
            res.isValid = false;
            res.invalidChar = ch;
            res.errorPosition = static_cast<int>(i + 1); // 1-based index
            res.errorMessage = "Invalid DNA nucleotide '" + std::string(1, ch) + "' encountered at position " +
                               std::to_string(res.errorPosition) + ". DNA alphabet strictly requires {A, T, G, C}.";
            return res;
        }
        sanitized.push_back(upper);
    }

    if (sanitized.empty()) {
        res.isValid = false;
        res.errorMessage = "Input contains only whitespace characters.";
        res.errorPosition = 0;
        return res;
    }

    res.isValid = true;
    res.sanitizedSequence = sanitized;
    res.length = sanitized.size();
    res.errorMessage = "";
    return res;
}
