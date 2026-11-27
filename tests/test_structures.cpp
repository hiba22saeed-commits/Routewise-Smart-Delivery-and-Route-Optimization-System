// ============================================================
//  test_structures.cpp  -  Automatic tests for the RouteWise data structures
//
//  Build and run (from the RouteWise folder):
//       g++ tests/test_structures.cpp -o test_structures
//       ./test_structures            (Windows: test_structures.exe)
//
//  Every check() prints [FAIL] when something is wrong.
//  At the end you see how many checks passed.
//
//  The graph algorithms are checked against a second, independent method:
//     Dijkstra  (our code)  <->  Floyd-Warshall (written here in the test)
//     Prim      (our code)  <->  Kruskal        (written here in the test)
//  on 300 random graphs.
// ============================================================
#include <iostream>
#include <sstream>
#include <string>
#include "../Utils.h"
#include "../Models.h"
#include "../Stack.h"
#include "../Queue.h"
#include "../UrgentHeap.h"
#include "../DeliveryHistory.h"
#include "../ParcelHashTable.h"
#include "../ParcelBST.h"
#include "../ParcelAVL.h"
#include "../Graph.h"
using namespace std;

int passedChecks = 0;
int failedChecks = 0;

void check(bool condition, const string& what) {
    if (condition) {
        passedChecks++;
    } else {
        failedChecks++;
        cout << "  [FAIL] " << what << endl;
    }
}

// A small, repeatable random number generator (same numbers on every run).
unsigned int randomSeed = 12345;
int nextRandom(int limit) {                 // returns 0 ... limit-1
    randomSeed = randomSeed * 1103515245u + 12345u;
    return (int)((randomSeed >> 16) % (unsigned int)limit);
}

// Catches everything that is printed with cout, so a test can look at it.
class CaptureCout {
private:
    streambuf* oldBuffer;
    ostringstream buffer;
    bool finished;
public:
    CaptureCout() {
        oldBuffer = cout.rdbuf(buffer.rdbuf());
        finished = false;
    }
    string finish() {
        if (!finished) cout.rdbuf(oldBuffer);
        finished = true;
        return buffer.str();
    }
    ~CaptureCout() { finish(); }
};

Parcel* makeParcel(int id, int priority, int arrivalNo) {
    Parcel* p = new Parcel;
    p->trackingID = id;
    p->customerID = 1;
    p->source = "A";
    p->destination = "B";
    p->weight = 1.0;
    p->priority = priority;
    p->status = STATUS_REGISTERED;
    p->arrivalNo = arrivalNo;
    return p;
}

string numberName(int i) {                  // 0 -> "N0", 11 -> "N11"
    string name = "N";
    if (i >= 10) name += (char)('0' + i / 10);
    name += (char)('0' + i % 10);
    return name;
}

// ---------------------------------------------------------------
void testUtils() {
    cout << "Testing helper functions ...\n";
    check(trim("  hello \t") == "hello", "trim removes spaces and tabs");
    check(trim("   ") == "", "trim of only spaces");
    check(trim("") == "", "trim of an empty text");
    check(toLower("City CENTER") == "city center", "toLower");
    check(sameText("Airport", "aIRPORT"), "sameText ignores capital letters");
    check(!sameText("Airport", "Airports"), "sameText sees different words");
    check(isWholeNumber("123") && isWholeNumber("-5") && isWholeNumber("+7"), "whole numbers are accepted");
    check(!isWholeNumber("") && !isWholeNumber("12a") && !isWholeNumber("-") && !isWholeNumber("1234567890"),
          "wrong whole numbers are rejected");
    check(isDecimalNumber("2.5") && isDecimalNumber("10") && isDecimalNumber(".5"), "decimal numbers are accepted");
    check(!isDecimalNumber("") && !isDecimalNumber("1.2.3") && !isDecimalNumber("-1")
          && !isDecimalNumber("abc") && !isDecimalNumber("."), "wrong decimal numbers are rejected");

    string parts[4];
    int n = splitText("101| Ali Khan |0300|Street 5", '|', parts, 4);
    check(n == 4 && parts[0] == "101" && parts[1] == "Ali Khan" && parts[3] == "Street 5", "splitText: normal line");
    check(splitText("a|b|c|d|e", '|', parts, 4) == 5, "splitText: too many fields are reported");
    check(splitText("a|b", '|', parts, 4) == 2, "splitText: too few fields are reported");
}

// ---------------------------------------------------------------
void testStack() {
    cout << "Testing Stack ...\n";
    Stack<int> s;
    int value = -1;
    check(s.isEmpty() && s.size() == 0, "new stack is empty");
    check(!s.pop(value) && !s.peek(value), "pop and peek on an empty stack return false");
    for (int i = 1; i <= 5; i++) s.push(i * 10);
    check(s.size() == 5, "size after 5 pushes");
    check(s.peek(value) && value == 50 && s.size() == 5, "peek shows the top and removes nothing");
    bool lifo = true;
    for (int i = 5; i >= 1; i--) {
        if (!s.pop(value) || value != i * 10) lifo = false;
    }
    check(lifo, "items come out in Last-In-First-Out order");
    check(s.isEmpty(), "stack is empty again");
    for (int i = 0; i < 1000; i++) s.push(i);
    s.clear();
    check(s.isEmpty() && s.size() == 0, "clear() empties the stack");
}

void testQueue() {
    cout << "Testing Queue ...\n";
    Queue<int> q;
    int value = -1;
    check(q.isEmpty() && q.size() == 0, "new queue is empty");
    check(!q.dequeue(value) && !q.peek(value), "dequeue and peek on an empty queue return false");
    for (int i = 1; i <= 5; i++) q.enqueue(i);
    check(q.size() == 5, "size after 5 enqueues");
    check(q.peek(value) && value == 1, "peek shows the front item");
    bool fifo = true;
    for (int i = 1; i <= 5; i++) {
        if (!q.dequeue(value) || value != i) fifo = false;
    }
    check(fifo, "items come out in First-In-First-Out order");
    check(q.isEmpty(), "queue is empty again");

    // enqueueFront (used by Undo) and rear handling after the queue became empty
    q.enqueueFront(7);
    q.enqueue(8);
    q.enqueueFront(6);
    check(q.size() == 3, "size after enqueueFront");
    check(q.dequeue(value) && value == 6, "enqueueFront item leaves first");
    check(q.dequeue(value) && value == 7, "then the item that was in the queue");
    check(q.dequeue(value) && value == 8, "then the last item");
    q.enqueue(9);
    check(q.dequeue(value) && value == 9 && q.isEmpty(), "queue works again after being empty");
}

// ---------------------------------------------------------------
void testHeap() {
    cout << "Testing UrgentHeap ...\n";

    // the tie rule: higher priority first, equal priority -> earlier arrival first
    UrgentHeap small;
    Parcel* a = makeParcel(1, 4, 4);
    Parcel* b = makeParcel(2, 5, 5);
    Parcel* c = makeParcel(3, 4, 6);
    Parcel* d = makeParcel(4, 5, 7);
    small.insert(a); small.insert(b); small.insert(c); small.insert(d);
    Parcel* p = NULL;
    check(small.peek() == b, "peek shows the most urgent parcel");
    check(small.removeTop(p) && p == b, "1st: priority 5, arrived first");
    check(small.removeTop(p) && p == d, "2nd: priority 5, arrived later");
    check(small.removeTop(p) && p == a, "3rd: priority 4, arrived first");
    check(small.removeTop(p) && p == c, "4th: priority 4, arrived later");
    check(!small.removeTop(p) && small.isEmpty() && small.peek() == NULL, "heap is empty at the end");
    small.insert(a);                                // a parcel can be put back (this is what Undo does)
    check(small.size() == 1 && small.peek() == a, "re-inserting a parcel works");
    delete a; delete b; delete c; delete d;

    // 300 random parcels (the heap must grow beyond its first size of 10)
    const int total = 300;
    Parcel* parcels[total];
    int order[total];
    for (int i = 0; i < total; i++) {
        parcels[i] = makeParcel(1000 + i, 1 + nextRandom(5), i + 1);
        order[i] = i;
    }
    for (int i = total - 1; i > 0; i--) {           // shuffle the insertion order
        int j = nextRandom(i + 1);
        int temp = order[i]; order[i] = order[j]; order[j] = temp;
    }
    UrgentHeap heap;
    for (int i = 0; i < total; i++) heap.insert(parcels[order[i]]);
    check(heap.size() == total, "300 parcels were inserted");

    bool sortedOrder = true;
    int removed = 0;
    Parcel* previous = NULL;
    while (heap.removeTop(p)) {
        if (previous != NULL) {
            bool wrong = previous->priority < p->priority ||
                         (previous->priority == p->priority && previous->arrivalNo > p->arrivalNo);
            if (wrong) sortedOrder = false;
        }
        previous = p;
        removed++;
    }
    check(removed == total, "all 300 parcels came out");
    check(sortedOrder, "parcels came out by priority, then by arrival order");
    for (int i = 0; i < total; i++) delete parcels[i];
}

// ---------------------------------------------------------------
void testHistory() {
    cout << "Testing DeliveryHistory ...\n";
    DeliveryHistory h;
    Parcel* a = makeParcel(7001, 1, 1);
    Parcel* b = makeParcel(7002, 1, 2);
    Parcel* c = makeParcel(7003, 1, 3);
    check(h.removeLast() == NULL && h.getCount() == 0, "empty history");
    h.addAtEnd(a); h.addAtEnd(b); h.addAtEnd(c);
    check(h.getCount() == 3, "three records");

    CaptureCout capture;
    h.display();
    string text = capture.finish();
    size_t first = text.find("7003"), second = text.find("7002"), third = text.find("7001");
    check(first != string::npos && first < second && second < third, "display shows the newest record first");

    check(h.removeLast() == c && h.getCount() == 2, "removeLast gives the newest record");
    check(h.removeLast() == b, "then the second newest");
    h.addAtEnd(c);
    check(h.removeLast() == c && h.removeLast() == a, "list still correct after removing and adding");
    check(h.getCount() == 0 && h.removeLast() == NULL, "empty again");
    delete a; delete b; delete c;
}

// ---------------------------------------------------------------
void testHashTable() {
    cout << "Testing ParcelHashTable ...\n";
    ParcelHashTable table;
    const int wanted = 200;
    Parcel* stored[wanted];
    int storedCount = 0;

    CaptureCout capture;                                // hide the "table grew" messages
    while (storedCount < wanted) {
        int id = 1 + nextRandom(5000);
        Parcel* p = makeParcel(id, 1, 0);
        if (table.insert(p)) {
            stored[storedCount] = p;
            storedCount++;
        } else {
            delete p;                                   // duplicate ID (the table did not take it)
        }
    }
    capture.finish();

    check(table.getCount() == wanted, "200 different parcels are stored");
    check(table.getCount() * 2 <= table.getTableSize(), "table is never more than half full");

    bool allFound = true;
    bool probesOk = true;
    for (int i = 0; i < wanted; i++) {
        int probes = 0, slot = -1;
        Parcel* found = table.search(stored[i]->trackingID, probes, slot);
        if (found != stored[i]) allFound = false;
        if (probes < 1 || slot < 0 || slot >= table.getTableSize()) probesOk = false;
    }
    check(allFound, "every stored parcel is found (same record)");
    check(probesOk, "probe count and slot number are reported");

    check(table.search(5001) == NULL && table.search(999999) == NULL, "missing IDs are not found");

    Parcel* duplicate = makeParcel(stored[0]->trackingID, 1, 0);
    check(!table.insert(duplicate), "a duplicate tracking ID is refused");
    delete duplicate;

    stored[0]->status = STATUS_DELIVERED;
    stored[1]->status = STATUS_DELIVERED;
    stored[2]->status = STATUS_PENDING;
    check(table.countWithStatus(STATUS_DELIVERED) == 2, "countWithStatus (Delivered)");
    check(table.countWithStatus(STATUS_PENDING) == 1, "countWithStatus (Pending)");
    check(table.countWithStatus(STATUS_REGISTERED) == wanted - 3, "countWithStatus (Registered)");

    // IDs that all have the same home slot must still be found (linear probing)
    ParcelHashTable colliding;
    CaptureCout capture2;
    int size = colliding.getTableSize();                // 11
    Parcel* chain[5];
    for (int i = 0; i < 5; i++) {
        chain[i] = makeParcel(7 + i * size, 1, 0);      // all have home slot 7 while the size is 11
        colliding.insert(chain[i]);
    }
    capture2.finish();
    bool chainFound = true;
    for (int i = 0; i < 5; i++) {
        if (colliding.search(chain[i]->trackingID) != chain[i]) chainFound = false;
    }
    check(chainFound, "parcels with the same home slot are all found");
}

// ---------------------------------------------------------------
// Reads all numbers of a text such as "1 2 3 " into an array. Returns how many there are.
int readNumbers(const string& text, int numbers[], int maxCount) {
    istringstream in(text);
    int count = 0;
    int value = 0;
    while (count < maxCount && (in >> value)) {
        numbers[count] = value;
        count++;
    }
    return count;
}

string preorderOfAvl(int a, int b, int c) {             // inserts three keys, returns the pre-order text
    ParcelAVL tree;
    tree.insert(a, NULL); tree.insert(b, NULL); tree.insert(c, NULL);
    CaptureCout capture;
    tree.showPreorder();
    return capture.finish();
}

void testTrees() {
    cout << "Testing ParcelBST and ParcelAVL ...\n";

    // ----- the four AVL rotation cases -----
    check(preorderOfAvl(3, 2, 1) == "2 1 3 \n", "AVL Left-Left case (rotate right)");
    check(preorderOfAvl(1, 2, 3) == "2 1 3 \n", "AVL Right-Right case (rotate left)");
    check(preorderOfAvl(3, 1, 2) == "2 1 3 \n", "AVL Left-Right case (double rotation)");
    check(preorderOfAvl(1, 3, 2) == "2 1 3 \n", "AVL Right-Left case (double rotation)");

    // ----- a perfect tree: 4,2,6,1,3,5,7 -----
    int perfect[7] = {4, 2, 6, 1, 3, 5, 7};
    ParcelBST bst7;
    for (int i = 0; i < 7; i++) bst7.insert(perfect[i], NULL);
    check(bst7.getHeight() == 3 && bst7.getCount() == 7, "BST: height 3 for 7 keys");
    double average = bst7.averageSearchSteps();
    check(average > 2.428 && average < 2.429, "BST: average search steps = 17 / 7");
    check(bst7.stepsToFind(4) == 1 && bst7.stepsToFind(7) == 3 && bst7.stepsToFind(99) == 3,
          "BST: steps to find the root, a leaf and a missing key");
    check(bst7.isBalanced(), "BST: the perfect tree is balanced");
    check(!bst7.insert(4, NULL) && bst7.getCount() == 7, "BST: a duplicate key is refused");

    // ----- sorted insertion: the BST turns into a chain, the AVL stays balanced -----
    ParcelBST chainBst;
    ParcelAVL balancedAvl;
    for (int i = 1; i <= 1000; i++) {
        chainBst.insert(i, NULL);
        balancedAvl.insert(i, NULL);
    }
    check(chainBst.getHeight() == 1000, "BST with 1000 sorted keys has height 1000");
    check(!chainBst.isBalanced(), "that BST is not balanced");
    check(balancedAvl.getHeight() <= 11, "AVL with 1000 sorted keys has height <= 11");
    check(balancedAvl.isBalanced(), "that AVL tree is balanced");
    check(balancedAvl.getRotationCount() > 0, "the AVL tree used rotations");
    int steps = 0;
    check(balancedAvl.search(777, steps) == NULL && steps <= 11, "AVL: search needs at most 11 steps");

    // ----- 500 random keys -----
    ParcelBST randomBst;
    ParcelAVL randomAvl;
    int keys[500];
    int inserted = 0;
    while (inserted < 500) {
        int key = 1 + nextRandom(100000);
        if (randomBst.insert(key, NULL)) {
            check(randomAvl.insert(key, NULL), "AVL accepts a new key");
            keys[inserted] = key;
            inserted++;
        }
    }
    check(randomAvl.isBalanced(), "AVL with 500 random keys is balanced");
    check(randomAvl.getHeight() <= 13, "AVL height is small (<= 13 for 500 keys)");
    check(randomAvl.getHeight() <= randomBst.getHeight(), "AVL is never taller than the BST");

    bool allFound = true;
    for (int i = 0; i < 500; i++) {
        int s1 = 0, s2 = 0;
        randomBst.search(keys[i], s1);
        randomAvl.search(keys[i], s2);
        if (randomBst.stepsToFind(keys[i]) != s1 || randomAvl.stepsToFind(keys[i]) != s2) allFound = false;
        if (s1 < 1 || s2 < 1) allFound = false;
    }
    check(allFound, "every key is found in both trees");

    // in-order traversal must give the keys in sorted order
    int numbers[600];
    CaptureCout capture;
    randomBst.showInorder();
    string bstText = capture.finish();
    CaptureCout capture2;
    randomAvl.showInorder();
    string avlText = capture2.finish();
    int bstCount = readNumbers(bstText, numbers, 600);
    bool bstSorted = (bstCount == 500);
    for (int i = 1; i < bstCount; i++) if (numbers[i - 1] >= numbers[i]) bstSorted = false;
    int avlCount = readNumbers(avlText, numbers, 600);
    bool avlSorted = (avlCount == 500);
    for (int i = 1; i < avlCount; i++) if (numbers[i - 1] >= numbers[i]) avlSorted = false;
    check(bstSorted, "BST in-order traversal is sorted");
    check(avlSorted, "AVL in-order traversal is sorted");
}

// ---------------------------------------------------------------
void testGraphBasics() {
    cout << "Testing Graph (the sample city network) ...\n";
    Graph g;
    const char* names[8] = {"Warehouse", "City Center", "University", "Hospital",
                            "Airport", "Railway Station", "Industrial Area", "Residential Area"};
    for (int i = 0; i < 8; i++) check(g.addLocation(names[i]), "addLocation");
    check(!g.addLocation("warehouse") && !g.addLocation(""), "duplicate (any letter case) and empty names are refused");
    check(g.findLocation("AIRPORT") == 4 && g.findLocation("Mars") == -1, "findLocation");

    // roads: a b length
    int roads[13][3] = {{0,1,5},{0,6,4},{0,5,9},{1,2,6},{1,3,4},{1,5,3},{2,4,7},
                        {2,7,5},{3,7,3},{3,4,12},{5,6,6},{6,4,20},{7,4,9}};
    for (int i = 0; i < 13; i++) check(g.addRoad(roads[i][0], roads[i][1], roads[i][2]), "addRoad");
    check(g.getRoadCount() == 13 && g.getLocationCount() == 8, "13 roads and 8 locations");
    check(!g.addRoad(0, 1, 7), "a second road between the same places is refused");
    check(!g.addRoad(1, 1, 7), "a road from a place to itself is refused");
    check(!g.addRoad(0, 2, 0) && !g.addRoad(0, 2, -4) && !g.addRoad(0, 2, MAX_ROAD_LENGTH + 1), "bad lengths are refused");
    check(!g.addRoad(0, 99, 5) && !g.addRoad(-1, 2, 5), "unknown locations are refused");
    check(g.hasRoad(0, 1) && g.hasRoad(1, 0) && !g.hasRoad(0, 2), "roads are two-way");
    check(g.totalRoadLength() == 93, "all roads together are 93 km");

    // shortest route
    int path[MAX_LOCATIONS];
    int length = 0;
    check(g.shortestPath(0, 4, path, length) == 18, "Warehouse -> Airport is 18 km");
    check(length == 4 && path[0] == 0 && path[1] == 1 && path[2] == 2 && path[3] == 4, "route: Warehouse, City Center, University, Airport");
    check(g.shortestPath(0, 7, path, length) == 12 && length == 4 && path[2] == 3, "Warehouse -> Residential Area is 12 km via the Hospital");
    check(g.shortestPath(3, 3, path, length) == 0 && length == 1, "route to the same place is 0 km");

    // BFS and DFS
    CaptureCout capture;
    bool reached = g.bfs(0, 4);
    string bfsText = capture.finish();
    check(reached, "BFS: Airport is reachable");
    check(bfsText.find("Warehouse -> City Center -> Industrial Area -> Railway Station -> University -> Hospital -> Airport -> Residential Area") != string::npos,
          "BFS order is correct");
    CaptureCout capture2;
    reached = g.dfs(0, 4);
    string dfsText = capture2.finish();
    check(reached, "DFS: Airport is reachable");
    check(dfsText.find("Warehouse -> City Center -> University -> Airport -> Hospital -> Residential Area -> Industrial Area -> Railway Station") != string::npos,
          "DFS order is correct");

    // minimum spanning tree
    CaptureCout capture3;
    int cost = g.minimumSpanningTree();
    string mstText = capture3.finish();
    check(cost == 31, "minimum spanning tree costs 31 km");
    check(mstText.find("WARNING") == string::npos, "no warning for a connected network");

    // an extra location without roads is not reachable
    g.addLocation("Farmhouse");
    CaptureCout capture4;
    reached = g.bfs(0, 8);
    bool reachedDfs = g.dfs(0, 8);
    int farm = g.minimumSpanningTree();
    string farmText = capture4.finish();
    check(!reached && !reachedDfs, "BFS and DFS: the new location is NOT reachable");
    check(g.shortestPath(0, 8, path, length) == NO_ROUTE && length == 0, "no route to the new location");
    check(farm == 31 && farmText.find("WARNING") != string::npos && farmText.find("Farmhouse") != string::npos,
          "MST warns about the location that has no road");
    CaptureCout capture5;
    bool noTarget = g.bfs(0, -1);
    capture5.finish();
    check(!noTarget, "BFS without a target returns false and does not crash");
}

// ---------------------------------------------------------------
// Random graphs: our algorithms against Floyd-Warshall and Kruskal.
void testGraphRandom() {
    cout << "Testing Graph on 300 random graphs (against Floyd-Warshall and Kruskal) ...\n";
    const int INF = 1000000000;
    bool distancesOk = true, pathsOk = true, reachOk = true, mstOk = true;

    for (int round = 0; round < 300; round++) {
        int n = 2 + nextRandom(11);                         // 2 ... 12 locations
        Graph g;
        for (int i = 0; i < n; i++) g.addLocation(numberName(i));

        int weight[12][12];
        int dist[12][12];
        for (int i = 0; i < 12; i++)
            for (int j = 0; j < 12; j++) {
                weight[i][j] = 0;
                dist[i][j] = (i == j) ? 0 : INF;
            }
        int edgeA[80], edgeB[80], edgeW[80];
        int edges = 0;
        int chance = 15 + nextRandom(50);                   // sparse and dense graphs
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (nextRandom(100) < chance) {
                    int w = 1 + nextRandom(20);
                    g.addRoad(i, j, w);
                    weight[i][j] = weight[j][i] = w;
                    dist[i][j] = dist[j][i] = w;
                    edgeA[edges] = i; edgeB[edges] = j; edgeW[edges] = w;
                    edges++;
                }

        // Floyd-Warshall: shortest distance between every pair
        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    if (dist[i][k] < INF && dist[k][j] < INF && dist[i][k] + dist[k][j] < dist[i][j])
                        dist[i][j] = dist[i][k] + dist[k][j];

        for (int s = 0; s < n; s++) {
            for (int t = 0; t < n; t++) {
                int path[MAX_LOCATIONS];
                int length = 0;
                int found = g.shortestPath(s, t, path, length);
                int expected = (dist[s][t] >= INF) ? NO_ROUTE : dist[s][t];
                if (found != expected) distancesOk = false;
                if (found != NO_ROUTE) {                    // the route must be a real route with that length
                    int sum = 0;
                    if (length < 1 || path[0] != s || path[length - 1] != t) pathsOk = false;
                    for (int i = 1; i < length; i++) {
                        int w = weight[path[i - 1]][path[i]];
                        if (w == 0) pathsOk = false;
                        sum += w;
                    }
                    if (sum != found) pathsOk = false;
                }
                CaptureCout capture;
                bool viaBfs = g.bfs(s, t);
                bool viaDfs = g.dfs(s, t);
                capture.finish();
                bool expectedReach = (dist[s][t] < INF);
                if (viaBfs != expectedReach || viaDfs != expectedReach) reachOk = false;
            }
        }

        // Kruskal on the part of the graph that location 0 can reach (Prim starts at location 0)
        int order[80];
        for (int i = 0; i < edges; i++) order[i] = i;
        for (int i = 1; i < edges; i++) {                   // insertion sort by weight
            int key = order[i];
            int j = i - 1;
            while (j >= 0 && edgeW[order[j]] > edgeW[key]) { order[j + 1] = order[j]; j--; }
            order[j + 1] = key;
        }
        int parent[12];
        for (int i = 0; i < n; i++) parent[i] = i;
        int kruskalTotal = 0;
        for (int i = 0; i < edges; i++) {
            int a = edgeA[order[i]], b = edgeB[order[i]];
            if (dist[0][a] >= INF) continue;                // not in the component of location 0
            while (parent[a] != a) a = parent[a];
            while (parent[b] != b) b = parent[b];
            if (a != b) {
                parent[a] = b;
                kruskalTotal += edgeW[order[i]];
            }
        }
        CaptureCout capture;
        int primTotal = g.minimumSpanningTree();
        capture.finish();
        if (primTotal != kruskalTotal) mstOk = false;
    }
    check(distancesOk, "Dijkstra distances equal Floyd-Warshall distances");
    check(pathsOk, "every route is a real route with the reported length");
    check(reachOk, "BFS and DFS reachability is correct");
    check(mstOk, "Prim's total cost equals Kruskal's total cost");
}

// ---------------------------------------------------------------
int main() {
    cout << "===== RouteWise data structure tests =====\n";
    testUtils();
    testStack();
    testQueue();
    testHeap();
    testHistory();
    testHashTable();
    testTrees();
    testGraphBasics();
    testGraphRandom();

    cout << "\nChecks passed: " << passedChecks << "\nChecks failed: " << failedChecks << endl;
    if (failedChecks == 0) {
        cout << "ALL TESTS PASSED\n";
        return 0;
    }
    cout << "SOME TESTS FAILED\n";
    return 1;
}
