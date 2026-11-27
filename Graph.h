// ============================================================
//  Graph.h  -  Modules 5, 6, 7 and 8: the CITY DELIVERY NETWORK
//
//  Location = vertex   (stored by number 0, 1, 2 ... with a name)
//  Road     = weighted, TWO-WAY edge (weight = distance in km)
//
//  The network is stored in TWO ways:
//    1. ADJACENCY LIST   : for every location a linked list of its roads
//                          (used by BFS, DFS and the shortest path)
//         Warehouse : [City Center,5] -> [Industrial Area,4] -> NULL
//    2. ADJACENCY MATRIX : a table, matrix[a][b] = road length (0 = no road)
//                          (used for quick "is there a road?" checks and by Prim)
//
//  Algorithms inside this class (all written by us, no STL):
//    BFS      (Queue)         - Module 6, nearest locations first
//    DFS      (Stack)         - Module 6, go as deep as possible first
//    Dijkstra                 - Module 7, shortest route between two locations
//    Prim                     - Module 8, minimum spanning tree
// ============================================================
#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <iomanip>
#include <string>
#include "Utils.h"
#include "Queue.h"
#include "Stack.h"
using namespace std;

// ##########################################################
// #  MEMBER 1  :  addRoad, addLocation, findLocation, constants, Edge struct, addToList, constructor, destructor, getters, hasRoad, saveTo, totalRoadLength
// ##########################################################
const int MAX_LOCATIONS = 50;               // the network can hold up to 50 locations
const int MAX_ROAD_LENGTH = 10000;          // longest allowed road (km)
const int INFINITE_DISTANCE = 1000000000;   // "not reached yet"
const int MAX_ROADS = MAX_LOCATIONS * (MAX_LOCATIONS - 1) / 2;
const int NO_ROUTE = -1;                    // returned by shortestPath when there is no route

// One road inside an adjacency list.
struct Edge {
    int to;         // the location at the other end of the road
    int weight;     // road length in km
    Edge* next;     // next road of the same location
};

class Graph {
private:
    string* names;          // names[i] = name of location i
    Edge** adjList;         // adjList[i] = first road of location i
    int** matrix;           // matrix[a][b] = length of the road a-b (0 = no road)
    int locationCount;
    int roadCount;
    int roadFrom[MAX_ROADS];    // the roads in the order they were added (used for saving)
    int roadTo[MAX_ROADS];
    int roadLength[MAX_ROADS];

    // Adds one road at the END of the adjacency list of 'from'.
    void addToList(int from, int to, int weight) {
        Edge* e = new Edge;
        e->to = to;
        e->weight = weight;
        e->next = NULL;
        if (adjList[from] == NULL) {
            adjList[from] = e;
        } else {
            Edge* last = adjList[from];
            while (last->next != NULL) last = last->next;
            last->next = e;
        }
    }

public:
    Graph() {
        names = new string[MAX_LOCATIONS];
        adjList = new Edge*[MAX_LOCATIONS];
        matrix = new int*[MAX_LOCATIONS];
        for (int i = 0; i < MAX_LOCATIONS; i++) {
            adjList[i] = NULL;
            matrix[i] = new int[MAX_LOCATIONS];
            for (int j = 0; j < MAX_LOCATIONS; j++) matrix[i][j] = 0;
        }
        locationCount = 0;
        roadCount = 0;
    }

    // Destructor: frees every adjacency list, every matrix row and the arrays.
    ~Graph() {
        for (int i = 0; i < MAX_LOCATIONS; i++) {
            Edge* current = adjList[i];
            while (current != NULL) {
                Edge* temp = current;
                current = current->next;
                delete temp;
            }
            delete[] matrix[i];
        }
        delete[] matrix;
        delete[] adjList;
        delete[] names;
    }

    int getLocationCount() { return locationCount; }
    int getRoadCount() { return roadCount; }
    string getName(int index) { return names[index]; }

    // Returns the number of a location (capital letters do not matter), or -1 if it does not exist.
    int findLocation(const string& name) {
        for (int i = 0; i < locationCount; i++) {
            if (sameText(names[i], name)) return i;
        }
        return -1;
    }

    // Adds a location. Returns false if the name is empty, already used, or the network is full.
    bool addLocation(const string& name) {
        if (name.length() == 0 || locationCount >= MAX_LOCATIONS) return false;
        if (findLocation(name) != -1) return false;
        names[locationCount] = name;
        locationCount++;
        return true;
    }

    bool hasRoad(int a, int b) { return matrix[a][b] != 0; }

    // Writes the network in the file format:  LOCATION|name   and   ROAD|nameA|nameB|km
    void saveTo(ostream& out) {
        for (int i = 0; i < locationCount; i++) out << "LOCATION|" << names[i] << "\n";
        for (int r = 0; r < roadCount; r++) {
            out << "ROAD|" << names[roadFrom[r]] << "|" << names[roadTo[r]] << "|" << roadLength[r] << "\n";
        }
    }

    // Adds up the length of all roads (every road is counted once).
    int totalRoadLength() {
        int total = 0;
        for (int i = 0; i < locationCount; i++) {
            for (int j = i + 1; j < locationCount; j++) {
                total += matrix[i][j];
            }
        }
        return total;
    }

    // Adds a two-way road. Returns false if the data is wrong or the road already exists.
    bool addRoad(int a, int b, int length) {
        if (a < 0 || b < 0 || a >= locationCount || b >= locationCount) return false;
        if (a == b) return false;                                   // a road needs two different places
        if (length <= 0 || length > MAX_ROAD_LENGTH) return false;  // length must be positive
        if (matrix[a][b] != 0) return false;                        // road already exists

        matrix[a][b] = length;                  // matrix: both directions
        matrix[b][a] = length;
        addToList(a, b, length);                // adjacency lists: both directions
        addToList(b, a, length);
        roadFrom[roadCount] = a;
        roadTo[roadCount] = b;
        roadLength[roadCount] = length;
        roadCount++;
        return true;
    }

    // ##########################################################
    // #  MEMBER 3  :  bfs, dfs, displayLocationList, displayAdjacencyList, displayMatrix
    // ##########################################################
    // ---------------------------------------------------------------
    //  Display (Module 5)
    // ---------------------------------------------------------------

    void displayLocationList() {
        cout << CYAN << "Locations:\n" << RESET;
        for (int i = 0; i < locationCount; i++) {
            cout << "  " << YELLOW << "[" << i << "]" << RESET << " " << left << setw(18) << names[i];
            if ((i + 1) % 3 == 0) cout << endl;
        }
        if (locationCount % 3 != 0) cout << endl;
    }

    void displayAdjacencyList() {
        cout << CYAN << "ADJACENCY LIST (every location with the roads that leave it)\n" << RESET;
        printLine('-', 78);
        for (int i = 0; i < locationCount; i++) {
            cout << YELLOW << left << setw(17) << names[i] << RESET << ": ";
            for (Edge* e = adjList[i]; e != NULL; e = e->next) {
                cout << GREEN << "[" << names[e->to] << ", " << e->weight << " km]" << RESET << " -> ";
            }
            cout << YELLOW << "NULL" << RESET << endl;
        }
    }

    void displayMatrix() {
        cout << CYAN << "ADJACENCY MATRIX (road length in km, 0 = no direct road)\n" << RESET;
        printLine('-', 78);
        cout << string(23, ' ');
        for (int j = 0; j < locationCount; j++) cout << YELLOW << right << setw(5) << j << RESET;
        cout << endl;
        for (int i = 0; i < locationCount; i++) {
            cout << YELLOW << right << "[" << setw(2) << i << "] " << left << setw(18) << names[i] << RESET << right;
            for (int j = 0; j < locationCount; j++) cout << (matrix[i][j] != 0 ? GREEN : string("")) << setw(5) << matrix[i][j] << RESET;
            cout << endl;
        }
        cout << left;
    }

    // ---------------------------------------------------------------
    //  BFS (Module 6) - uses a QUEUE
    //  Visits the start location, then all its neighbours, then the neighbours
    //  of the neighbours ... (level by level).
    //  Returns true if 'target' was reached (give target = -1 to skip this check).
    // ---------------------------------------------------------------
    bool bfs(int start, int target) {
        bool visited[MAX_LOCATIONS];
        for (int i = 0; i < MAX_LOCATIONS; i++) visited[i] = false;

        Queue<int> waiting;
        visited[start] = true;                  // mark when it enters the queue
        waiting.enqueue(start);

        cout << CYAN << "BFS visiting order:" << RESET << "\n  ";
        int reached = 0;
        int v = 0;
        while (!waiting.isEmpty()) {
            waiting.dequeue(v);                 // take the location at the front
            if (reached > 0) cout << YELLOW << " -> " << RESET;
            cout << GREEN << names[v] << RESET;
            reached++;
            for (Edge* e = adjList[v]; e != NULL; e = e->next) {
                if (!visited[e->to]) {          // new location: put it at the back of the queue
                    visited[e->to] = true;
                    waiting.enqueue(e->to);
                }
            }
        }
        cout << YELLOW << "\n  Locations reached: " << reached << " of " << locationCount << endl << RESET;
        return target >= 0 && target < locationCount && visited[target];
    }

    // ---------------------------------------------------------------
    //  DFS (Module 6) - uses a STACK
    //  Goes as deep as possible along one road, then comes back and tries the next one.
    //  Returns true if 'target' was reached (give target = -1 to skip this check).
    // ---------------------------------------------------------------
    bool dfs(int start, int target) {
        bool visited[MAX_LOCATIONS];
        for (int i = 0; i < MAX_LOCATIONS; i++) visited[i] = false;

        Stack<int> pending;
        pending.push(start);

        cout << CYAN << "DFS visiting order:" << RESET << "\n  ";
        int reached = 0;
        int v = 0;
        while (!pending.isEmpty()) {
            pending.pop(v);                     // take the location from the top
            if (visited[v]) continue;           // it was already visited through another road
            visited[v] = true;
            if (reached > 0) cout << YELLOW << " -> " << RESET;
            cout << GREEN << names[v] << RESET;
            reached++;

            // Push the neighbours in REVERSE order, so the FIRST neighbour ends on top
            // of the stack and is visited first.
            int neighbours[MAX_LOCATIONS];
            int n = 0;
            for (Edge* e = adjList[v]; e != NULL; e = e->next) {
                neighbours[n] = e->to;
                n++;
            }
            for (int i = n - 1; i >= 0; i--) {
                if (!visited[neighbours[i]]) pending.push(neighbours[i]);
            }
        }
        cout << YELLOW << "\n  Locations reached: " << reached << " of " << locationCount << endl << RESET;
        return target >= 0 && target < locationCount && visited[target];
    }

    // ##########################################################
    // #  MEMBER 2  :  shortestPath (Dijkstra), minimumSpanningTree (Prim), printRoute
    // ##########################################################
    // ---------------------------------------------------------------
    //  Shortest route (Module 7) - DIJKSTRA'S ALGORITHM
    //
    //  dist[i]   = shortest distance found so far from the source to location i
    //  parent[i] = the location we came from on that shortest way
    //  done[i]   = true when dist[i] is final
    //
    //  Repeat: take the not-done location with the smallest dist, mark it done,
    //  and see if going THROUGH it gives a shorter way to its neighbours.
    //
    //  path[] must have room for MAX_LOCATIONS numbers. On success the route
    //  (source ... destination) is stored in path[0..pathLength-1] and the total
    //  distance is returned. If there is no route, NO_ROUTE (-1) is returned.
    // ---------------------------------------------------------------
    int shortestPath(int source, int destination, int path[], int& pathLength) {
        int dist[MAX_LOCATIONS];
        int parent[MAX_LOCATIONS];
        bool done[MAX_LOCATIONS];
        for (int i = 0; i < locationCount; i++) {
            dist[i] = INFINITE_DISTANCE;
            parent[i] = -1;
            done[i] = false;
        }
        dist[source] = 0;
        pathLength = 0;

        for (int round = 0; round < locationCount; round++) {
            // 1. choose the not-done location with the smallest distance
            int u = -1;
            for (int i = 0; i < locationCount; i++) {
                if (!done[i] && dist[i] < INFINITE_DISTANCE && (u == -1 || dist[i] < dist[u])) {
                    u = i;
                }
            }
            if (u == -1) break;                 // the rest of the locations cannot be reached
            done[u] = true;
            if (u == destination) break;        // the shortest way to the destination is final

            // 2. try to improve the distance of every neighbour of u
            for (Edge* e = adjList[u]; e != NULL; e = e->next) {
                if (!done[e->to] && dist[u] + e->weight < dist[e->to]) {
                    dist[e->to] = dist[u] + e->weight;
                    parent[e->to] = u;
                }
            }
        }

        if (dist[destination] >= INFINITE_DISTANCE) return NO_ROUTE;

        // Walk back from the destination to the source using parent[], then reverse the order.
        int backwards[MAX_LOCATIONS];
        int n = 0;
        for (int v = destination; v != -1; v = parent[v]) {
            backwards[n] = v;
            n++;
        }
        for (int i = 0; i < n; i++) path[i] = backwards[n - 1 - i];
        pathLength = n;
        return dist[destination];
    }

    // Prints a route in the format  A -> B -> C  with the total distance.
    void printRoute(int path[], int pathLength, int totalDistance) {
        cout << CYAN << "Recommended Route:\n" << RESET;
        for (int i = 0; i < pathLength; i++) {
            if (i > 0) cout << YELLOW << " -> " << RESET;
            cout << GREEN << names[path[i]] << RESET;
        }
        cout << GREEN << "\nTotal Distance: " << totalDistance << " km\n" << RESET;
    }

    // ---------------------------------------------------------------
    //  Minimum Spanning Tree (Module 8) - PRIM'S ALGORITHM
    //
    //  Start with one location. Again and again add the CHEAPEST road that connects
    //  a location inside the tree to a location outside the tree.
    //  When all locations are inside, the roads we chose connect everything with
    //  the smallest possible total cost.
    //
    //  cheapest[i] = cost of the cheapest road that connects i to the tree
    //  from[i]     = the tree location at the other end of that road
    //  Prints the chosen roads and returns the total cost.
    // ---------------------------------------------------------------
    int minimumSpanningTree() {
        if (locationCount == 0) {
            cout << RED << "  The network has no locations.\n" << RESET;
            return 0;
        }
        bool inTree[MAX_LOCATIONS];
        int cheapest[MAX_LOCATIONS];
        int from[MAX_LOCATIONS];
        for (int i = 0; i < locationCount; i++) {
            inTree[i] = false;
            cheapest[i] = INFINITE_DISTANCE;
            from[i] = -1;
        }
        cheapest[0] = 0;                        // start with location 0

        int totalCost = 0;
        int roadsChosen = 0;
        int reached = 0;
        cout << CYAN << "Selected roads (in the order Prim's algorithm adds them):\n" << RESET;

        for (int round = 0; round < locationCount; round++) {
            // choose the outside location with the cheapest connection to the tree
            int u = -1;
            for (int i = 0; i < locationCount; i++) {
                if (!inTree[i] && cheapest[i] < INFINITE_DISTANCE && (u == -1 || cheapest[i] < cheapest[u])) {
                    u = i;
                }
            }
            if (u == -1) break;                 // the remaining locations are not connected
            inTree[u] = true;
            reached++;

            if (from[u] != -1) {                // every location except the first adds one road
                roadsChosen++;
                totalCost += cheapest[u];
                cout << GREEN << "  " << roadsChosen << ". " << names[from[u]] << " --- " << names[u]
                     << "   (" << cheapest[u] << " km)\n" << RESET;
            }

            // the new tree member may offer cheaper connections to the outside locations
            for (int v = 0; v < locationCount; v++) {
                if (matrix[u][v] != 0 && !inTree[v] && matrix[u][v] < cheapest[v]) {
                    cheapest[v] = matrix[u][v];
                    from[v] = u;
                }
            }
        }

        cout << GREEN << "Total network cost: " << totalCost << " km\n" << RESET;
        if (reached < locationCount) {
            cout << RED << "WARNING: the network is not fully connected. These locations have no road link to "
                 << names[0] << ":\n  " << RESET;
            bool first = true;
            for (int i = 0; i < locationCount; i++) {
                if (!inTree[i]) {
                    if (!first) cout << ", ";
                    cout << RED << names[i] << RESET;
                    first = false;
                }
            }
            cout << endl;
        }
        return totalCost;
    }
};

#endif
