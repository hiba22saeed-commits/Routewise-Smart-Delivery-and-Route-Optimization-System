// ============================================================
//  Models.h  -  The records (data) of RouteWise
//
//  Customer    : one customer (also acts as a node of the customer linked list)
//  Parcel      : one parcel. Every parcel is created ONCE with new and is owned
//                by the hash table. All other structures (queue, heap, trees,
//                history, undo stack) only keep a POINTER to the same parcel.
//  UndoRecord  : what the undo stack remembers about one status change
// ============================================================
#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <iomanip>
#include <string>
#include "Utils.h"
using namespace std;


// ---------- delivery status of a parcel ----------
const string STATUS_REGISTERED = "Registered";   // record created, no delivery request yet
const string STATUS_PENDING    = "Pending";      // waiting in the normal queue or in the urgent heap
const string STATUS_IN_TRANSIT = "In Transit";   // processed (dispatched), on its way
const string STATUS_DELIVERED  = "Delivered";    // delivery completed

// ---------- priority of a parcel: 1 = lowest ... 5 = highest ----------
const int MIN_PRIORITY = 1;
const int MAX_PRIORITY = 5;

// ---------- Customer ----------
struct Customer {
    int customerID;
    string name;
    string phone;
    string address;
    Customer* next;        // next customer in the linked list
};


// ---------- Parcel ----------
struct Parcel {
    int trackingID;        // unique number used for searching
    int customerID;        // owner of the parcel
    string source;         // where the parcel starts (a location of the city network)
    string destination;    // where the parcel must go (a location of the city network)
    double weight;         // in kg
    int priority;          // 1 (lowest) ... 5 (highest)
    string status;         // one of the STATUS_ texts above
    int arrivalNo;         // order in which the delivery request arrived (0 = no request yet)
};

// ---------- Undo ----------
const int OP_PROCESS = 1;  // a delivery was processed  (Pending    -> In Transit)
const int OP_DELIVER = 2;  // a parcel was delivered    (In Transit -> Delivered)

struct UndoRecord {
    int operation;             // OP_PROCESS or OP_DELIVER
    Parcel* parcel;            // the parcel that was changed
    string previousStatus;     // the status it had BEFORE the change
    bool wasUrgent;            // for OP_PROCESS: did it come from the urgent heap?
};

inline string statusColor(const string& status) {
    if (status == STATUS_PENDING) return YELLOW;
    if (status == STATUS_IN_TRANSIT) return CYAN;
    if (status == STATUS_DELIVERED) return GREEN;
    return "";
}
// ---------- Printing a parcel as one row of a table ----------
inline void printParcelHeader() {
    cout << YELLOW<<left << setw(9) << "Track ID" << setw(9) << "Cust ID"
         << setw(17) << "Source" << setw(17) << "Destination"
         << setw(7) << "Kg" << setw(5) << "Pri" << setw(5) << "Arr"
         << "Status" << RESET<<endl;
    printLine('-', 78);
}

inline void printParcelRow(const Parcel* p) {
    cout << left << setw(9) << p->trackingID << setw(9) << p->customerID
         << setw(17) << p->source << setw(17) << p->destination
         << fixed << setprecision(1) << setw(7) << p->weight
         << setw(5) << p->priority;
    if (p->arrivalNo == 0) cout << setw(5) << "-";
    else cout << setw(5) << p->arrivalNo;
        cout << statusColor(p->status) << p->status << RESET << endl;
}

#endif
