#ifndef MATERIAL_ITEM_H
#define MATERIAL_ITEM_H

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

// This struct stores ONE material item in inventory.
struct MaterialItem {
    int id = 0;                 // Unique ID number
    std::string department;     // "Offset Press", "Label", "Bindery"
    std::string name;           // Description/name of material
    std::string unit;           // Unit (sheets, rolls, lbs, etc.)
    long long quantityOnHand = 0; // Current quantity
    long long reorderLevel   = 0; // Low-stock level
    bool isActive = true;         // True = usable, False = inactive

    // Split one line using '|' character
    static std::vector<std::string> splitPipe(const std::string& line) {
        std::vector<std::string> out;
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, '|')) {
            out.push_back(token);
        }
        return out;
    }

    // Trim spaces from the start/end of a string
    static std::string trim(const std::string& s) {
        const auto a = s.find_first_not_of(" \t\r\n");
        const auto b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return s.substr(a, b - a + 1);
    }

    // Convert a file line into a MaterialItem object
    // Format:
    // id|department|name|unit|quantityOnHand|reorderLevel|isActive
    static MaterialItem fromLine(const std::string& line) {
        auto parts = splitPipe(line);

        // require exactly 7 fields
        if (parts.size() != 7) {
            throw std::runtime_error("Invalid materials line (expected 7 fields).");
        }

        MaterialItem m;

        // Convert string fields into correct types
        m.id             = std::stoi(trim(parts[0]));
        m.department     = trim(parts[1]);
        m.name           = trim(parts[2]);
        m.unit           = trim(parts[3]);
        m.quantityOnHand = std::stoll(trim(parts[4]));
        m.reorderLevel   = std::stoll(trim(parts[5]));
        m.isActive       = (std::stoi(trim(parts[6])) != 0);

        return m;
    }

    // Convert this item back into one line for the file
    std::string toLine() const {
        std::ostringstream os;
        os << id << "|"
           << department << "|"
           << name << "|"
           << unit << "|"
           << quantityOnHand << "|"
           << reorderLevel << "|"
           << (isActive ? 1 : 0);
        return os.str();
    }
};

#endif
