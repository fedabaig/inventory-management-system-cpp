#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

// This struct stores ONE transaction record.
// File format:
// timestamp|actionType|itemId|fromDepartment|toDepartment|quantity
struct Transaction {
    std::string timestamp;        // Date/time string
    std::string actionType;       // RECEIVE / USE / TRANSFER / UNDO_...
    int itemId = 0;               // ID of the item
    std::string fromDepartment;   // Empty for RECEIVE
    std::string toDepartment;     // Empty for USE
    long long quantity = 0;       // How much was moved/used/received

    // Split a line by the '|' character
    static std::vector<std::string> splitPipe(const std::string& line) {
        std::vector<std::string> out;
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, '|')) {
            out.push_back(token);
        }
        return out;
    }

    // Trim spaces from start/end of string
    static std::string trim(const std::string& s) {
        const auto a = s.find_first_not_of(" \t\r\n");
        const auto b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return s.substr(a, b - a + 1);
    }

    // Convert one file line into a Transaction object
    static Transaction fromLine(const std::string& line) {
        auto parts = splitPipe(line);

        // require exactly 6 fields
        if (parts.size() != 6) {
            throw std::runtime_error("Invalid transactions line (expected 6 fields).");
        }

        Transaction t;
        t.timestamp      = trim(parts[0]);
        t.actionType     = trim(parts[1]);
        t.itemId         = std::stoi(trim(parts[2]));
        t.fromDepartment = trim(parts[3]);
        t.toDepartment   = trim(parts[4]);
        t.quantity       = std::stoll(trim(parts[5]));

        return t;
    }

    // Convert this transaction back into one file line
    std::string toLine() const {
        std::ostringstream os;
        os << timestamp << "|"
           << actionType << "|"
           << itemId << "|"
           << fromDepartment << "|"
           << toDepartment << "|"
           << quantity;
        return os.str();
    }
};

#endif
