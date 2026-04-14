#ifndef INVENTORY_SYSTEM_H
#define INVENTORY_SYSTEM_H

#include "DepartmentInventory.h" // DepartmentInventory + MaterialItem
#include "Transaction.h"         // Transaction struct
#include "Stack.h"               // Custom stack for undo

#include <vector>   // For transaction history list
#include <queue>    // For reorder queue
#include <string>   // For strings

// This class runs the whole inventory program.
// It loads files, shows menus, and calls actions.
class InventorySystem {
public:
    InventorySystem(); // Set up the 3 departments and starting ID
    void loadAll();    // Load materials + transactions from files
    void run();        // Start the menu program

private:
    // 3 departments in the company
    DepartmentInventory offset;
    DepartmentInventory label;
    DepartmentInventory bindery;

    // Undo stack (last transaction)
    Stack<Transaction> undoStack;

    // Full transaction history in memory
    std::vector<Transaction> allTransactions;

    // Optional: queue for reorder requests (low stock)
    std::queue<int> reorderQueue;

    // Next ID to give to a new material item
    int nextMaterialId;

    // File names
    static constexpr const char* MATERIALS_FILE = "materials.txt";
    static constexpr const char* TRANSACTIONS_FILE = "transactions.txt";

private:
    // -------- File I/O --------
    void loadMaterials();                      // Read materials.txt
    void loadTransactions();                   // Read transactions.txt
    void saveMaterials();                      // Write materials.txt
    void appendTransaction(const Transaction& t); // Add one transaction line to file
    void createStarterData();                  // Create default data if file missing

    // -------- Menu helpers --------
    void runDepartmentMenu(DepartmentInventory& dept); // Menu for one department
    DepartmentInventory& deptByChoice(int c);          // Pick dept by number 1-3
    DepartmentInventory* deptByName(const std::string& name); // Find dept by name

    // -------- Actions / Operations --------
    void addMaterial(DepartmentInventory& dept);       // Add a new item
    void editMaterial(DepartmentInventory& dept);      // Edit name/unit/reorder level
    void deactivateMaterial(DepartmentInventory& dept);// Mark inactive

    void receiveMaterial(DepartmentInventory& dept);   // Increase quantity
    void useMaterial(DepartmentInventory& dept);       // Decrease quantity
    void transferMaterial(DepartmentInventory& dept);  // Move quantity to another dept

    void undoLast();                                   // Undo last transaction

    // -------- Reports / Queries --------
    void lowStockReport() const;        // Show items below reorder level
    void searchMaterial();              // Search by ID or name
    void transactionHistoryForItem();   // Show history for one item
    void showReorderQueue();            // Show reorder queue (optional)

    // -------- Small helper functions --------
    static std::string nowTimestamp();  // Current date/time string
    static void clearCinLine();         // Clear bad input
    static int readInt(const std::string& prompt, int minV, int maxV); // Read int safely
    static long long readLongLong(const std::string& prompt, long long minV, long long maxV); // Read long long safely
    static std::string readLine(const std::string& prompt); // Read a full line
    static std::string trim(const std::string& s);          // Trim spaces
    static std::string upper(std::string s);                // Uppercase string

    // -------- Transfer helpers --------
    const MaterialItem* findPrototypeById(int id) const; // Find item in any dept (used for transfer)
    void ensureDestHasItem(DepartmentInventory& dest, int id, const MaterialItem& proto); // Create item in dest if missing

    // Add to reorder queue if below reorder level
    void enqueueReorderIfNeeded(const MaterialItem& m);
};

#endif
