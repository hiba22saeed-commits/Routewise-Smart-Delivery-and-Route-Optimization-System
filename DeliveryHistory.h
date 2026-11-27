// ============================================================
//  DeliveryHistory.h  -  Module 3: delivery history in a DOUBLY LINKED LIST
//
//  NULL <- [oldest] <-> [older] <-> [newest] -> NULL
//            head                    tail
//
//  Every parcel that is processed is added at the END (tail).
//  Why doubly linked?  Each node knows its previous node, so
//     * "Undo" can remove the newest record quickly (no need to walk from the head)
//     * the history can be shown from the newest record to the oldest one.
//  The list stores only POINTERS to parcels (the parcels belong to the hash table).
// ============================================================
#ifndef DELIVERY_HISTORY_H
#define DELIVERY_HISTORY_H

#include <iostream>
#include "Models.h"
using namespace std;

// ##########################################################
// #  MEMBER 3  :  addAtEnd, HistoryNode struct, constructor, destructor
// ##########################################################
struct HistoryNode {
    Parcel* parcel;
    HistoryNode* prev;      // older record
    HistoryNode* next;      // newer record
};

class DeliveryHistory {
private:
    HistoryNode* head;      // oldest record
    HistoryNode* tail;      // newest record
    int count;

public:
    DeliveryHistory() {
        head = NULL;
        tail = NULL;
        count = 0;
    }

    ~DeliveryHistory() {
        while (head != NULL) {
            HistoryNode* temp = head;
            head = head->next;
            delete temp;
        }
    }

    // Adds a parcel as the newest record.
    void addAtEnd(Parcel* p) {
        HistoryNode* node = new HistoryNode;
        node->parcel = p;
        node->next = NULL;
        node->prev = tail;
        if (tail == NULL) head = node;      // first record
        else tail->next = node;
        tail = node;
        count++;
    }

    // ##########################################################
    // #  MEMBER 1  :  removeLast, getCount
    // ##########################################################
    // Removes the newest record (used by Undo). Returns the parcel, or NULL if the list is empty.
    Parcel* removeLast() {
        if (tail == NULL) return NULL;
        HistoryNode* node = tail;
        Parcel* p = node->parcel;
        tail = node->prev;                  // the second newest record becomes the newest
        if (tail == NULL) head = NULL;      // the list is empty now
        else tail->next = NULL;
        delete node;
        count--;
        return p;
    }

    int getCount() { return count; }

    // ##########################################################
    // #  MEMBER 2  :  display (newest first)
    // ##########################################################
    // Shows the history, newest record first.
    void display() {
        if (tail == NULL) {
            cout << YELLOW<<"  The delivery history is empty (no delivery has been processed yet).\n"<<RESET;
            return;
        }
        cout <<CYAN<< "DELIVERY HISTORY (newest first) - " << count << " record(s)\n"<<RESET;
        cout <<YELLOW<< left << setw(5) << "No."<<RESET;
        printParcelHeader();
        int number = count;                 // the newest record has the biggest number
        for (HistoryNode* node = tail; node != NULL; node = node->prev) {
            cout << left << setw(5) << number;
            printParcelRow(node->parcel);
            number--;
        }
    }
};

#endif
