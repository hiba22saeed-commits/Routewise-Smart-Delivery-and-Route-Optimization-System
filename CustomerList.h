// ============================================================
//  CustomerList.h  -  Module 1: customers stored in a SINGLY LINKED LIST
//
//  head -> [Customer] -> [Customer] -> [Customer] -> NULL
//  Every customer is created with new and deleted in the destructor.
//  New customers are added at the END (tail), so the list keeps the
//  order of registration.
// ============================================================
#ifndef CUSTOMER_LIST_H
#define CUSTOMER_LIST_H

#include <iostream>
#include <iomanip>
#include <string>
#include "Models.h"
#include "Utils.h"
using namespace std;

class CustomerList {
private:
    Customer* head;     // first customer
    Customer* tail;     // last customer (makes adding at the end quick)
    int count;          // how many customers

public:
   
    CustomerList() {
        head = NULL;
        tail = NULL;
        count = 0;
    }

    // Destructor: deletes every node, so no memory is leaked.
    ~CustomerList() {
        while (head != NULL) {
            Customer* temp = head;
            head = head->next;
            delete temp;
        }
    }

    // Returns the customer with this ID, or NULL if there is none.
    Customer* findCustomer(int id) {
        Customer* current = head;
        while (current != NULL) {
            if (current->customerID == id) return current;
            current = current->next;
        }
        return NULL;
    }

   
    // Adds a new customer at the end of the list. Returns false if the ID is already used.
    bool addCustomer(int id, const string& name, const string& phone, const string& address) {
        if (findCustomer(id) != NULL) return false;

        Customer* c = new Customer;          // dynamic memory
        c->customerID = id;
        c->name = name;
        c->phone = phone;
        c->address = address;
        c->next = NULL;

        if (head == NULL) {                  // empty list: the new node is first and last
            head = c;
            tail = c;
        } else {                             // otherwise attach it after the last node
            tail->next = c;
            tail = c;
        }
        count++;
        return true;
    }

    int getCount() { return count; }

   
    // Writes every customer as one line:  id|name|phone|address
    void saveTo(ostream& out) {
        for (Customer* c = head; c != NULL; c = c->next) {
            out << c->customerID << "|" << c->name << "|" << c->phone << "|" << c->address << "\n";
        }
    }

    void display() {
        if (head == NULL) {
            cout << YELLOW<<"  No customers registered yet.\n"<<RESET;
            return;
        }
        cout <<YELLOW<< left << setw(8) << "ID" << setw(22) << "Name"
             << setw(16) << "Phone" << "Address" <<RESET<< endl;
        printLine('-', 72);
        for (Customer* c = head; c != NULL; c = c->next) {
            cout << left << setw(8) << c->customerID << setw(22) << c->name
                 << setw(16) << c->phone << c->address << endl;
        }
    }
};

#endif
