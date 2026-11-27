// ============================================================
//  UrgentHeap.h  -  Module 2: urgent deliveries in a MAX-HEAP
//
//  A heap is a binary tree stored inside an ARRAY:
//       parent of i      = (i - 1) / 2
//       left child of i  = 2 * i + 1
//       right child of i = 2 * i + 2
//  The parcel that must be served FIRST is always at index 0 (the top).
//
//  Which parcel is "better"?   (see hasHigherPriority)
//     1. the one with the higher priority number
//     2. if the priorities are equal, the one that arrived first
//
//  The heap stores only POINTERS to parcels (the parcels are owned by the hash table).
// ============================================================
#ifndef URGENT_HEAP_H
#define URGENT_HEAP_H

#include <iostream>
#include "Models.h"
using namespace std;

class UrgentHeap {
private:
    Parcel** data;      // dynamic array of parcel pointers
    int capacity;       // size of the array
    int count;          // how many parcels are stored

    // ##########################################################
    // #  MEMBER 1  :  hasHigherPriority, moveUp, swapItems
    // ##########################################################
    // true when parcel a must be served BEFORE parcel b
    bool hasHigherPriority(Parcel* a, Parcel* b) {
        if (a->priority != b->priority) {
            return a->priority > b->priority;       // bigger number = more urgent
        }
        return a->arrivalNo < b->arrivalNo;         // same priority: first come, first served
    }

    void swapItems(int i, int j) {
        Parcel* temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }

    // A new item is added at the bottom. Move it UP while it is better than its parent.
    void moveUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (hasHigherPriority(data[i], data[parent])) {
                swapItems(i, parent);
                i = parent;
            } else {
                break;
            }
        }
    }

    // ##########################################################
    // #  MEMBER 2  :  moveDown, grow, constructor, destructor
    // ##########################################################
    // After removing the top, the last item is put on top. Move it DOWN until the heap is correct.
    void moveDown(int i) {
        while (true) {
            int leftChild = 2 * i + 1;
            int rightChild = 2 * i + 2;
            int best = i;                           // the best of: node, left child, right child
            if (leftChild < count && hasHigherPriority(data[leftChild], data[best])) best = leftChild;
            if (rightChild < count && hasHigherPriority(data[rightChild], data[best])) best = rightChild;
            if (best == i) break;                   // the node is already better than its children
            swapItems(i, best);
            i = best;
        }
    }

    // The array is full: make a bigger one (twice the size) and copy the pointers.
    void grow() {
        int newCapacity = capacity * 2;
        Parcel** bigger = new Parcel*[newCapacity];
        for (int i = 0; i < count; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

public:
    UrgentHeap() {
        capacity = 10;
        count = 0;
        data = new Parcel*[capacity];
    }

    ~UrgentHeap() {
        delete[] data;          // only the array is deleted; the parcels belong to the hash table
    }

    // ##########################################################
    // #  MEMBER 3  :  insert, removeTop, peek, getAt, isEmpty, size
    // ##########################################################
    // Adds a parcel.
    void insert(Parcel* p) {
        if (count == capacity) grow();
        data[count] = p;                // put it at the bottom ...
        count++;
        moveUp(count - 1);              // ... and move it up to its place
    }

    // Removes the most urgent parcel and gives it back in 'p'. Returns false if the heap is empty.
    bool removeTop(Parcel*& p) {
        if (count == 0) return false;
        p = data[0];
        count--;
        data[0] = data[count];          // the last item goes to the top ...
        if (count > 0) moveDown(0);     // ... and sinks to its place
        return true;
    }

    // Looks at the most urgent parcel without removing it.
    Parcel* peek() {
        if (count == 0) return NULL;
        return data[0];
    }

    // Gives the parcel stored at an array position (used to copy the heap for display).
    Parcel* getAt(int index) { return data[index]; }

    bool isEmpty() { return count == 0; }
    int size() { return count; }
};

#endif
