// ============================================================
//  Stack.h  -  A STACK (Last In, First Out) built with a linked list
//
//  It is a template, so ONE class works for any data type:
//     Stack<UndoRecord>  -> Module 3: undo of the last status change
//     Stack<int>         -> Module 6: DFS traversal of the city network
//
//  top -> [newest] -> [older] -> [oldest] -> NULL
//  push and pop both work at the top, so both are very fast.
// ============================================================
#ifndef STACK_H
#define STACK_H

#include <iostream>
using namespace std;

template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
    };
    Node* topNode;      // the newest item
    int count;          // number of items

public:
 
    Stack() {
        topNode = NULL;
        count = 0;
    }

    ~Stack() {
        clear();
    }

    // Puts a new item on top of the stack.
    void push(const T& value) {
        Node* n = new Node;
        n->data = value;
        n->next = topNode;      // the old top goes below the new item
        topNode = n;
        count++;
    }

    
    // Removes the top item and gives it back in 'value'.
    // Returns false (and changes nothing) when the stack is empty.
    bool pop(T& value) {
        if (topNode == NULL) return false;
        Node* temp = topNode;
        value = temp->data;
        topNode = topNode->next;
        delete temp;
        count--;
        return true;
    }

    
    // Looks at the top item without removing it.
    bool peek(T& value) {
        if (topNode == NULL) return false;
        value = topNode->data;
        return true;
    }

    bool isEmpty() { return topNode == NULL; }
    int size() { return count; }

    // Removes every item.
    void clear() {
        T ignored;
        while (pop(ignored)) {
        }
    }
};

#endif
