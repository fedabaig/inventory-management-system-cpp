#include "InventorySystem.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cctype>   

using std::cout;
using std::cin;
using std::endl;

// Department names used everywhere
static const std::string DEPT_OFFSET  = "Offset Press";
static const std::string DEPT_LABEL   = "Label";
static const std::string DEPT_BINDERY = "Bindery";

// Constructor: create 3 departments and start IDs at 1000
InventorySystem::InventorySystem()
: offset(DEPT_OFFSET),
  label(DEPT_LABEL),
  bindery(DEPT_BINDERY),
  nextMaterialId(1000) {}

// Load inventory + transactions from files
void InventorySystem::loadAll() {
    loadMaterials();
    loadTransactions();
}

// Remove spaces from start/end of a string
std::string InventorySystem::trim(const std::string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    auto b = s.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, b - a + 1);
}

// Make string uppercase (for case-insensitive compare)
std::string InventorySystem::upper(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

// Clear bad input and ignore the rest of the line
void InventorySystem::clearCinLine() {
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Read an int with range checking
int InventorySystem::readInt(const std::string& prompt, int minV, int maxV) {
    while (true) {
        cout << prompt;
        int x;
        if (!(cin >> x)) {
            clearCinLine();
            cout << "Invalid number. Try again.\n";
            continue;
        }
        clearCinLine();
        if (x < minV || x > maxV) {
            cout << "Out of range (" << minV << " to " << maxV << "). Try again.\n";
            continue;
        }
        return x;
    }
}

// Read a long long with range checking
long long InventorySystem::readLongLong(const std::string& prompt, long long minV, long long maxV) {
    while (true) {
        cout << prompt;
        long long x;
        if (!(cin >> x)) {
            clearCinLine();
            cout << "Invalid number. Try again.\n";
            continue;
        }
        clearCinLine();
        if (x < minV || x > maxV) {
            cout << "Out of range (" << minV << " to " << maxV << "). Try again.\n";
            continue;
        }
        return x;
    }
}

// Read a full line of text input
std::string InventorySystem::readLine(const std::string& prompt) {
    cout << prompt;
    std::string s;
    std::getline(cin, s);
    return trim(s);
}

// Make a timestamp string like "2025-12-01 10:30:00"
std::string InventorySystem::nowTimestamp() {
    auto tp = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return os.str();
}

// Convert menu choice to a department
DepartmentInventory& InventorySystem::deptByChoice(int c) {
    if (c == 1) return offset;
    if (c == 2) return label;
    return bindery;
}

// Find department object by its name
DepartmentInventory* InventorySystem::deptByName(const std::string& name) {
    if (name == DEPT_OFFSET) return &offset;
    if (name == DEPT_LABEL) return &label;
    if (name == DEPT_BINDERY) return &bindery;
    return nullptr;
}

// If item is below reorder level, add it to reorder queue
void InventorySystem::enqueueReorderIfNeeded(const MaterialItem& m) {
    if (m.isActive && m.quantityOnHand < m.reorderLevel) {
        reorderQueue.push(m.id);
    }
}

// ---------------- File I/O ----------------

// Create starter inventory if materials.txt does not exist
void InventorySystem::createStarterData() {
    MaterialItem a;
    a.id = nextMaterialId++;
    a.department = DEPT_OFFSET;
    a.name = "70# Gloss Text Paper - 28x40";
    a.unit = "sheets";
    a.quantityOnHand = 5000;
    a.reorderLevel = 1500;
    a.isActive = true;
    offset.addItem(a);

    MaterialItem b;
    b.id = nextMaterialId++;
    b.department = DEPT_OFFSET;
    b.name = "Black Ink";
    b.unit = "lbs";
    b.quantityOnHand = 120;
    b.reorderLevel = 40;
    b.isActive = true;
    offset.addItem(b);

    MaterialItem c;
    c.id = nextMaterialId++;
    c.department = DEPT_LABEL;
    c.name = "Label Stock - 4in Roll";
    c.unit = "rolls";
    c.quantityOnHand = 80;
    c.reorderLevel = 25;
    c.isActive = true;
    label.addItem(c);

    MaterialItem d;
    d.id = nextMaterialId++;
    d.department = DEPT_BINDERY;
    d.name = "Shrink Wrap";
    d.unit = "rolls";
    d.quantityOnHand = 18;
    d.reorderLevel = 10;
    d.isActive = true;
    bindery.addItem(d);

    saveMaterials();
}

// Load materials from materials.txt
void InventorySystem::loadMaterials() {
    std::ifstream in(MATERIALS_FILE);
    if (!in) {
        cout << "materials.txt not found. Creating starter data...\n";
        createStarterData();
        return;
    }

    std::string line;
    int maxId = nextMaterialId;

    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        try {
            MaterialItem m = MaterialItem::fromLine(line);
            if (m.id > maxId) maxId = m.id;

            DepartmentInventory* d = deptByName(m.department);
            if (d) d->addItem(m);
        } catch (...) {
            // Ignore bad lines
        }
    }

    nextMaterialId = maxId + 1;
}

// Save materials to materials.txt
void InventorySystem::saveMaterials() {
    std::ofstream out(MATERIALS_FILE);
    if (!out) throw std::runtime_error("Failed to open materials.txt for writing.");

    out << "# id|department|name|unit|quantityOnHand|reorderLevel|isActive\n";

    auto writeDept = [&](const DepartmentInventory& d) {
        d.forEach([&](const MaterialItem& m) {
            out << m.toLine() << "\n";
        });
    };

    writeDept(offset);
    writeDept(label);
    writeDept(bindery);
}

// Load transaction history from transactions.txt (if it exists)
void InventorySystem::loadTransactions() {
    std::ifstream in(TRANSACTIONS_FILE);
    if (!in) return;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        try {
            allTransactions.push_back(Transaction::fromLine(line));
        } catch (...) {
            // Ignore bad lines
        }
    }
}

// Append one transaction to transactions.txt
void InventorySystem::appendTransaction(const Transaction& t) {
    allTransactions.push_back(t);

    std::ofstream out(TRANSACTIONS_FILE, std::ios::app);
    if (!out) throw std::runtime_error("Failed to open transactions.txt for appending.");
    out << t.toLine() << "\n";
}

// ---------------- Transfer helper ----------------

// Find an item by ID in ANY department (used for transfer)
const MaterialItem* InventorySystem::findPrototypeById(int id) const {
    const MaterialItem* found = nullptr;

    offset.forEach([&](const MaterialItem& m){ if (!found && m.id == id) found = &m; });
    label.forEach([&](const MaterialItem& m){ if (!found && m.id == id) found = &m; });
    bindery.forEach([&](const MaterialItem& m){ if (!found && m.id == id) found = &m; });

    return found;
}

// If destination department does not have the item, create it with qty 0
void InventorySystem::ensureDestHasItem(DepartmentInventory& dest, int id, const MaterialItem& proto) {
    MaterialItem* inDest = dest.findById(id);
    if (inDest) return;

    MaterialItem clone = proto;
    clone.department = dest.name();
    clone.quantityOnHand = 0;
    dest.addItem(clone);
}

// ---------------- Operations ----------------

// Add a new material to a department
void InventorySystem::addMaterial(DepartmentInventory& dept) {
    MaterialItem m;
    m.id = nextMaterialId++;
    m.department = dept.name();

    m.name = readLine("Material name/description: ");
    if (m.name.empty()) throw std::runtime_error("Name cannot be empty.");

    m.unit = readLine("Unit of measure (e.g., sheets, rolls, lbs): ");
    if (m.unit.empty()) throw std::runtime_error("Unit cannot be empty.");

    m.quantityOnHand = readLongLong("Starting quantity: ", 0, 9'000'000'000LL);
    m.reorderLevel   = readLongLong("Reorder level: ", 0, 9'000'000'000LL);
    m.isActive = true;

    dept.addItem(m);
    cout << "Added item ID " << m.id << " to " << dept.name() << ".\n";
}

// Edit material info
void InventorySystem::editMaterial(DepartmentInventory& dept) {
    int id = readInt("Enter item ID to edit: ", 1, 2'000'000'000);
    MaterialItem* m = dept.findById(id);
    if (!m) throw std::runtime_error("Item not found in this department.");

    cout << "Current name: " << m->name << "\n";
    std::string newName = readLine("New name (blank = keep): ");
    if (!newName.empty()) m->name = newName;

    cout << "Current unit: " << m->unit << "\n";
    std::string newUnit = readLine("New unit (blank = keep): ");
    if (!newUnit.empty()) m->unit = newUnit;

    cout << "Current reorder level: " << m->reorderLevel << "\n";
    std::string rl = readLine("New reorder level (blank = keep): ");
    if (!rl.empty()) {
        long long newRL = std::stoll(rl);
        if (newRL < 0) throw std::runtime_error("Reorder level cannot be negative.");
        m->reorderLevel = newRL;
    }

    cout << "Updated.\n";
}

// Mark item inactive (do not delete)
void InventorySystem::deactivateMaterial(DepartmentInventory& dept) {
    int id = readInt("Enter item ID to deactivate: ", 1, 2'000'000'000);
    MaterialItem* m = dept.findById(id);
    if (!m) throw std::runtime_error("Item not found in this department.");
    m->isActive = false;
    cout << "Item " << id << " is now inactive.\n";
}

// Receive material (increase qty)
void InventorySystem::receiveMaterial(DepartmentInventory& dept) {
    int id = readInt("Enter item ID to receive: ", 1, 2'000'000'000);
    MaterialItem* m = dept.findById(id);
    if (!m) throw std::runtime_error("Item not found.");
    if (!m->isActive) throw std::runtime_error("Item is inactive.");

    long long qty = readLongLong("Quantity to receive (>0): ", 1, 9'000'000'000LL);
    m->quantityOnHand += qty;
    enqueueReorderIfNeeded(*m);

    Transaction t;
    t.timestamp = nowTimestamp();
    t.actionType = "RECEIVE";
    t.itemId = id;
    t.fromDepartment = "";
    t.toDepartment = dept.name();
    t.quantity = qty;

    undoStack.push(t);
    appendTransaction(t);

    cout << "Received. New qty: " << m->quantityOnHand << "\n";
}

// Use material (decrease qty)
void InventorySystem::useMaterial(DepartmentInventory& dept) {
    int id = readInt("Enter item ID to use: ", 1, 2'000'000'000);
    MaterialItem* m = dept.findById(id);
    if (!m) throw std::runtime_error("Item not found.");
    if (!m->isActive) throw std::runtime_error("Item is inactive.");

    long long qty = readLongLong("Quantity to use (>0): ", 1, 9'000'000'000LL);
    if (qty > m->quantityOnHand) throw std::runtime_error("Not enough quantity on hand.");

    m->quantityOnHand -= qty;
    enqueueReorderIfNeeded(*m);

    Transaction t;
    t.timestamp = nowTimestamp();
    t.actionType = "USE";
    t.itemId = id;
    t.fromDepartment = dept.name();
    t.toDepartment = "";
    t.quantity = qty;

    undoStack.push(t);
    appendTransaction(t);

    cout << "Used. New qty: " << m->quantityOnHand << "\n";
}

// Transfer material from one department to another
void InventorySystem::transferMaterial(DepartmentInventory& fromDept) {
    int id = readInt("Enter item ID to transfer: ", 1, 2'000'000'000);
    MaterialItem* from = fromDept.findById(id);
    if (!from) throw std::runtime_error("Item not found in this department.");
    if (!from->isActive) throw std::runtime_error("Item is inactive.");

    cout << "Transfer destination:\n"
         << "1) " << DEPT_OFFSET << "\n"
         << "2) " << DEPT_LABEL << "\n"
         << "3) " << DEPT_BINDERY << "\n";
    int c = readInt("Choose (1-3): ", 1, 3);
    DepartmentInventory& toDept = deptByChoice(c);

    if (toDept.name() == fromDept.name())
        throw std::runtime_error("Cannot transfer to the same department.");

    long long qty = readLongLong("Quantity to transfer (>0): ", 1, 9'000'000'000LL);
    if (qty > from->quantityOnHand)
        throw std::runtime_error("Not enough quantity on hand.");

    const MaterialItem* proto = findPrototypeById(id);
    if (!proto) throw std::runtime_error("Internal error: prototype not found.");
    ensureDestHasItem(toDept, id, *proto);

    MaterialItem* to = toDept.findById(id);
    if (!to) throw std::runtime_error("Internal error: destination record missing.");
    if (!to->isActive) throw std::runtime_error("Destination record is inactive.");

    from->quantityOnHand -= qty;
    to->quantityOnHand += qty;

    enqueueReorderIfNeeded(*from);
    enqueueReorderIfNeeded(*to);

    Transaction t;
    t.timestamp = nowTimestamp();
    t.actionType = "TRANSFER";
    t.itemId = id;
    t.fromDepartment = fromDept.name();
    t.toDepartment = toDept.name();
    t.quantity = qty;

    undoStack.push(t);
    appendTransaction(t);

    cout << "Transferred.\n";
}

// Undo the last action (receive/use/transfer)
void InventorySystem::undoLast() {
    if (undoStack.empty()) {
        cout << "Nothing to undo.\n";
        return;
    }

    Transaction last = undoStack.pop();

    if (last.actionType == "RECEIVE") {
        DepartmentInventory* d = deptByName(last.toDepartment);
        if (!d) throw std::runtime_error("Undo failed: dept missing.");
        MaterialItem* m = d->findById(last.itemId);
        if (!m) throw std::runtime_error("Undo failed: item missing.");
        if (last.quantity > m->quantityOnHand) throw std::runtime_error("Undo failed: would go negative.");

        m->quantityOnHand -= last.quantity;

        Transaction u;
        u.timestamp = nowTimestamp();
        u.actionType = "UNDO_RECEIVE";
        u.itemId = last.itemId;
        u.fromDepartment = last.toDepartment;
        u.toDepartment = "";
        u.quantity = last.quantity;
        appendTransaction(u);

        cout << "Undo: reversed RECEIVE.\n";
    }
    else if (last.actionType == "USE") {
        DepartmentInventory* d = deptByName(last.fromDepartment);
        if (!d) throw std::runtime_error("Undo failed: dept missing.");
        MaterialItem* m = d->findById(last.itemId);
        if (!m) throw std::runtime_error("Undo failed: item missing.");

        m->quantityOnHand += last.quantity;

        Transaction u;
        u.timestamp = nowTimestamp();
        u.actionType = "UNDO_USE";
        u.itemId = last.itemId;
        u.fromDepartment = "";
        u.toDepartment = last.fromDepartment;
        u.quantity = last.quantity;
        appendTransaction(u);

        cout << "Undo: reversed USE.\n";
    }
    else if (last.actionType == "TRANSFER") {
        DepartmentInventory* fromD = deptByName(last.fromDepartment);
        DepartmentInventory* toD   = deptByName(last.toDepartment);
        if (!fromD || !toD) throw std::runtime_error("Undo failed: dept missing.");

        MaterialItem* from = fromD->findById(last.itemId);
        MaterialItem* to   = toD->findById(last.itemId);
        if (!from || !to) throw std::runtime_error("Undo failed: item missing.");
        if (last.quantity > to->quantityOnHand) throw std::runtime_error("Undo failed: dest too low.");

        to->quantityOnHand -= last.quantity;
        from->quantityOnHand += last.quantity;

        Transaction u;
        u.timestamp = nowTimestamp();
        u.actionType = "UNDO_TRANSFER";
        u.itemId = last.itemId;
        u.fromDepartment = last.toDepartment;
        u.toDepartment = last.fromDepartment;
        u.quantity = last.quantity;
        appendTransaction(u);

        cout << "Undo: reversed TRANSFER.\n";
    }
    else {
        throw std::runtime_error("Undo failed: unknown action type.");
    }
}

// ---------------- Reports ----------------

// Print low stock items
void InventorySystem::lowStockReport() const {
    cout << "\n=== LOW STOCK REPORT (Qty < Reorder) ===\n";

    auto printDept = [&](const DepartmentInventory& d) {
        bool any = false;

        d.forEach([&](const MaterialItem& m) {
            if (m.isActive && m.quantityOnHand < m.reorderLevel) any = true;
        });

        if (!any) return;

        cout << "\nDepartment: " << d.name() << "\n";
        cout << std::left
             << std::setw(6)  << "ID"
             << std::setw(12) << "Unit"
             << std::setw(12) << "Qty"
             << std::setw(12) << "Reorder"
             << "Name\n";
        cout << std::string(60, '-') << "\n";

        d.forEach([&](const MaterialItem& m) {
            if (m.isActive && m.quantityOnHand < m.reorderLevel) {
                cout << std::left
                     << std::setw(6)  << m.id
                     << std::setw(12) << m.unit
                     << std::setw(12) << m.quantityOnHand
                     << std::setw(12) << m.reorderLevel
                     << m.name << "\n";
            }
        });
    };

    printDept(offset);
    printDept(label);
    printDept(bindery);
    cout << "\n";
}

// Search by ID or name
void InventorySystem::searchMaterial() {
    cout << "\nSearch by:\n1) ID\n2) Name (exact)\n";
    int c = readInt("Choice: ", 1, 2);

    if (c == 1) {
        int id = readInt("Enter ID: ", 1, 2'000'000'000);
        bool found = false;

        auto show = [&](const DepartmentInventory& d) {
            d.forEach([&](const MaterialItem& m) {
                if (m.id == id) {
                    if (!found) cout << "\nMatches:\n";
                    cout << "[" << d.name() << "] "
                         << m.id << " | " << (m.isActive ? "Active" : "Inactive")
                         << " | " << m.quantityOnHand << " " << m.unit
                         << " | Reorder: " << m.reorderLevel
                         << " | " << m.name << "\n";
                    found = true;
                }
            });
        };

        show(offset);
        show(label);
        show(bindery);

        if (!found) cout << "No match found.\n";
        cout << "\n";
    } else {
        std::string name = readLine("Enter name (exact): ");
        std::string target = upper(name);
        bool found = false;

        auto show = [&](const DepartmentInventory& d) {
            d.forEach([&](const MaterialItem& m) {
                if (upper(m.name) == target) {
                    if (!found) cout << "\nMatches:\n";
                    cout << "[" << d.name() << "] "
                         << m.id << " | " << (m.isActive ? "Active" : "Inactive")
                         << " | " << m.quantityOnHand << " " << m.unit
                         << " | Reorder: " << m.reorderLevel
                         << " | " << m.name << "\n";
                    found = true;
                }
            });
        };

        show(offset);
        show(label);
        show(bindery);

        if (!found) cout << "No match found.\n";
        cout << "\n";
    }
}

// Show all transactions for one item ID
void InventorySystem::transactionHistoryForItem() {
    int id = readInt("Enter item ID for history: ", 1, 2'000'000'000);
    cout << "\n=== TRANSACTION HISTORY for Item " << id << " ===\n";

    bool any = false;
    for (const auto& t : allTransactions) {
        if (t.itemId == id) {
            cout << t.timestamp << " | " << t.actionType
                 << " | from: " << (t.fromDepartment.empty() ? "-" : t.fromDepartment)
                 << " | to: "   << (t.toDepartment.empty() ? "-" : t.toDepartment)
                 << " | qty: "  << t.quantity << "\n";
            any = true;
        }
    }

    if (!any) cout << "No transactions found.\n";
    cout << "\n";
}

// Show reorder queue (optional feature)
void InventorySystem::showReorderQueue() {
    cout << "\n=== REORDER QUEUE (optional) ===\n";

    if (reorderQueue.empty()) {
        cout << "No reorder requests.\n\n";
        return;
    }

    int shown = 0;
    while (!reorderQueue.empty() && shown < 20) {
        cout << "Reorder request for item ID: " << reorderQueue.front() << "\n";
        reorderQueue.pop();
        ++shown;
    }

    if (!reorderQueue.empty()) cout << "(More requests remain...)\n";
    cout << "\n";
}

// ---------------- Menus ----------------

void InventorySystem::runDepartmentMenu(DepartmentInventory& dept) {
    // Print header ONCE when user enters this menu
    cout << "\n=============================================\n";
    cout << "Department Menu - " << dept.name() << "\n";  
    cout << "=============================================\n";

    while (true) {
        cout << "\n1) Add new material\n";
        cout << "2) Edit material\n";
        cout << "3) Deactivate material\n";
        cout << "4) Receive material\n";
        cout << "5) Use material\n";
        cout << "6) Transfer material\n";
        cout << "7) List materials\n";
        cout << "8) Sort by name (merge sort)\n";
        cout << "9) Sort by quantity (merge sort)\n";
        cout << "0) Back\n";

        int c = readInt("Choice: ", 0, 9);

        try {
            if (c == 0) return;
            if (c == 1) addMaterial(dept);
            else if (c == 2) editMaterial(dept);
            else if (c == 3) deactivateMaterial(dept);
            else if (c == 4) receiveMaterial(dept);
            else if (c == 5) useMaterial(dept);
            else if (c == 6) transferMaterial(dept);
            else if (c == 7) dept.listAll();
            else if (c == 8) { dept.sortByName(); cout << "Sorted by name.\n"; }
            else if (c == 9) { dept.sortByQuantity(); cout << "Sorted by quantity.\n"; }
        } catch (const std::exception& ex) {
            cout << "ERROR: " << ex.what() << "\n";
        }
    }
}

// Main program loop
void InventorySystem::run() {
    cout << "=============================================\n";
    cout << " MM Packaging Inventory Management System\n";
    cout << "=============================================\n";

    while (true) {
        cout << "\nMain Menu\n";
        cout << "1) Manage " << DEPT_OFFSET << "\n";
        cout << "2) Manage " << DEPT_LABEL << "\n";
        cout << "3) Manage " << DEPT_BINDERY << "\n";
        cout << "4) Low-stock report\n";
        cout << "5) Undo last transaction\n";
        cout << "6) Search material\n";
        cout << "7) Transaction history (by item ID)\n";
        cout << "8) Save\n";
        cout << "9) Show reorder queue (optional)\n";
        cout << "0) Save and Exit\n";

        int choice = readInt("Choice: ", 0, 9);

        try {
            if (choice == 0) {
                saveMaterials();
                cout << "Saved. Goodbye!\n";
                return;
            } else if (choice >= 1 && choice <= 3) {
                runDepartmentMenu(deptByChoice(choice));
            } else if (choice == 4) lowStockReport();
            else if (choice == 5) undoLast();
            else if (choice == 6) searchMaterial();
            else if (choice == 7) transactionHistoryForItem();
            else if (choice == 8) { saveMaterials(); cout << "Saved materials.txt\n"; }
            else if (choice == 9) showReorderQueue();
        } catch (const std::exception& ex) {
            cout << "ERROR: " << ex.what() << "\n";
        }
    }
}
