// ============================================================
//  ParcelHashTable.h  -  Module 1: fast parcel lookup by tracking ID
//
//  HASHING WITH LINEAR PROBING
//     * hash function : home slot = trackingID % tableSize
//     * collision     : if the home slot is taken, try the next slot,
//                       then the next one ... (wrap around at the end)
//     * search        : start at the home slot and move forward until the
//                       parcel is found or an EMPTY slot is reached
//     * the table is kept at most HALF full. When it gets fuller it grows
//       (new size = 2 * old size + 1) and every parcel is re-hashed.
//
//  This table is the OWNER of every Parcel: the parcels are created with
//  new (in RouteWiseSystem) and deleted here, in the destructor.
// ============================================================
#ifndef PARCEL_HASH_TABLE_H
#define PARCEL_HASH_TABLE_H

#include <iostream>
#include <iomanip>
#include "Models.h"
using namespace std;

class ParcelHashTable {
private:
    Parcel** slots;         // slots[i] points to a parcel, or is NULL when the slot is empty
    int tableSize;          // number of slots
    int count;              // number of parcels stored
    bool quiet;             // true = do not print the "table grew" message

    
    // The hash function: where should this tracking ID live?
    int homeSlot(int trackingID) {
        return trackingID % tableSize;
    }

    // Puts a parcel in the first free slot, starting at its home slot (linear probing).
    void place(Parcel* p) {
        int index = homeSlot(p->trackingID);
        while (slots[index] != NULL) {
            index = (index + 1) % tableSize;        // collision: try the next slot
        }
        slots[index] = p;
    }

    // Makes the table bigger and re-hashes every parcel into the new table.
    void grow() {
        int oldSize = tableSize;
        Parcel** oldSlots = slots;

        tableSize = tableSize * 2 + 1;
        slots = new Parcel*[tableSize];
        for (int i = 0; i < tableSize; i++) slots[i] = NULL;

        for (int i = 0; i < oldSize; i++) {
            if (oldSlots[i] != NULL) place(oldSlots[i]);
        }
        delete[] oldSlots;
        if (!quiet) {
            cout <<YELLOW<< "  [Hash table grew from " << oldSize << " to " << tableSize
                 << " slots and all parcels were re-hashed]\n"<<RESET;
        }
    }

public:
  
    ParcelHashTable() {
        tableSize = 11;
        count = 0;
        quiet = false;
        slots = new Parcel*[tableSize];
        for (int i = 0; i < tableSize; i++) slots[i] = NULL;
    }

    // Destructor: the table owns the parcels, so it deletes them.
    ~ParcelHashTable() {
        for (int i = 0; i < tableSize; i++) {
            if (slots[i] != NULL) delete slots[i];
        }
        delete[] slots;
    }

    // Looks for a tracking ID.
    //   probes = how many slots were checked, slot = where the parcel was found (-1 if not found)
    Parcel* search(int trackingID, int& probes, int& slot) {
        probes = 0;
        slot = -1;
        int index = homeSlot(trackingID);
        while (slots[index] != NULL) {              // an empty slot means: "not stored"
            probes++;
            if (slots[index]->trackingID == trackingID) {
                slot = index;
                return slots[index];
            }
            index = (index + 1) % tableSize;
        }
        return NULL;
    }

    // Short version when we do not need the probe information.
    Parcel* search(int trackingID) {
        int probes = 0;
        int slot = -1;
        return search(trackingID, probes, slot);
    }

    // Stores a parcel. Returns false if the tracking ID already exists.
    bool insert(Parcel* p) {
        if (search(p->trackingID) != NULL) return false;
        if ((count + 1) * 2 > tableSize) grow();    // keep the table at most half full
        place(p);
        count++;
        return true;
    }

 
    int getCount() { return count; }

    void setQuiet(bool value) { quiet = value; }

    // Writes every parcel as one line:  trackingID|customerID|source|destination|kg|priority
    void saveTo(ostream& out) {
        out << fixed << setprecision(2);
        for (int i = 0; i < tableSize; i++) {
            if (slots[i] != NULL) {
                Parcel* p = slots[i];
                out << p->trackingID << "|" << p->customerID << "|" << p->source << "|"
                    << p->destination << "|" << p->weight << "|" << p->priority << "\n";
            }
        }
    }
    int getTableSize() { return tableSize; }

    // How many parcels have this status?
    int countWithStatus(const string& status) {
        int total = 0;
        for (int i = 0; i < tableSize; i++) {
            if (slots[i] != NULL && slots[i]->status == status) total++;
        }
        return total;
    }

    // Shows every slot, so collisions and probing can be seen.
    void displayLayout() {
        cout <<CYAN<< "HASH TABLE LAYOUT  (" << count << " parcels in " << tableSize << " slots)\n"<<RESET;
        cout << CYAN <<"hash function: slot = trackingID % " << tableSize << "   |   collision rule: linear probing\n"<<RESET;
        printLine('-', 78);
        for (int i = 0; i < tableSize; i++) {
            cout << "Slot " << left << setw(4) << i << ": ";
            if (slots[i] == NULL) {
                cout << "(empty)";
            } else {
                cout <<GREEN<< "Parcel " << slots[i]->trackingID<<RESET;
                int home = homeSlot(slots[i]->trackingID);
                if (home != i) {
                    cout << YELLOW<<"   <- collision: its home slot is " << home << ", so it was moved forward"<<RESET;
                }
            }
            cout << endl;
        }
    }
};

#endif
