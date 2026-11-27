// ============================================================
//  Queue.h  -  A QUEUE (First In, First Out) built with a linked list
//
//  It is a template, so ONE class works for any data type:
//     Queue<Parcel*>  -> Module 2: normal delivery requests (arrival order)
//     Queue<int>      -> Module 6: BFS traversal of the city network
//
//  front -> [first] -> [second] -> [third] -> NULL
//                                     ^
//                                    rear
//  enqueue adds at the rear, dequeue removes from the front.
// ============================================================
#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
using namespace std;

template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
    };
    Node* frontNode;    // oldest item (leaves first)
    Node* rearNode;     // newest item (last in line)
    int count;

public:
   
    Queue() {
        frontNode = NULL;
        rearNode = NULL;
        count = 0;
    }

    ~Queue() {
        clear();
    }

    // Adds an item at the rear (end of the line).
    void enqueue(const T& value) {
        Node* n = new Node;
        n->data = value;
        n->next = NULL;
        if (rearNode == NULL) {         // queue was empty
            frontNode = n;
            rearNode = n;
        } else {
            rearNode->next = n;
            rearNode = n;
        }
        count++;
    }

  
    // Removes the front item and gives it back in 'value'.
    // Returns false (and changes nothing) when the queue is empty.
    bool dequeue(T& value) {
        if (frontNode == NULL) return false;
        Node* temp = frontNode;
        value = temp->data;
        frontNode = frontNode->next;
        if (frontNode == NULL) rearNode = NULL;     // queue became empty
        delete temp;
        count--;
        return true;
    }

    // Looks at the front item without removing it.
    bool peek(T& value) {
        if (frontNode == NULL) return false;
        value = frontNode->data;
        return true;
    }

    // Special function used ONLY by "Undo": it puts an item back at the FRONT,
    // because that is where the parcel was before it was processed.
    void enqueueFront(const T& value) {
        Node* n = new Node;
        n->data = value;
        n->next = frontNode;
        frontNode = n;
        if (rearNode == NULL) rearNode = n;
        count++;
    }

    bool isEmpty() { return frontNode == NULL; }
    int size() { return count; }

    // Removes every item.
    void clear() {
        T ignored;
        while (dequeue(ignored)) {
        }
    }
};

#endif
