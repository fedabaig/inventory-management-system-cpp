/*
 * Program name: main.cpp
 * Author: Feda Bigzad
 * Date last updated: 12/01/2025
 * Purpose: Start and run the MM-IMS inventory program.
 */

#include "InventorySystem.h"
#include <iostream>
#include <exception> 

int main() {
    try {
        // Create the system
        InventorySystem sys;

        // Load files (materials and transactions)
        sys.loadAll();

        // Run the menu program
        sys.run();
    }
    catch (const std::exception& ex) {
        // If something crashes, show the error
        std::cout << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
