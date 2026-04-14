#ifndef DEPARTMENT_INVENTORY_H
#define DEPARTMENT_INVENTORY_H

#include "LinkedList.h"
#include "MaterialItem.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <functional> 
#include <cctype>    

// This class stores and manages inventory for ONE department.
class DepartmentInventory {
private:
    std::string deptName;            // Department name (Offset Press, Label, Bindery)
    LinkedList<MaterialItem> items;  // Linked list of items in this department

    // Convert a string to uppercase so comparisons are easier
    static std::string upper(std::string s) {
        for (char& c : s) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        return s;
    }

public:
    // Create a department with a name
    explicit DepartmentInventory(const std::string& name) : deptName(name) {}

    // Return department name
    const std::string& name() const { return deptName; }

    // Add a new item to the linked list
    void addItem(const MaterialItem& m) { items.pushBack(m); }

    // Find an item by ID (returns pointer or nullptr)
    MaterialItem* findById(int id) {
        return items.findIf([id](MaterialItem& m) { return m.id == id; });
    }

    // Find an item by exact name (case-insensitive)
    MaterialItem* findByNameExact(const std::string& nm) {
        const std::string target = upper(MaterialItem::trim(nm));
        return items.findIf([&](MaterialItem& m) {
            return upper(m.name) == target;
        });
    }

    // Print all items in this department
    void listAll() const {
        std::cout << "\nDepartment: " << deptName << "\n";

        // Print table header
        std::cout << std::left
                  << std::setw(6)  << "ID"
                  << std::setw(10) << "Active"
                  << std::setw(12) << "Unit"
                  << std::setw(12) << "Qty"
                  << std::setw(12) << "Reorder"
                  << "Name\n";

        std::cout << std::string(70, '-') << "\n";

        // Print each item
        items.forEach([](const MaterialItem& m) {
            std::cout << std::left
                      << std::setw(6)  << m.id
                      << std::setw(10) << (m.isActive ? "Yes" : "No")
                      << std::setw(12) << m.unit
                      << std::setw(12) << m.quantityOnHand
                      << std::setw(12) << m.reorderLevel
                      << m.name << "\n";
        });

        std::cout << "\n";
    }

    // Sort items by name using merge sort (linked list sort)
    void sortByName() {
        items.sort([this](const MaterialItem& a, const MaterialItem& b) {
            return upper(a.name) < upper(b.name);
        });
    }

    // Sort items by quantity using merge sort (linked list sort)
    void sortByQuantity() {
        items.sort([](const MaterialItem& a, const MaterialItem& b) {
            return a.quantityOnHand < b.quantityOnHand;
        });
    }

    // Run a function on every item (read-only)
    void forEach(const std::function<void(const MaterialItem&)>& fn) const {
        items.forEach(fn);
    }
};

#endif
