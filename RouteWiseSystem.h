// ============================================================
//  RouteWiseSystem.h  -  The "brain" of the application
//
//  This class owns ONE object of every data structure and connects them:
//
//   Module 1  customers      CustomerList       singly linked list
//             parcelTable    ParcelHashTable    hash table + linear probing (owns the parcels)
//   Module 2  normalQueue    Queue<Parcel*>     normal requests, first come first served
//             urgentHeap     UrgentHeap         urgent requests, highest priority first
//   Module 3  history        DeliveryHistory    doubly linked list
//             undoStack      Stack<UndoRecord>  undo of the last status change
//   Module 4  bstIndex       ParcelBST          ordered parcel index
//             avlIndex       ParcelAVL          balanced parcel index
//   Modules 5-8  network     Graph              roads, BFS, DFS, shortest path, MST
//
//  Every public function below belongs to one menu option of main.cpp.
//
//  Life of a parcel:
//     Registered --(menu 4 or 5)--> Pending --(menu 6)--> In Transit --(menu 7)--> Delivered
//  Undo (menu 8) reverses the last "Process" or "Deliver" step.
// ============================================================
#ifndef ROUTEWISE_SYSTEM_H
#define ROUTEWISE_SYSTEM_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>
#include "Utils.h"
#include "Models.h"
#include "CustomerList.h"
#include "ParcelHashTable.h"
#include "ParcelBST.h"
#include "ParcelAVL.h"
#include "Queue.h"
#include "Stack.h"
#include "UrgentHeap.h"
#include "DeliveryHistory.h"
#include "Graph.h"
using namespace std;

class RouteWiseSystem {
private:
    // ---------- the data structures of the project ----------
    CustomerList customers;
    ParcelHashTable parcelTable;
    Queue<Parcel*> normalQueue;
    UrgentHeap urgentHeap;
    DeliveryHistory history;
    Stack<UndoRecord> undoStack;
    ParcelBST bstIndex;
    ParcelAVL avlIndex;
    Graph network;
    string dataFolder;              // folder with network.txt, customers.txt, parcels.txt
    int nextArrivalNo;              // gives every delivery request its arrival number (1, 2, 3 ...)

    // ===============================================================
    //  Small private helpers
    // ===============================================================

   
    // true if the network has locations; otherwise prints a hint.
    bool checkNetworkNotEmpty() {
        if (network.getLocationCount() == 0) {
            cout << RED << "  The city network has no locations yet. Add locations first (menu option 11)\n"
                 << "  or load the sample data files when the program starts.\n" << RESET;
            return false;
        }
        return true;
    }

    // Asks for a location by NAME or by NUMBER. Returns its number, or -1 if the user just presses Enter.
    int askLocation(const string& prompt) {
        while (true) {
            string text = readLine(prompt);
            if (text.length() == 0) return -1;
            int index = -1;
            if (isWholeNumber(text)) {
                int number = atoi(text.c_str());
                if (number >= 0 && number < network.getLocationCount()) index = number;
            }
            if (index == -1) index = network.findLocation(text);
            if (index != -1) return index;
            cout << RED << "  Location not found. Type a name or a number from the list (Enter = cancel).\n" << RESET;
        }
    }

 
    // Creates a parcel with new and stores it in the hash table (owner) and in both trees.
    Parcel* addParcelRecord(int trackingID, int customerID, const string& source,
                            const string& destination, double weight, int priority) {
        Parcel* p = new Parcel;                 // dynamic memory
        p->trackingID = trackingID;
        p->customerID = customerID;
        p->source = source;
        p->destination = destination;
        p->weight = weight;
        p->priority = priority;
        p->status = STATUS_REGISTERED;
        p->arrivalNo = 0;

        if (!parcelTable.insert(p)) {           // tracking ID already used
            delete p;
            return NULL;
        }
        bstIndex.insert(trackingID, p);
        avlIndex.insert(trackingID, p);
        return p;
    }

    // Asks for a tracking ID and gives the parcel back only if it is newly registered
    // (so it can still get a delivery request).
    Parcel* askForNewRequestParcel() {
        int id = readPositiveInt("Tracking ID of the parcel: ");
        Parcel* p = parcelTable.search(id);
        if (p == NULL) {
            cout << RED << "  No parcel with tracking ID " << id << ". Register it first (menu option 2).\n" << RESET;
            return NULL;
        }
        if (p->status != STATUS_REGISTERED) {
            cout << RED << "  Parcel " << id << " already has the status '" << p->status
                 << "'. Only a newly registered parcel can get a delivery request.\n" << RESET;
            return NULL;
        }
        return p;
    }

    // Shows the shortest route of a parcel (from its source to its destination).
    void showRouteOfParcel(Parcel* p) {
        int from = network.findLocation(p->source);
        int to = network.findLocation(p->destination);
        if (from == -1 || to == -1) {
            cout << YELLOW << "  (No route shown: the source or the destination is not in the city network.)\n" << RESET;
            return;
        }
        int path[MAX_LOCATIONS];
        int pathLength = 0;
        int totalDistance = network.shortestPath(from, to, path, pathLength);
        if (totalDistance == NO_ROUTE) {
            cout << RED << "  No road route exists from " << p->source << " to " << p->destination << ".\n" << RESET;
            return;
        }
        network.printRoute(path, pathLength, totalDistance);
    }

 
    // The smallest possible height of a binary tree with n nodes.
    // (1 level holds 1 node, 2 levels hold 3, 3 levels hold 7, ...)
    int bestPossibleHeight(int n) {
        int height = 0;
        int capacity = 0;
        int levelSize = 1;
        while (capacity < n) {
            capacity = capacity + levelSize;
            levelSize = levelSize * 2;
            height++;
        }
        return height;
    }

    // Prints the comparison table of two trees that hold the same keys.
    void printTreeComparison(ParcelBST& bst, ParcelAVL& avl) {
        int best = bestPossibleHeight(bst.getCount());
        cout << YELLOW << left << setw(34) << "" << setw(10) << "BST" << setw(10) << "AVL" << RESET << endl;
        printLine('-', 54);
        cout << setw(34) << "Number of parcels (keys)" << setw(10) << bst.getCount() << setw(10) << avl.getCount() << endl;
        cout << GREEN << setw(34) << "Tree height (levels)" << setw(10) << bst.getHeight() << setw(10) << avl.getHeight() << RESET << endl;
        cout << setw(34) << "Smallest possible height" << setw(10) << best << setw(10) << best << endl;
        cout << fixed << setprecision(2);
        cout << setw(34) << "Average steps to find a key" << setw(10) << bst.averageSearchSteps()
             << setw(10) << avl.averageSearchSteps() << endl;
        cout << setw(34) << "Rotations used while inserting" << setw(10) << "none" << setw(10) << avl.getRotationCount() << endl;
        cout << setw(34) << "Follows the AVL balance rule?" << setw(10) << (bst.isBalanced() ? "YES" : "NO")
             << setw(10) << (avl.isBalanced() ? "YES" : "NO") << endl;
        cout << YELLOW << "\n  The BST is " << (bst.getHeight() - avl.getHeight()) << " level(s) taller than the AVL tree.\n" << RESET;
    }

    void printTreeStructures(ParcelBST& bst, ParcelAVL& avl) {
        cout << CYAN << "\nTREE DRAWINGS (the trees are turned 90 degrees)\n" << RESET;
        cout << "  Root = left edge.  " << GREEN << "/--" << RESET << " bigger key (above its parent)   "
             << GREEN << "\\--" << RESET << " smaller key (below its parent)\n\n";

        printLine('=', 72);
        cout << YELLOW << "  BST   |  " << bst.getCount() << " node(s)  |  height " << bst.getHeight() << " level(s)\n" << RESET;
        printLine('-', 72);
        bst.showStructure();
        printLine('=', 72);

        cout << endl;

        printLine('=', 72);
        cout << YELLOW << "  AVL   |  " << avl.getCount() << " node(s)  |  height " << avl.getHeight() << " level(s)\n" << RESET;
        printLine('-', 72);
        avl.showStructure();
        printLine('=', 72);
    }

    
    // Removes the BOM (hidden first characters that some editors add) and the spaces around a line.
    string cleanLine(string line, int lineNumber) {
        if (lineNumber == 1 && line.length() >= 3 && (unsigned char)line[0] == 0xEF
            && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
            line = line.substr(3);
        }
        return trim(line);
    }

    void reportSkipped(const string& path, int lineNumber, const string& reason) {
        cout << RED << "    " << path << ", line " << lineNumber << " skipped: " << reason << endl << RESET;
    }

    // ---------- file loading (network.txt, customers.txt, parcels.txt) ----------

    // Line formats:   LOCATION|name        ROAD|nameA|nameB|kilometres
    void loadNetworkFile(const string& path) {
        ifstream file(path.c_str());
        if (!file.is_open()) {
            cout << RED << "  Could not open " << path << " (city network not loaded).\n" << RESET;
            return;
        }
        string line;
        int lineNumber = 0, locationsAdded = 0, roadsAdded = 0, skipped = 0;
        while (getline(file, line)) {
            lineNumber++;
            line = cleanLine(line, lineNumber);
            if (line.length() == 0 || line[0] == '#') continue;       // empty line or comment

            string parts[4];
            int fields = splitText(line, '|', parts, 4);
            if (parts[0] == "LOCATION" && fields == 2) {
                if (network.addLocation(parts[1])) locationsAdded++;
                else { skipped++; reportSkipped(path, lineNumber, "location is empty, duplicate, or the network is full"); }
            } else if (parts[0] == "ROAD" && fields == 4) {
                int a = network.findLocation(parts[1]);
                int b = network.findLocation(parts[2]);
                if (a == -1 || b == -1) {
                    skipped++; reportSkipped(path, lineNumber, "unknown location in the road");
                } else if (!isWholeNumber(parts[3]) || !network.addRoad(a, b, atoi(parts[3].c_str()))) {
                    skipped++; reportSkipped(path, lineNumber, "bad length, same location twice, or the road already exists");
                } else {
                    roadsAdded++;
                }
            } else {
                skipped++; reportSkipped(path, lineNumber, "unknown line format");
            }
        }
        if (skipped > 0) cout << RED << "  " << path << ": " << skipped << " line(s) skipped.\n" << RESET;
    }

    // Line format:   customerID|name|phone|address
    void loadCustomerFile(const string& path) {
        ifstream file(path.c_str());
        if (!file.is_open()) {
            cout << RED << "  Could not open " << path << " (customers not loaded).\n" << RESET;
            return;
        }
        string line;
        int lineNumber = 0, added = 0, skipped = 0;
        while (getline(file, line)) {
            lineNumber++;
            line = cleanLine(line, lineNumber);
            if (line.length() == 0 || line[0] == '#') continue;

            string parts[4];
            int fields = splitText(line, '|', parts, 4);
            if (fields != 4 || !isWholeNumber(parts[0]) || atoi(parts[0].c_str()) <= 0
                || parts[1].length() == 0 || parts[2].length() == 0 || parts[3].length() == 0) {
                skipped++; reportSkipped(path, lineNumber, "wrong format");
            } else if (!customers.addCustomer(atoi(parts[0].c_str()), parts[1], parts[2], parts[3])) {
                skipped++; reportSkipped(path, lineNumber, "customer ID already exists");
            } else {
                added++;
            }
        }
        if (skipped > 0) cout << RED << "  " << path << ": " << skipped << " line(s) skipped.\n" << RESET;
    }

    // Line format:   trackingID|customerID|source|destination|weightKg|priority
    void loadParcelFile(const string& path) {
        ifstream file(path.c_str());
        if (!file.is_open()) {
            cout << RED << "  Could not open " << path << " (parcels not loaded).\n" << RESET;
            return;
        }
        string line;
        int lineNumber = 0, added = 0, skipped = 0;
        while (getline(file, line)) {
            lineNumber++;
            line = cleanLine(line, lineNumber);
            if (line.length() == 0 || line[0] == '#') continue;

            string parts[6];
            int fields = splitText(line, '|', parts, 6);
            if (fields != 6 || !isWholeNumber(parts[0]) || !isWholeNumber(parts[1])
                || !isDecimalNumber(parts[4]) || !isWholeNumber(parts[5])) {
                skipped++; reportSkipped(path, lineNumber, "wrong format");
                continue;
            }
            int trackingID = atoi(parts[0].c_str());
            int customerID = atoi(parts[1].c_str());
            double weight = atof(parts[4].c_str());
            int priority = atoi(parts[5].c_str());
            int source = network.findLocation(parts[2]);
            int destination = network.findLocation(parts[3]);

            if (trackingID <= 0 || weight <= 0 || priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
                skipped++; reportSkipped(path, lineNumber, "tracking ID, weight or priority is not valid");
            } else if (customers.findCustomer(customerID) == NULL) {
                skipped++; reportSkipped(path, lineNumber, "customer does not exist");
            } else if (source == -1 || destination == -1 || source == destination) {
                skipped++; reportSkipped(path, lineNumber, "source/destination unknown or the same");
            } else if (addParcelRecord(trackingID, customerID, network.getName(source),
                                       network.getName(destination), weight, priority) == NULL) {
                skipped++; reportSkipped(path, lineNumber, "tracking ID already exists");
            } else {
                added++;
            }
        }
        if (skipped > 0) cout << RED << "  " << path << ": " << skipped << " line(s) skipped.\n" << RESET;
    }

  
    // ---------- network display and editing (used by menu option 11) ----------

    void showNetwork() {
        cout << endl;
        if (network.getLocationCount() == 0) {
            cout << YELLOW << "  The network is empty. Add locations and roads with the options below.\n" << RESET;
            return;
        }
        network.displayLocationList();
        cout << endl;
        network.displayAdjacencyList();
        cout << endl;
        network.displayMatrix();
        cout << "\n  " << network.getLocationCount() << " location(s), "
             << network.getRoadCount() << " road(s).\n";
    }

    void addLocationInteractive() {
        string name = removeBars(readNonEmpty("  Name of the new location: "));
        if (network.findLocation(name) != -1) {
            cout << RED << "  This location already exists.\n" << RESET;
        } else if (network.getLocationCount() >= MAX_LOCATIONS) {
            cout << RED << "  The network is full (" << MAX_LOCATIONS << " locations).\n" << RESET;
        } else {
            network.addLocation(name);
            cout << GREEN << "  Location '" << name << "' added as number " << (network.getLocationCount() - 1) << ".\n" << RESET;
            saveAll();
        }
    }

    void addRoadInteractive() {
        if (network.getLocationCount() < 2) {
            cout << RED << "  Add at least two locations first.\n" << RESET;
            return;
        }
        network.displayLocationList();
        int a = askLocation("  First location (name or number): ");
        if (a == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }
        int b = askLocation("  Second location (name or number): ");
        if (b == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }
        if (a == b) {
            cout << RED << "  A road must connect two different locations.\n" << RESET;
            return;
        }
        if (network.hasRoad(a, b)) {
            cout << RED << "  There is already a road between these two locations.\n" << RESET;
            return;
        }
        int length = readIntInRange("  Road length in km (1 - 10000): ", 1, MAX_ROAD_LENGTH);
        network.addRoad(a, b, length);
        cout << GREEN << "  Road added: " << network.getName(a) << " <-> " << network.getName(b)
             << " (" << length << " km, two-way).\n" << RESET;
        saveAll();
    }

    // BFS and DFS use the same questions, so one function does both.
    void runTraversal(bool useBFS) {
        cout << CYAN << "\n--- " << (useBFS ? "BFS" : "DFS") << " Network Traversal ---\n" << RESET;
        if (!checkNetworkNotEmpty()) return;
        network.displayLocationList();
        int start = askLocation("Start location (name or number, Enter = cancel): ");
        if (start == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }
        int target = askLocation("Destination to check (name or number, Enter = skip): ");

        cout << YELLOW << "\nStart location: " << network.getName(start) << endl << RESET;
        bool reachable = false;
        if (useBFS) reachable = network.bfs(start, target);
        else reachable = network.dfs(start, target);

        if (target != -1) {
            cout << "\nReachability: " << network.getName(target)
                 << (reachable ? GREEN + " IS" : RED + " is NOT") << RESET << " reachable from " << network.getName(start) << ".\n";
        }
    }

public:
 
    RouteWiseSystem() {
        nextArrivalNo = 1;
        dataFolder = "data";
    }

    // ===============================================================
    //  Loading the data files (optional, at program start)
    // ===============================================================
    void loadFromFiles(const string& folder) {
        dataFolder = folder;                            // saving later uses the same folder
        parcelTable.setQuiet(true);                     // no "table grew" message while loading
        loadNetworkFile(folder + "/network.txt");       // locations and roads first
        loadCustomerFile(folder + "/customers.txt");    // then customers
        loadParcelFile(folder + "/parcels.txt");        // parcels last (they need customers and locations)
        parcelTable.setQuiet(false);
    }

    // ===============================================================
    //  Saving: the three data files are rewritten after every new record,
    //  so the next start loads everything again.
    //  (Only the records are saved: customers, parcels, locations, roads.
    //   Delivery requests, history and undo are not saved.)
    // ===============================================================
    void saveAll() {
        ofstream networkFile((dataFolder + "/network.txt").c_str());
        ofstream customerFile((dataFolder + "/customers.txt").c_str());
        ofstream parcelFile((dataFolder + "/parcels.txt").c_str());
        if (!networkFile.is_open() || !customerFile.is_open() || !parcelFile.is_open()) {
            cout << RED << "  WARNING: could not save. Does the folder '" << dataFolder << "' exist?\n" << RESET;
            return;
        }
        networkFile << "# RouteWise - city network (saved automatically)\n";
        network.saveTo(networkFile);
        customerFile << "# RouteWise - customers (saved automatically)\n";
        customers.saveTo(customerFile);
        parcelFile << "# RouteWise - parcels (saved automatically)\n";
        parcelTable.saveTo(parcelFile);
        cout << GREEN << "  (Saved to the '" << dataFolder << "' folder.)\n" << RESET;
    }

    // ===============================================================
    //  Menu 1 - Register Customer
    // ===============================================================
    void registerCustomer() {
        cout << CYAN << "\n--- Register Customer ---\n" << RESET;
        int id = readPositiveInt("Customer ID (a positive number): ");
        if (customers.findCustomer(id) != NULL) {
            cout << RED << "  A customer with ID " << id << " already exists.\n" << RESET;
            return;
        }
        string name = removeBars(readNonEmpty("Name: "));
        string phone = removeBars(readNonEmpty("Phone number: "));
        string address = removeBars(readNonEmpty("Address: "));
        customers.addCustomer(id, name, phone, address);
        cout << GREEN << "  Customer '" << name << "' registered with ID " << id << ".\n" << RESET;
        saveAll();
    }

    // ===============================================================
    //  Menu 2 - Register Parcel
    // ===============================================================
    void registerParcel() {
        cout << CYAN << "\n--- Register Parcel ---\n" << RESET;
        if (!checkNetworkNotEmpty()) return;
        if (customers.getCount() == 0) {
            cout << RED << "  There are no customers yet. Register a customer first (menu option 1).\n" << RESET;
            return;
        }

        int trackingID = readPositiveInt("Tracking ID (a positive number): ");
        if (parcelTable.search(trackingID) != NULL) {
            cout << RED << "  A parcel with tracking ID " << trackingID << " already exists.\n" << RESET;
            return;
        }
        int customerID = readPositiveInt("Customer ID: ");
        Customer* owner = customers.findCustomer(customerID);
        if (owner == NULL) {
            cout << RED << "  Customer " << customerID << " was not found. Register the customer first (menu option 1).\n" << RESET;
            return;
        }

        network.displayLocationList();
        int source = askLocation("Source location (name or number): ");
        if (source == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }
        int destination = askLocation("Destination location (name or number): ");
        if (destination == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }
        if (source == destination) {
            cout << RED << "  The source and the destination must be different locations.\n" << RESET;
            return;
        }
        double weight = readPositiveDouble("Weight in kg: ");
        int priority = readIntInRange("Priority (1 = lowest ... 5 = highest): ", MIN_PRIORITY, MAX_PRIORITY);

        Parcel* p = addParcelRecord(trackingID, customerID, network.getName(source),
                                    network.getName(destination), weight, priority);
        if (p == NULL) {
            cout << RED << "  The parcel could not be stored.\n" << RESET;
            return;
        }
        cout << GREEN << "\n  Parcel registered for " << owner->name << ".\n\n" << RESET;
        printParcelHeader();
        printParcelRow(p);
        saveAll();
    }

    // ===============================================================
    //  Menu 3 - Search Parcel  (hash table)
    // ===============================================================
    void searchParcel() {
        cout << CYAN << "\n--- Search Parcel (hash table lookup) ---\n" << RESET;
        int id = readPositiveInt("Tracking ID: ");
        int probes = 0;
        int slot = -1;
        Parcel* p = parcelTable.search(id, probes, slot);
        if (p == NULL) {
            cout << RED << "  No parcel with tracking ID " << id << " was found (" << probes << " occupied slot(s) checked).\n" << RESET;
            return;
        }
        cout << GREEN << "  Found in hash table slot " << slot << " after " << probes << " probe(s).\n\n" << RESET;
        printParcelHeader();
        printParcelRow(p);
        Customer* owner = customers.findCustomer(p->customerID);
        if (owner != NULL) {
            cout << "\n  Customer: " << owner->name << " | " << owner->phone << " | " << owner->address << endl;
        }
    }

    // ===============================================================
    //  Menu 4 - Add Normal Delivery Request  (queue)
    // ===============================================================
    void addNormalRequest() {
        cout << CYAN << "\n--- Add Normal Delivery Request ---\n" << RESET;
        Parcel* p = askForNewRequestParcel();
        if (p == NULL) return;
        p->arrivalNo = nextArrivalNo;
        nextArrivalNo++;
        p->status = STATUS_PENDING;
        normalQueue.enqueue(p);
        cout << GREEN << "  Parcel " << p->trackingID << " joined the NORMAL queue (arrival no. " << p->arrivalNo
             << ", place " << normalQueue.size() << " in the queue).\n" << RESET;
    }

    // ===============================================================
    //  Menu 5 - Add Urgent Delivery Request  (heap)
    // ===============================================================
    void addUrgentRequest() {
        cout << CYAN << "\n--- Add Urgent Delivery Request ---\n" << RESET;
        Parcel* p = askForNewRequestParcel();
        if (p == NULL) return;
        p->arrivalNo = nextArrivalNo;
        nextArrivalNo++;
        p->status = STATUS_PENDING;
        urgentHeap.insert(p);
        cout << GREEN << "  Parcel " << p->trackingID << " was added to the URGENT heap (priority " << p->priority
             << ", arrival no. " << p->arrivalNo << ").\n" << RESET;
    }

    // ===============================================================
    //  Menu 6 - Process Next Delivery
    //  Urgent parcels go first (highest priority, then arrival order).
    //  When there is no urgent parcel, the normal queue is served.
    // ===============================================================
    void processNextDelivery() {
        cout << CYAN << "\n--- Process Next Delivery ---\n" << RESET;
        Parcel* p = NULL;
        bool urgent = false;
        if (urgentHeap.removeTop(p)) {
            urgent = true;
        } else if (!normalQueue.dequeue(p)) {
            cout << YELLOW << "  There are no pending deliveries.\n" << RESET;
            return;
        }

        // Remember the old state, so that "Undo" can restore it.
        UndoRecord record;
        record.operation = OP_PROCESS;
        record.parcel = p;
        record.previousStatus = p->status;
        record.wasUrgent = urgent;
        undoStack.push(record);

        p->status = STATUS_IN_TRANSIT;
        history.addAtEnd(p);

        if (urgent) cout << GREEN << "  Taken from the URGENT heap (priority " << p->priority << ").\n\n" << RESET;
        else cout << GREEN << "  Taken from the NORMAL queue (first come, first served).\n\n" << RESET;
        printParcelHeader();
        printParcelRow(p);
        cout << GREEN << "\n  Status changed: " << record.previousStatus << " -> " << p->status << "\n\n" << RESET;
        showRouteOfParcel(p);
    }

    // ===============================================================
    //  Menu 7 - Mark Parcel as Delivered
    // ===============================================================
    void markDelivered() {
        cout << CYAN << "\n--- Mark Parcel as Delivered ---\n" << RESET;
        int id = readPositiveInt("Tracking ID: ");
        Parcel* p = parcelTable.search(id);
        if (p == NULL) {
            cout << RED << "  No parcel with tracking ID " << id << ".\n" << RESET;
            return;
        }
        if (p->status != STATUS_IN_TRANSIT) {
            cout << RED << "  Only a parcel that is '" << STATUS_IN_TRANSIT << "' can be delivered. Parcel " << id
                 << " is '" << p->status << "'.\n" << RESET;
            if (p->status == STATUS_PENDING) cout << YELLOW << "  Process it first (menu option 6).\n" << RESET;
            return;
        }

        UndoRecord record;
        record.operation = OP_DELIVER;
        record.parcel = p;
        record.previousStatus = p->status;
        record.wasUrgent = false;
        undoStack.push(record);

        p->status = STATUS_DELIVERED;
        cout << GREEN << "  Parcel " << id << " is delivered.  Status changed: "
             << record.previousStatus << " -> " << p->status << endl << RESET;
    }

    // ===============================================================
    //  Menu 8 - Undo Last Operation  (stack)
    //  The newest record is on top of the stack, so the LAST change is undone first.
    // ===============================================================
    void undoLastOperation() {
        cout << CYAN << "\n--- Undo Last Operation ---\n" << RESET;
        UndoRecord record;
        if (!undoStack.pop(record)) {
            cout << YELLOW << "  Nothing to undo. (Undo remembers 'Process Next Delivery' and 'Mark Parcel as Delivered'.)\n" << RESET;
            return;
        }
        Parcel* p = record.parcel;
        string changedStatus = p->status;
        p->status = record.previousStatus;              // restore the previous state

        if (record.operation == OP_DELIVER) {
            cout << GREEN << "  Undone: the delivery of parcel " << p->trackingID << ".\n" << RESET;
        } else {
            history.removeLast();                       // the parcel was the newest history record
            if (record.wasUrgent) {
                urgentHeap.insert(p);                   // the heap puts it back in the right place
                cout << GREEN << "  Undone: the processing of parcel " << p->trackingID
                     << ". It is back in the URGENT heap.\n" << RESET;
            } else {
                normalQueue.enqueueFront(p);            // it was at the front of the queue before
                cout << GREEN << "  Undone: the processing of parcel " << p->trackingID
                     << ". It is back at the FRONT of the NORMAL queue.\n" << RESET;
            }
        }
        cout << GREEN << "  Status restored: " << changedStatus << " -> " << p->status << "\n\n" << RESET;
        printParcelHeader();
        printParcelRow(p);
    }

    // ===============================================================
    //  Menu 9 - Display Pending Deliveries
    // ===============================================================
    void displayPendingDeliveries() {
        cout << CYAN << "\n--- Pending Deliveries ---\n" << RESET;

        cout << CYAN << "\nURGENT deliveries (highest priority first, equal priority = arrival order): "
             << urgentHeap.size() << endl << RESET;
        if (urgentHeap.isEmpty()) {
            cout << YELLOW << "  none\n" << RESET;
        } else {
            // Copy the heap into a temporary heap and take the parcels out one by one.
            // The real heap is not touched.
            UrgentHeap tempHeap;
            for (int i = 0; i < urgentHeap.size(); i++) tempHeap.insert(urgentHeap.getAt(i));
            printParcelHeader();
            Parcel* p = NULL;
            while (tempHeap.removeTop(p)) printParcelRow(p);
        }

        cout << CYAN << "\nNORMAL deliveries (first come, first served): " << normalQueue.size() << endl << RESET;
        if (normalQueue.isEmpty()) {
            cout << YELLOW << "  none\n" << RESET;
        } else {
            // Show the queue without losing anything: take each parcel from the front,
            // print it and put it at the rear. After size() rounds the queue is as before.
            printParcelHeader();
            int howMany = normalQueue.size();
            for (int i = 0; i < howMany; i++) {
                Parcel* p = NULL;
                normalQueue.dequeue(p);
                printParcelRow(p);
                normalQueue.enqueue(p);
            }
        }
    }


    // ===============================================================
    //  Menu 10 - Display Parcel Records
    // ===============================================================
    void displayParcelRecords() {
        cout << CYAN << "\n--- Display Records ---\n" << RESET;
        cout << "  " << YELLOW << "1." << GREEN << " All parcels (sorted by tracking ID)" << RESET << "\n";
        cout << "  " << YELLOW << "2." << GREEN << " Delivery history (processed and delivered parcels)" << RESET << "\n";
        cout << "  " << YELLOW << "3." << GREEN << " All customers" << RESET << "\n";
        cout << "  " << YELLOW << "4." << GREEN << " Hash table layout (shows collisions and probing)" << RESET << "\n";
        int choice = readIntInRange("Your choice (1-4): ", 1, 4);
        cout << endl;

        if (choice == 1) {
            if (parcelTable.getCount() == 0) {
                cout << YELLOW << "  No parcels registered yet.\n" << RESET;
            } else {
                cout << CYAN << "ALL PARCEL RECORDS (sorted by tracking ID, read from the AVL tree) - "
                     << parcelTable.getCount() << " parcel(s)\n" << RESET;
                avlIndex.displayRecords();
            }
        } else if (choice == 2) {
            history.display();
        } else if (choice == 3) {
            customers.display();
        } else {
            parcelTable.displayLayout();
        }
    }

    // ===============================================================
    //  Menu 11 - Display City Network (and add locations / roads)
    // ===============================================================
    void displayCityNetwork() {
        cout << CYAN << "\n--- City Delivery Network ---\n" << RESET;
        showNetwork();
        while (true) {
            cout << "\n  " << YELLOW << "1." << GREEN << " Add a location" << RESET << "\n"
                 << "  " << YELLOW << "2." << GREEN << " Add a road" << RESET << "\n"
                 << "  " << YELLOW << "3." << GREEN << " Display the network again" << RESET << "\n"
                 << "  " << YELLOW << "0." << GREEN << " Back to the main menu" << RESET << "\n";
            int choice = readIntInRange("  Your choice (0-3): ", 0, 3);
            if (choice == 0) return;
            if (choice == 1) addLocationInteractive();
            else if (choice == 2) addRoadInteractive();
            else showNetwork();
        }
    }

    // ===============================================================
    //  Menus 12 and 13 - BFS and DFS
    // ===============================================================
    void bfsTraversal() { runTraversal(true); }
    void dfsTraversal() { runTraversal(false); }

    // ===============================================================
    //  Menu 14 - Find Shortest Delivery Route  (Dijkstra)
    // ===============================================================
    void findShortestRoute() {
        cout << CYAN << "\n--- Find Shortest Delivery Route ---\n" << RESET;
        if (!checkNetworkNotEmpty()) return;
        network.displayLocationList();
        int source = askLocation("Source (name or number, Enter = cancel): ");
        if (source == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }
        int destination = askLocation("Destination (name or number, Enter = cancel): ");
        if (destination == -1) { cout << YELLOW << "  Cancelled.\n" << RESET; return; }

        cout << YELLOW << "\nSource: " << network.getName(source) << endl << RESET;
        cout << YELLOW << "Destination: " << network.getName(destination) << endl << RESET;
        int path[MAX_LOCATIONS];
        int pathLength = 0;
        int totalDistance = network.shortestPath(source, destination, path, pathLength);
        if (totalDistance == NO_ROUTE) {
            cout << RED << "No route exists: these two locations are not connected by roads.\n" << RESET;
        } else {
            network.printRoute(path, pathLength, totalDistance);
        }
    }

    // ===============================================================
    //  Menu 15 - Generate Minimum Delivery Network  (Prim)
    // ===============================================================
    void generateMinimumNetwork() {
        cout << CYAN << "\n--- Generate Minimum Delivery Network (Minimum Spanning Tree) ---\n" << RESET;
        if (!checkNetworkNotEmpty()) return;
        network.minimumSpanningTree();
        cout << GREEN << "All roads of the city together: " << network.totalRoadLength() << " km\n" << RESET;
    }

    // ===============================================================
    //  Menu 16 - Compare BST and AVL Performance
    // ===============================================================
    void compareBSTandAVL() {
        cout << CYAN << "\n--- Compare BST and AVL Performance ---\n" << RESET;

        cout << CYAN << "\nPART 1: the parcels that are registered now\n\n" << RESET;
        if (parcelTable.getCount() == 0) {
            cout << YELLOW << "  No parcels registered yet.\n" << RESET;
        } else {
            printTreeComparison(bstIndex, avlIndex);
            if (parcelTable.getCount() <= 15) {
                printTreeStructures(bstIndex, avlIndex);
            }
            if (parcelTable.getCount() <= 30) {
                cout << CYAN << "\nTraversals (tracking IDs):\n" << RESET;
                cout << YELLOW << "  In-order   BST : " << RESET; bstIndex.showInorder();
                cout << YELLOW << "  In-order   AVL : " << RESET; avlIndex.showInorder();
                cout << YELLOW << "  Pre-order  BST : " << RESET; bstIndex.showPreorder();
                cout << YELLOW << "  Pre-order  AVL : " << RESET; avlIndex.showPreorder();
                cout << YELLOW << "  Post-order BST : " << RESET; bstIndex.showPostorder();
                cout << YELLOW << "  Post-order AVL : " << RESET; avlIndex.showPostorder();
            }
        }

        cout << CYAN << "\nPART 2: demonstration - IDs inserted in SORTED order (the worst case for a BST)\n" << RESET;
        int n = readIntInRange("How many sorted IDs (1, 2, 3, ...) should be inserted? (0 = skip, max 1000): ", 0, 1000);
        if (n == 0) return;

        ParcelBST demoBst;          // temporary trees, they hold only the numbers (no parcels)
        ParcelAVL demoAvl;
        for (int id = 1; id <= n; id++) {
            demoBst.insert(id, NULL);
            demoAvl.insert(id, NULL);
        }
        cout << GREEN << "\nThe IDs 1, 2, 3, ... " << n << " were inserted into both trees in this order.\n\n" << RESET;
        printTreeComparison(demoBst, demoAvl);
        if (n <= 15) printTreeStructures(demoBst, demoAvl);
    }

    // ===============================================================
    //  Menu 17 - Display System Statistics
    // ===============================================================
    void displayStatistics() {
        double loadFactor = (double)parcelTable.getCount() / parcelTable.getTableSize();
        cout << CYAN << "\n=========== SYSTEM STATISTICS ===========\n" << RESET;
        cout << YELLOW << "Customers registered          : " << GREEN << customers.getCount() << endl << RESET;
        cout << YELLOW << "Parcels registered            : " << GREEN << parcelTable.getCount() << endl << RESET;
        cout << YELLOW << "    Registered (no request)   : " << GREEN << parcelTable.countWithStatus(STATUS_REGISTERED) << endl << RESET;
        cout << YELLOW << "    Pending                   : " << GREEN << parcelTable.countWithStatus(STATUS_PENDING) << endl << RESET;
        cout << YELLOW << "    In Transit                : " << GREEN << parcelTable.countWithStatus(STATUS_IN_TRANSIT) << endl << RESET;
        cout << YELLOW << "    Delivered                 : " << GREEN << parcelTable.countWithStatus(STATUS_DELIVERED) << endl << RESET;
        cout << YELLOW << "Normal queue length           : " << GREEN << normalQueue.size() << endl << RESET;
        cout << YELLOW << "Urgent heap size              : " << GREEN << urgentHeap.size() << endl << RESET;
        cout << YELLOW << "Delivery history records      : " << GREEN << history.getCount() << endl << RESET;
        cout << YELLOW << "Undo steps available          : " << GREEN << undoStack.size() << endl << RESET;
        cout << YELLOW << "Locations in the network      : " << GREEN << network.getLocationCount() << endl << RESET;
        cout << YELLOW << "Roads in the network          : " << GREEN << network.getRoadCount() << endl << RESET;
        cout << YELLOW << "Hash table                    : " << GREEN << parcelTable.getCount() << " parcels in "
             << parcelTable.getTableSize() << " slots (load factor " << fixed << setprecision(2)
             << loadFactor << ")\n" << RESET;
        cout << YELLOW << "Tree height  BST / AVL        : " << GREEN << bstIndex.getHeight() << " / " << avlIndex.getHeight() << endl << RESET;
    }
};

#endif
