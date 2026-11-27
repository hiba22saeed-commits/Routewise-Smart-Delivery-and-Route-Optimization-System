// ============================================================
//  ParcelBST.h  -  Module 4: ordered parcel index in a BINARY SEARCH TREE
//
//  Rule of a BST: for every node
//        all keys in the LEFT subtree  are smaller
//        all keys in the RIGHT subtree are bigger
//  The key is the parcel tracking ID.
//
//  Weak point: the tree is NOT balanced. If IDs arrive in sorted order
//  (101, 102, 103 ...) the tree becomes a long chain and searching is as
//  slow as a linked list. The AVL tree (ParcelAVL.h) fixes this.
//
//  Height is counted in LEVELS: empty tree = 0, one node = 1.
//  The nodes store only POINTERS to parcels (parcels belong to the hash table).
// ============================================================
#ifndef PARCEL_BST_H
#define PARCEL_BST_H

#include <iostream>
#include <string>
#include <cstdio>
#include "Models.h"
using namespace std;

// ##########################################################
// #  MEMBER 3  :  insertHelper, findNode, BSTNode struct, heightHelper, depthSum
// ##########################################################
struct BSTNode {
    int key;                // tracking ID
    Parcel* parcel;         // pointer to the parcel record
    BSTNode* left;          // smaller keys
    BSTNode* right;         // bigger keys
};

class ParcelBST {
private:
    BSTNode* root;
    int nodeCount;

    // ---------- private helper functions (they work with recursion) ----------

    BSTNode* insertHelper(BSTNode* node, int key, Parcel* parcel) {
        if (node == NULL) {                         // found the empty place: create the node here
            BSTNode* newNode = new BSTNode;
            newNode->key = key;
            newNode->parcel = parcel;
            newNode->left = NULL;
            newNode->right = NULL;
            return newNode;
        }
        if (key < node->key) node->left = insertHelper(node->left, key, parcel);
        else node->right = insertHelper(node->right, key, parcel);
        return node;
    }

    // Looks for a key. 'steps' counts how many nodes were visited.
    BSTNode* findNode(int key, int& steps) {
        BSTNode* current = root;
        steps = 0;
        while (current != NULL) {
            steps++;
            if (key == current->key) return current;
            if (key < current->key) current = current->left;
            else current = current->right;
        }
        return NULL;
    }

    int heightHelper(BSTNode* node) {
        if (node == NULL) return 0;
        int leftHeight = heightHelper(node->left);
        int rightHeight = heightHelper(node->right);
        if (leftHeight > rightHeight) return leftHeight + 1;
        return rightHeight + 1;
    }

    // Adds up the "number of steps needed to find each node" (the root needs 1 step).
    int depthSum(BSTNode* node, int depth) {
        if (node == NULL) return 0;
        return depth + depthSum(node->left, depth + 1) + depthSum(node->right, depth + 1);
    }

    // ##########################################################
    // #  MEMBER 1  :  checkBalance, inorderKeys, inorderRecords, preorderKeys, postorderKeys
    // ##########################################################
    // Returns the height of the subtree, or -1 if some node breaks the AVL balance rule.
    // (A plain BST does not keep this rule, but we can still CHECK it for the comparison.)
    int checkBalance(BSTNode* node) {
        if (node == NULL) return 0;
        int leftHeight = checkBalance(node->left);
        int rightHeight = checkBalance(node->right);
        if (leftHeight == -1 || rightHeight == -1) return -1;
        int difference = leftHeight - rightHeight;
        if (difference > 1 || difference < -1) return -1;
        if (leftHeight > rightHeight) return leftHeight + 1;
        return rightHeight + 1;
    }

    void inorderRecords(BSTNode* node) {
        if (node == NULL) return;
        inorderRecords(node->left);
        printParcelRow(node->parcel);
        inorderRecords(node->right);
    }

    void inorderKeys(BSTNode* node) {               // left, node, right  -> sorted order
        if (node == NULL) return;
        inorderKeys(node->left);
        cout << node->key << " ";
        inorderKeys(node->right);
    }

    void preorderKeys(BSTNode* node) {              // node, left, right
        if (node == NULL) return;
        cout << node->key << " ";
        preorderKeys(node->left);
        preorderKeys(node->right);
    }

    void postorderKeys(BSTNode* node) {             // left, right, node
        if (node == NULL) return;
        postorderKeys(node->left);
        postorderKeys(node->right);
        cout << node->key << " ";
    }

    // ##########################################################
    // #  MEMBER 2  :  sidewaysHelper (tree drawing), insert, search, destroy, constructor, destructor, getters, isBalanced, averageSearchSteps, show functions
    // ##########################################################
    // Prints the tree turned 90 degrees: the root is at the left edge, bigger keys are drawn
    // ABOVE their parent (/--), smaller keys BELOW their parent (\--) and | joins a child to its parent.
    //        /-- 20
    //   10   (root)
    //        \-- 5
    // 'prefix' is the text in front of the node (it carries the | lines of the ancestors).
    void sidewaysHelper(BSTNode* node, string prefix, bool isRoot, bool isRightChild) {
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
    int blockWidth(BSTNode* node) {
        if (node == NULL) return 0;
        return blockWidth(node->left) + keyLength(node->key) + blockWidth(node->right);
    }

    // Column (inside its block) where the middle of this node's key is.
    int rootOffset(BSTNode* node) {
        return blockWidth(node->left) + keyLength(node->key) / 2;
    }

    int drawLevels(BSTNode* node) {
        if (node == NULL) return 0;
        int l = drawLevels(node->left);
        int r = drawLevels(node->right);
        if (l > r) return l + 1;
        return r + 1;
    }

    // Writes this node (and everything below it) into the board.
    // 'row' = line of the board, 'col' = first column of this node's block.
    void drawNode(BSTNode* node, int row, int col, char** board) {
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

    // Deletes all nodes (children first, then the node itself).
    void destroy(BSTNode* node) {
        if (node == NULL) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    ParcelBST() {
        root = NULL;
        nodeCount = 0;
    }

    ~ParcelBST() {
        destroy(root);
    }

    // Adds a key. Returns false if the key is already in the tree.
    bool insert(int key, Parcel* parcel) {
        int steps = 0;
        if (findNode(key, steps) != NULL) return false;
        root = insertHelper(root, key, parcel);
        nodeCount++;
        return true;
    }

    // Finds a parcel by tracking ID (NULL if it is not in the tree).
    Parcel* search(int key, int& steps) {
        BSTNode* node = findNode(key, steps);
        if (node == NULL) return NULL;
        return node->parcel;
    }

    // How many steps does a search for this key need?
    int stepsToFind(int key) {
        int steps = 0;
        findNode(key, steps);
        return steps;
    }

    int getCount() { return nodeCount; }
    int getHeight() { return heightHelper(root); }

    // true when every node satisfies the AVL rule (a BST only has this by luck)
    bool isBalanced() {
        return checkBalance(root) != -1;
    }

    // Average number of steps of a successful search (add up all depths, divide by node count).
    double averageSearchSteps() {
        if (nodeCount == 0) return 0;
        return (double)depthSum(root, 1) / nodeCount;
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
