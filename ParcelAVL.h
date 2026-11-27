// ============================================================
//  ParcelAVL.h  -  Module 4: BALANCED parcel index in an AVL TREE
//
//  An AVL tree is a BST that keeps itself balanced.
//     balance factor of a node = height(left subtree) - height(right subtree)
//     AVL rule: the balance factor of EVERY node must be -1, 0 or +1.
//
//  After each insertion we go back up and check every node. If a node is
//  out of balance (+2 or -2) we fix it with ROTATIONS:
//        Left-Left   (left side too tall)            -> rotate right
//        Right-Right (right side too tall)           -> rotate left
//        Left-Right                                  -> rotate left, then right
//        Right-Left                                  -> rotate right, then left
//
//  Result: the height is always about log2(n), so searching stays fast
//  even when the IDs are inserted in sorted order.
//
//  Height is counted in LEVELS: empty tree = 0, one node = 1.
//  The nodes store only POINTERS to parcels (parcels belong to the hash table).
// ============================================================
#ifndef PARCEL_AVL_H
#define PARCEL_AVL_H

#include <iostream>
#include <string>
#include <cstdio>
#include "Models.h"
using namespace std;

// ##########################################################
// #  MEMBER 1  :  rotateRight, rotateLeft, AVLNode struct, heightOf, maxOf, updateHeight, balanceFactor
// ##########################################################
struct AVLNode {
    int key;                // tracking ID
    Parcel* parcel;         // pointer to the parcel record
    AVLNode* left;
    AVLNode* right;
    int height;             // height of the subtree that starts at this node
};

class ParcelAVL {
private:
    AVLNode* root;
    int nodeCount;
    int rotationCount;      // how many rotations were needed so far

    // ---------- height and balance ----------

    int heightOf(AVLNode* node) {
        if (node == NULL) return 0;
        return node->height;
    }

    int maxOf(int a, int b) {
        if (a > b) return a;
        return b;
    }

    void updateHeight(AVLNode* node) {
        node->height = 1 + maxOf(heightOf(node->left), heightOf(node->right));
    }

    int balanceFactor(AVLNode* node) {
        if (node == NULL) return 0;
        return heightOf(node->left) - heightOf(node->right);
    }

    // ---------- rotations ----------

    /*  Right rotation (the left child x moves up):

              y                x
             / \              / \
            x   C    ==>     A   y
           / \                  / \
          A   B                B   C
    */
    AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* middle = x->right;
        x->right = y;               // y becomes the right child of x
        y->left = middle;           // B moves from x to y
        updateHeight(y);            // y is lower now, so update it first
        updateHeight(x);
        rotationCount++;
        return x;                   // x is the new root of this subtree
    }

    /*  Left rotation (the right child y moves up):

            x                    y
           / \                  / \
          A   y      ==>       x   C
             / \              / \
            B   C            A   B
    */
    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* middle = y->left;
        y->left = x;                // x becomes the left child of y
        x->right = middle;          // B moves from y to x
        updateHeight(x);
        updateHeight(y);
        rotationCount++;
        return y;                   // y is the new root of this subtree
    }

    // ##########################################################
    // #  MEMBER 2  :  insertHelper (balancing), findNode, depthSum, checkBalance, traversal helpers
    // ##########################################################
    // ---------- insertion ----------

    AVLNode* insertHelper(AVLNode* node, int key, Parcel* parcel) {
        // Step 1: normal BST insertion
        if (node == NULL) {
            AVLNode* newNode = new AVLNode;
            newNode->key = key;
            newNode->parcel = parcel;
            newNode->left = NULL;
            newNode->right = NULL;
            newNode->height = 1;
            return newNode;
        }
        if (key < node->key) node->left = insertHelper(node->left, key, parcel);
        else node->right = insertHelper(node->right, key, parcel);

        // Step 2: update the height of this node
        updateHeight(node);

        // Step 3: check the balance and rotate when needed
        int balance = balanceFactor(node);

        if (balance > 1) {                                  // left side is too tall
            if (balanceFactor(node->left) < 0) {            // Left-Right case
                node->left = rotateLeft(node->left);
            }
            return rotateRight(node);                       // Left-Left case
        }
        if (balance < -1) {                                 // right side is too tall
            if (balanceFactor(node->right) > 0) {           // Right-Left case
                node->right = rotateRight(node->right);
            }
            return rotateLeft(node);                        // Right-Right case
        }
        return node;                                        // already balanced
    }

    // ---------- searching ----------

    AVLNode* findNode(int key, int& steps) {
        AVLNode* current = root;
        steps = 0;
        while (current != NULL) {
            steps++;
            if (key == current->key) return current;
            if (key < current->key) current = current->left;
            else current = current->right;
        }
        return NULL;
    }

    // ---------- other helpers ----------

    int depthSum(AVLNode* node, int depth) {
        if (node == NULL) return 0;
        return depth + depthSum(node->left, depth + 1) + depthSum(node->right, depth + 1);
    }

    // Returns the height of the subtree, or -1 if some node breaks the AVL rule.
    int checkBalance(AVLNode* node) {
        if (node == NULL) return 0;
        int leftHeight = checkBalance(node->left);
        int rightHeight = checkBalance(node->right);
        if (leftHeight == -1 || rightHeight == -1) return -1;
        int difference = leftHeight - rightHeight;
        if (difference > 1 || difference < -1) return -1;
        return 1 + maxOf(leftHeight, rightHeight);
    }

    void inorderRecords(AVLNode* node) {
        if (node == NULL) return;
        inorderRecords(node->left);
        printParcelRow(node->parcel);
        inorderRecords(node->right);
    }

    void inorderKeys(AVLNode* node) {
        if (node == NULL) return;
        inorderKeys(node->left);
        cout << node->key << " ";
        inorderKeys(node->right);
    }

    void preorderKeys(AVLNode* node) {
        if (node == NULL) return;
        cout << node->key << " ";
        preorderKeys(node->left);
        preorderKeys(node->right);
    }

    void postorderKeys(AVLNode* node) {
        if (node == NULL) return;
        postorderKeys(node->left);
        postorderKeys(node->right);
        cout << node->key << " ";
    }

    // ##########################################################
    // #  MEMBER 3  :  sidewaysHelper (tree drawing), insert, search, destroy, constructor, destructor, getters, isBalanced, averageSearchSteps, show functions
    // ##########################################################
    // Prints the tree turned 90 degrees: the root is at the left edge, bigger keys are drawn
    // ABOVE their parent (/--), smaller keys BELOW their parent (\--) and | joins a child to its parent.
    //        /-- 20
    //   10   (root)
    //        \-- 5
    // 'prefix' is the text in front of the node (it carries the | lines of the ancestors).
    void sidewaysHelper(AVLNode* node, string prefix, bool isRoot, bool isRightChild) {
        if (node == NULL) return;

        // 1. bigger keys first (they appear on top)
        string abovePrefix = prefix;
        if (!isRoot) abovePrefix += (isRightChild ? "    " : "|   ");
        sidewaysHelper(node->right, abovePrefix, false, true);

        // 2. the node itself
        if (isRoot) {
            cout << prefix << YELLOW << node->key << RESET << CYAN << "   (root)" << RESET << endl;
        } else {
            cout << CYAN << prefix << (isRightChild ? "/-- " : "\\-- ") << RESET
                 << GREEN << node->key << RESET << endl;
        }

        // 3. smaller keys last (they appear below)
        string belowPrefix = prefix;
        if (!isRoot) belowPrefix += (isRightChild ? "|   " : "    ");
        sidewaysHelper(node->left, belowPrefix, false, false);
    }

    // ---------- normal drawing: root on top, children below ----------
    //   /  = line to the LEFT child (smaller key)
    //   \  = line to the RIGHT child (bigger key)
    // Every subtree gets a block of the board: [left block][key][right block].
    // The key sits between its two blocks, so the tree never overlaps itself.
    int keyLength(int key) {
        char buf[16];
        return sprintf(buf, "%d", key);             // number of characters of the key
    }

    // Width (in characters) of the block of this subtree.
    int blockWidth(AVLNode* node) {
        if (node == NULL) return 0;
        return blockWidth(node->left) + keyLength(node->key) + blockWidth(node->right);
    }

    // Column (inside its block) where the middle of this node's key is.
    int rootOffset(AVLNode* node) {
        return blockWidth(node->left) + keyLength(node->key) / 2;
    }

    int drawLevels(AVLNode* node) {
        if (node == NULL) return 0;
        int l = drawLevels(node->left);
        int r = drawLevels(node->right);
        if (l > r) return l + 1;
        return r + 1;
    }

    // Writes this node (and everything below it) into the board.
    // 'row' = line of the board, 'col' = first column of this node's block.
    void drawNode(AVLNode* node, int row, int col, char** board) {
        int leftWidth = blockWidth(node->left);
        int len = keyLength(node->key);
        char buf[16];
        sprintf(buf, "%d", node->key);
        for (int j = 0; j < len; j++) board[row][col + leftWidth + j] = buf[j];

        if (node->left != NULL) {
            int childRoot = col + rootOffset(node->left);
            for (int x = childRoot + 1; x < col + leftWidth; x++) board[row][x] = '_';
            board[row + 1][childRoot] = '/';
            drawNode(node->left, row + 2, col, board);
        }
        if (node->right != NULL) {
            int rightStart = col + leftWidth + len;
            int childRoot = rightStart + rootOffset(node->right);
            for (int x = rightStart; x < childRoot; x++) board[row][x] = '_';
            board[row + 1][childRoot] = '\\';
            drawNode(node->right, row + 2, rightStart, board);
        }
    }

    void drawTopDown() {
        int width = blockWidth(root);
        int rows = 2 * drawLevels(root) - 1;        // a key line + a / \ line between two levels

        char** board = new char*[rows];
        for (int r = 0; r < rows; r++) {
            board[r] = new char[width + 1];
            for (int c = 0; c < width; c++) board[r][c] = ' ';
            board[r][width] = '\0';
        }
        drawNode(root, 0, 0, board);

        for (int r = 0; r < rows; r++) {
            int last = width - 1;
            while (last >= 0 && board[r][last] == ' ') last--;
            cout << "  ";
            for (int c = 0; c <= last; c++) {
                char ch = board[r][c];
                if (ch >= '0' && ch <= '9') cout << (r == 0 ? YELLOW : GREEN) << ch << RESET;   // root = yellow
                else if (ch == ' ') cout << ch;
                else cout << CYAN << ch << RESET;                                                // / \ _
            }
            cout << endl;
        }
        cout << CYAN << "  (top = root,  / = left child (smaller),  \\ = right child (bigger))" << RESET << endl;

        for (int r = 0; r < rows; r++) delete[] board[r];
        delete[] board;
    }

    void destroy(AVLNode* node) {
        if (node == NULL) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    ParcelAVL() {
        root = NULL;
        nodeCount = 0;
        rotationCount = 0;
    }

    ~ParcelAVL() {
        destroy(root);
    }

    // Adds a key and keeps the tree balanced. Returns false if the key is already in the tree.
    bool insert(int key, Parcel* parcel) {
        int steps = 0;
        if (findNode(key, steps) != NULL) return false;
        root = insertHelper(root, key, parcel);
        nodeCount++;
        return true;
    }

    // Finds a parcel by tracking ID (NULL if it is not in the tree).
    Parcel* search(int key, int& steps) {
        AVLNode* node = findNode(key, steps);
        if (node == NULL) return NULL;
        return node->parcel;
    }

    int stepsToFind(int key) {
        int steps = 0;
        findNode(key, steps);
        return steps;
    }

    int getCount() { return nodeCount; }
    int getHeight() { return heightOf(root); }
    int getRotationCount() { return rotationCount; }

    double averageSearchSteps() {
        if (nodeCount == 0) return 0;
        return (double)depthSum(root, 1) / nodeCount;
    }

    // true when every node satisfies the AVL rule (used for checking and testing)
    bool isBalanced() {
        return checkBalance(root) != -1;
    }

    // ---------- traversals ----------
    void displayRecords() {                         // full parcel rows, sorted by tracking ID
        printParcelHeader();
        inorderRecords(root);
    }
    void showInorder()   { inorderKeys(root);   cout << endl; }
    void showPreorder()  { preorderKeys(root);  cout << endl; }
    void showPostorder() { postorderKeys(root); cout << endl; }

    void showStructure() {
        if (root == NULL) {
            cout << "  (empty tree)\n";
            return;
        }
        if (nodeCount <= 15) drawTopDown();         // small tree: normal drawing, root on top
        else sidewaysHelper(root, "  ", true, false);   // big tree: turned 90 degrees (fits the console)
    }
};

#endif
