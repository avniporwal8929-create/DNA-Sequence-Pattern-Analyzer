#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <ctime>

#include "httplib.h"
#include "json.hpp"

#include "DNAValidator.h"
#include "RegexParser.h"
#include "ThompsonConstruction.h"
#include "SubsetConstruction.h"
#include "PatternDetector.h"
#include "SequenceComparator.h"

using json = nlohmann::json;

// Real Activity Tracker
struct ActivityStats {
    int totalSequencesAnalyzed = 0;
    int totalPatternsDetected = 0;
    int totalAutomataGenerated = 0;
    int totalMutationsCompared = 0;
    std::vector<json> activityHistory;
    std::mutex mtx;

    void record(const std::string& action, const std::string& details) {
        std::lock_guard<std::mutex> lock(mtx);
        auto now = std::chrono::system_clock::now();
        std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", std::localtime(&timeNow));

        json item;
        item["time"] = std::string(timeBuf);
        item["action"] = action;
        item["details"] = details;

        activityHistory.insert(activityHistory.begin(), item);
        if (activityHistory.size() > 50) {
            activityHistory.pop_back();
        }
    }

    json toJson() {
        std::lock_guard<std::mutex> lock(mtx);
        json j;
        j["totalSequencesAnalyzed"] = totalSequencesAnalyzed;
        j["totalPatternsDetected"] = totalPatternsDetected;
        j["totalAutomataGenerated"] = totalAutomataGenerated;
        j["totalMutationsCompared"] = totalMutationsCompared;
        j["history"] = activityHistory;
        return j;
    }
} gStats;

void setCors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    httplib::Server svr;

    // CORS preflight handler
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.status = 204;
    });

    // 1. Health check
    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        json j = {
            {"status", "ok"},
            {"service", "Automata-Based DNA Sequence Pattern Analyzer"},
            {"version", "1.0.0"},
            {"engine", "C++17 Theory of Computation Engine"}
        };
        res.set_content(j.dump(2), "application/json");
    });

    // 2. Real Stats
    svr.Get("/api/stats", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.set_content(gStats.toJson().dump(2), "application/json");
    });

    // 3. DNA Validation: POST /api/dna/validate
    svr.Post("/api/dna/validate", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string seq = body.value("sequence", "");
            ValidationResult v = DNAValidator::validate(seq);

            if (v.isValid) {
                gStats.totalSequencesAnalyzed++;
                gStats.record("Sequence Validation", "Validated sequence of length " + std::to_string(v.length));
            }

            res.set_content(v.toJson().dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"isValid", false}, {"errorMessage", std::string("Malformed JSON: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 4. Pattern Detection: POST /api/pattern/detect
    svr.Post("/api/pattern/detect", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string seq = body.value("sequence", "");
            std::string pat = body.value("pattern", "");

            PatternDetectionResult result = PatternDetector::detect(seq, pat);

            if (result.success) {
                gStats.totalPatternsDetected += result.totalOccurrences;
                gStats.totalAutomataGenerated += 2; // NFA and DFA created
                gStats.record("Pattern Search", "Pattern '" + pat + "' searched in sequence. Found: " + std::to_string(result.totalOccurrences));
            }

            res.set_content(result.toJson().dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"success", false}, {"errorMessage", std::string("Error during pattern detection: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 5. Regular Expression Compilation: POST /api/regex/compile
    svr.Post("/api/regex/compile", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string regexStr = body.value("regex", "");

            RegexParseResult parsed = RegexParser::parse(regexStr);
            if (!parsed.isValid) {
                json response = {
                    {"success", false},
                    {"errorMessage", parsed.errorMessage},
                    {"errorPosition", parsed.errorPosition},
                    {"parsed", parsed.toJson()}
                };
                res.set_content(response.dump(2), "application/json");
                return;
            }

            NFA nfa = ThompsonConstruction::buildFromPostfix(parsed.postfixRegex);
            SubsetConversionReport report = SubsetConstruction::convert(nfa);

            gStats.totalAutomataGenerated += 2;
            gStats.record("Regex Compilation", "Compiled regex: " + regexStr);

            json response;
            response["success"] = true;
            response["parsed"] = parsed.toJson();
            response["nfa"] = nfa.toJson();
            response["dfa"] = report.dfa.toJson();
            response["steps"] = report.toJson()["steps"];
            response["conversionTable"] = report.conversionTable;
            response["theoreticalExplanation"] = report.theoreticalExplanation;

            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"success", false}, {"errorMessage", std::string("Regex compilation error: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 6. NFA Generation: POST /api/automata/nfa
    svr.Post("/api/automata/nfa", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string input = body.value("input", "");
            std::string type = body.value("type", "regex"); // "regex" or "literal"

            NFA nfa;
            if (type == "literal") {
                ValidationResult v = DNAValidator::validate(input);
                if (!v.isValid) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"errorMessage", v.errorMessage}}).dump(2), "application/json");
                    return;
                }
                nfa = ThompsonConstruction::buildFromLiteral(v.sanitizedSequence);
            } else {
                nfa = ThompsonConstruction::buildFromRegex(input);
            }

            gStats.totalAutomataGenerated++;
            gStats.record("NFA Generation", "Generated NFA for " + type + " '" + input + "'");

            json response = {
                {"success", true},
                {"nfa", nfa.toJson()}
            };
            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"success", false}, {"errorMessage", std::string("NFA generation failed: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 7. NFA to DFA Subset Conversion: POST /api/automata/convert
    svr.Post("/api/automata/convert", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string input = body.value("input", "");
            std::string type = body.value("type", "regex"); // "regex" or "literal"

            NFA nfa;
            if (type == "literal") {
                ValidationResult v = DNAValidator::validate(input);
                if (!v.isValid) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"errorMessage", v.errorMessage}}).dump(2), "application/json");
                    return;
                }
                nfa = ThompsonConstruction::buildFromLiteral(v.sanitizedSequence);
            } else {
                nfa = ThompsonConstruction::buildFromRegex(input);
            }

            SubsetConversionReport report = SubsetConstruction::convert(nfa);

            gStats.totalAutomataGenerated++;
            gStats.record("NFA to DFA Conversion", "Converted NFA to DFA (" + std::to_string(report.dfa.getStates().size()) + " states)");

            json response;
            response["success"] = true;
            response["nfa"] = nfa.toJson();
            response["dfa"] = report.dfa.toJson();
            response["steps"] = report.toJson()["steps"];
            response["conversionTable"] = report.conversionTable;
            response["theoreticalExplanation"] = report.theoreticalExplanation;

            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"success", false}, {"errorMessage", std::string("Automata conversion failed: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 8. DFA Simulation: POST /api/automata/simulate
    svr.Post("/api/automata/simulate", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string input = body.value("input", "");
            std::string type = body.value("type", "regex");
            std::string sequence = body.value("sequence", "");

            ValidationResult valSeq = DNAValidator::validate(sequence);
            if (!valSeq.isValid && !sequence.empty()) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"errorMessage", valSeq.errorMessage}}).dump(2), "application/json");
                return;
            }

            std::string sanitizedSeq = valSeq.isValid ? valSeq.sanitizedSequence : "";

            NFA nfa;
            if (type == "literal") {
                ValidationResult v = DNAValidator::validate(input);
                if (!v.isValid) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"errorMessage", v.errorMessage}}).dump(2), "application/json");
                    return;
                }
                nfa = ThompsonConstruction::buildFromLiteral(v.sanitizedSequence);
            } else {
                nfa = ThompsonConstruction::buildFromRegex(input);
            }

            SubsetConversionReport report = SubsetConstruction::convert(nfa);
            DFA dfa = report.dfa;

            std::vector<DFASimulationStep> trace = dfa.simulateTrace(sanitizedSeq);
            bool isAccepted = dfa.accepts(sanitizedSeq);

            json traceJson = json::array();
            for (const auto& step : trace) {
                json s;
                s["stepNumber"] = step.stepNumber;
                s["currentState"] = step.currentState;
                s["currentStateLabel"] = step.currentStateLabel;
                s["currentSubset"] = step.currentSubset;
                s["inputSymbol"] = step.inputSymbol != '\0' ? std::string(1, step.inputSymbol) : "λ";
                s["nextState"] = step.nextState;
                s["nextStateLabel"] = step.nextStateLabel;
                s["nextSubset"] = step.nextSubset;
                s["transitionDescription"] = step.transitionDescription;
                s["isAccepted"] = step.isAccepted;
                traceJson.push_back(s);
            }

            gStats.record("DFA Simulation", "Simulated sequence '" + sanitizedSeq + "' (Accepted: " + (isAccepted ? "Yes" : "No") + ")");

            json response;
            response["success"] = true;
            response["isAccepted"] = isAccepted;
            response["inputSequence"] = sanitizedSeq;
            response["trace"] = traceJson;
            response["dfa"] = dfa.toJson();

            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"success", false}, {"errorMessage", std::string("DFA simulation failed: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 9. Sequence Comparator: POST /api/mutation/compare
    svr.Post("/api/mutation/compare", [](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            json body = json::parse(req.body);
            std::string seq1 = body.value("seq1", "");
            std::string seq2 = body.value("seq2", "");

            ComparisonResult result = SequenceComparator::compare(seq1, seq2);

            if (result.success) {
                gStats.totalMutationsCompared++;
                gStats.record("Mutation Comparison", "Compared sequences (" + std::to_string(result.totalSubstitutions) + " subs, " +
                                                      std::to_string(result.totalInsertions) + " ins, " +
                                                      std::to_string(result.totalDeletions) + " del)");
            }

            res.set_content(result.toJson().dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {{"success", false}, {"errorMessage", std::string("Sequence comparison error: ") + e.what()}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // Static files serving from frontend/
    svr.set_mount_point("/", "./frontend");

    std::cout << "========================================================\n";
    std::cout << "  DNA PATTERN ANALYZER & MUTATION DETECTION BACKEND  \n";
    std::cout << "  Language: C++17 | Framework: Modern REST API\n";
    std::cout << "  Port: " << port << " | Status: RUNNING\n";
    std::cout << "  Endpoint: http://localhost:" << port << "/\n";
    std::cout << "========================================================\n";

    svr.listen("0.0.0.0", port);

    return 0;
}
