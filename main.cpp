// ============================================================
//  RouteWise - Smart Delivery and Route Optimization System
//  CL2001 Data Structures Lab Project
//
//  main.cpp only shows the menu and calls the matching function of
//  RouteWiseSystem. All the real work is done in the header files.
//
//  Build:   g++ main.cpp -o RouteWise
//  Run:     ./RouteWise        (Windows: RouteWise.exe)
// ============================================================
// ##########################################################
// #  MEMBER 2  :  enableColors, includes
// ##########################################################
#include <iostream>
#include <string>
#include "Utils.h"
#include "RouteWiseSystem.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>      // only used to switch colours on in the Windows console
#endif
using namespace std;

// On Windows the console must be told to understand colour codes.
void enableColors() {
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(console, &mode)) {
        SetConsoleMode(console, mode | 0x0004);     // 0x0004 = ENABLE_VIRTUAL_TERMINAL_PROCESSING
    }
#endif
}

// ##########################################################
// #  MEMBER 3  :  showMenu, menuItem
// ##########################################################
// One menu line: the number in yellow, the text in green.
void menuItem(int number, const string& text) {
    cout << YELLOW << number << "." << GREEN << " " << text << RESET << "\n";
}

// firstTime = true : the welcome banner is shown above the menu (only once, at start-up)
// firstTime = false: only the title "ROUTEWISE MANAGEMENT SYSTEM" is shown above the menu
void showMenu(bool firstTime) {
    if (firstTime) {
        cout << CYAN << "=========================================\n";
        cout << "  Welcome to RouteWise\n";
        cout << "  Smart Delivery and Route Optimization\n";
        cout << "=========================================" << RESET << "\n";
    } else {
        cout << CYAN << "\n=========================================\n";
        cout << "ROUTEWISE DELIVERY SYSTEM\n";
        cout << "=========================================" << RESET << "\n";
    }
    menuItem(1, "Register Customer");
    menuItem(2, "Register Parcel");
    menuItem(3, "Search Parcel");
    menuItem(4, "Add Normal Delivery Request");
    menuItem(5, "Add Urgent Delivery Request");
    menuItem(6, "Process Next Delivery");
    menuItem(7, "Mark Parcel as Delivered");
    menuItem(8, "Undo Last Operation");
    menuItem(9, "Display Pending Deliveries");
    menuItem(10, "Display Parcel Records");
    menuItem(11, "Display City Network");
    menuItem(12, "BFS Network Traversal");
    menuItem(13, "DFS Network Traversal");
    menuItem(14, "Find Shortest Delivery Route");
    menuItem(15, "Generate Minimum Delivery Network");
    menuItem(16, "Compare BST and AVL Performance");
    menuItem(17, "Display System Statistics");
    menuItem(18, "Exit");
    cout << CYAN << "=========================================" << RESET << "\n";
}

// ##########################################################
// #  MEMBER 1  :  main() - start-up, menu loop, switch
// ##########################################################
int main() {
    RouteWiseSystem app;        // one object that holds every data structure

    enableColors();

    // Load customers, parcels and the city network from the 'data' folder automatically.
    // New records typed in the program are saved back to the same files.
    app.loadFromFiles("data");

    int choice = 0;
    bool firstTime = true;
    do {
        showMenu(firstTime);
        firstTime = false;
        choice = readIntInRange("Enter your choice (1-18): ", 1, 18);

        switch (choice) {
            case 1:  app.registerCustomer();          break;
            case 2:  app.registerParcel();            break;
            case 3:  app.searchParcel();              break;
            case 4:  app.addNormalRequest();          break;
            case 5:  app.addUrgentRequest();          break;
            case 6:  app.processNextDelivery();       break;
            case 7:  app.markDelivered();             break;
            case 8:  app.undoLastOperation();         break;
            case 9:  app.displayPendingDeliveries();  break;
            case 10: app.displayParcelRecords();      break;
            case 11: app.displayCityNetwork();        break;
            case 12: app.bfsTraversal();              break;
            case 13: app.dfsTraversal();              break;
            case 14: app.findShortestRoute();         break;
            case 15: app.generateMinimumNetwork();    break;
            case 16: app.compareBSTandAVL();          break;
            case 17: app.displayStatistics();         break;
            case 18: cout << GREEN << "\nThank you for using RouteWise. Goodbye!" << RESET << "\n"; break;
        }
    } while (choice != 18);

    return 0;       // 'app' is destroyed here and every destructor frees its memory
}
