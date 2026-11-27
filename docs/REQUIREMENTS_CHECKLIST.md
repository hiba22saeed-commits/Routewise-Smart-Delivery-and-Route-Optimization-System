# RouteWise - Requirements Checklist

Every requirement of the project document, where it is implemented, and how it was verified.
(DS = data structure. "Unit tests" = `tests/test_structures.cpp`; "Demo" = `tests/demo_output.txt`.)

## Modules

| # | Requirement | Implemented in | Verified by |
|---|---|---|---|
| 1 | Customer: ID, name, phone, address | `Models.h` (`Customer`), `CustomerList.h` | Demo, menu 1 / 10-3 |
| 1 | Parcel: tracking ID, customer ID, source, destination, weight, priority, status | `Models.h` (`Parcel`), menu 2 in `RouteWiseSystem.h` | Demo |
| 1 | Records created with dynamic memory | `new Customer`, `new Parcel` | AddressSanitizer: no leaks |
| 1 | Custom hash table with probing for tracking-ID lookup | `ParcelHashTable.h` (linear probing, auto-grow) | Unit tests (200 random IDs, forced collisions); Demo menu 3 and 10-4 |
| 2 | Normal requests in arrival order, custom queue | `Queue.h`, menu 4 | Unit tests; Demo menu 9 and 6 |
| 2 | Urgent requests, custom heap, higher priority first, ties by arrival | `UrgentHeap.h`, menu 5 | Unit tests (300 random parcels, tie rule); Demo |
| 3 | History of processed / completed deliveries in a linked structure | `DeliveryHistory.h` (doubly linked list), menu 10-2 | Unit tests; Demo |
| 3 | Undo of the latest status-changing operation with a custom stack; parcel restored to previous state | `Stack.h`, `UndoRecord` in `Models.h`, menu 8 | Demo; 1500-operation random sessions compared with a model |
| 4 | Ordered parcel index with BST: insert, search, traversal | `ParcelBST.h` | Unit tests; menu 16 (in/pre/post-order) |
| 4 | AVL version; show how balancing changes height | `ParcelAVL.h`, menu 16 (table, drawings, sorted-ID demo) | Unit tests (4 rotation cases); Demo |
| 5 | Locations as vertices, roads as weighted edges | `Graph.h` | Unit tests |
| 5 | Adjacency list AND adjacency matrix; add locations, add roads, display | `Graph.h`, menu 11 | Demo |
| 6 | BFS and DFS with custom structures; traversal order; reachability | `Graph.h` (`Queue<int>`, `Stack<int>`), menus 12 and 13 | Unit tests (orders; 300 random graphs); Demo |
| 7 | Shortest route and total distance, algorithm written by us | `Graph.h` (Dijkstra), menu 14 | Unit tests vs Floyd-Warshall; Demo (18 km example) |
| 8 | Minimum spanning tree: chosen connections and total cost | `Graph.h` (Prim), menu 15 | Unit tests vs Kruskal; Demo (31 km) |

## Integrated menu (all 18 options)

| Option | Function | Option | Function |
|---|---|---|---|
| 1 | `registerCustomer` | 10 | `displayParcelRecords` (parcels, history, customers, hash layout) |
| 2 | `registerParcel` | 11 | `displayCityNetwork` (+ add location / road) |
| 3 | `searchParcel` | 12 | `bfsTraversal` |
| 4 | `addNormalRequest` | 13 | `dfsTraversal` |
| 5 | `addUrgentRequest` | 14 | `findShortestRoute` |
| 6 | `processNextDelivery` | 15 | `generateMinimumNetwork` |
| 7 | `markDelivered` | 16 | `compareBSTandAVL` |
| 8 | `undoLastOperation` | 17 | `displayStatistics` |
| 9 | `displayPendingDeliveries` | 18 | Exit (destructors free all memory) |

## Implementation restrictions

| Requirement | Status |
|---|---|
| All listed structures implemented by us | Yes - see the table above |
| No STL containers (vector, list, stack, queue, deque, map, unordered_map, set, priority_queue) | None used. Only `iostream`, `iomanip`, `fstream`, `string`, `cstdlib` (the unit test also uses `sstream` to capture output) |
| Dynamic memory released correctly | Every destructor frees its nodes / arrays; parcels are deleted once by the hash table. AddressSanitizer reports no leak or invalid access |
| Not hard-coded for the demo data | All data comes from text files or from typing; limits are only 50 locations and the validated ranges |

## Required demonstration scenario

| Capability | Menu | Where to see it |
|---|---|---|
| Multiple customers, parcels with different priorities, network of 8 locations / 13 roads | loaded automatically at start, 17 | `data/` |
| Normal and urgent requests, correct dispatch | 4, 5, 9, 6 | Demo |
| Parcel lookup through the hash table | 3, 10-4 | Demo (parcel 1032 collides, 2 probes) |
| Undo of a recent operation | 7, 8 | Demo |
| BST and AVL comparison by height and structure | 16 | Demo (10 vs 4 levels) |
| BFS and DFS of the city network | 12, 13 | Demo |
| Shortest path between two locations | 14 | Demo (Warehouse -> Airport, 18 km) |
| Minimum spanning tree | 15 | Demo (31 km) |

## Extras beyond the minimum
Automatic load at start and automatic save of new records, automatic hash-table growth, text-file format with line-by-line error messages, full input validation,
shortest route shown when a delivery is processed, BST/AVL average search steps and rotation count,
sideways tree drawings, delivery history shown newest first, Makefile, unit tests, scripted demo.
