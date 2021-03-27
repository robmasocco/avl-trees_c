/* Roberto Masocco
 * Creation Date: 6/8/2018
 * Latest Version: 26/7/2019
 * ----------------------------------------------------------------------------
 * This is the main source file for the AVL Trees library.
 * See the comments above each function definition for information about what
 * each one does. See the library header file for a brief description of the
 * "AVL Tree" data type.
 */
/* This code is released under the MIT license.
 * See the attached LICENSE file.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "AVLTree_StringKeys.h"

/* Macro to find the maximum between two integers. */
#define MAX(X, Y) ((X) <= (Y) ? (Y) : (X))

/* Internal library subroutines declarations. */
AVLStrNode *_createStrNode(char *newKey, void *newData);
void _deleteStrNode(AVLStrNode *node);
AVLStrNode *_searchStrNode(AVLStrTree *tree, char *key);
void _strInsertAsLeftSubtree(AVLStrNode *father, AVLStrNode *newSon);
void _strInsertAsRightSubtree(AVLStrNode *father, AVLStrNode *newSon);
AVLStrNode *_strCutLeftSubtree(AVLStrNode *father);
AVLStrNode *_strCutRightSubtree(AVLStrNode *father);
AVLStrNode *_strCutSubtree(AVLStrNode *node);
AVLStrNode *_strMaxKeySon(AVLStrNode *node);
AVLStrNode *_strCutOneSonNode(AVLStrNode *node);
int _strHeight(AVLStrNode *node);
void _strSetHeight(AVLStrNode *node, int newHeight);
void _strSwapInfo(AVLStrNode *node1, AVLStrNode *node2);
int _strBalanceFactor(AVLStrNode *node);
void _strUpdateHeight(AVLStrNode *node);
void _strRightRotation(AVLStrNode *node);
void _strLeftRotation(AVLStrNode *node);
void _strRotate(AVLStrNode *node);
void _strBalanceInsert(AVLStrNode *newNode);
void _strBalanceDelete(AVLStrNode *remFather);
void _strInODFS(AVLStrNode *rootNode, void ***intPtr, int intOpt);
void _strPreODFS(AVLStrNode *rootNode, void ***intPtr, int intOpt);
void _strPostODFS(AVLStrNode *rootNode, void ***intPtr, int intOpt);


// USER FUNCTIONS //
/* Creates a new AVL Tree in the heap. */
AVLStrTree *createStrTree(void) {
    AVLStrTree *newTree = (AVLStrTree *) malloc(sizeof(AVLStrTree));
    if (newTree == NULL) return NULL;
    newTree->_root = NULL;
    newTree->nodesCount = 0;
    newTree->maxNodes = ULONG_MAX;
    return newTree;
}

/* Frees a given AVL Tree from the heap. Using options defined in the header,
 * it's possible to specify whether also keys and/or data have to be freed or
 * not.
 */
int deleteStrTree(AVLStrTree *tree, int opts) {
    // Sanity check on input arguments.
    if (tree == NULL) return -1;
    if (opts < 0) return -1;
    // If the tree is empty free it directly.
    if (tree->_root == NULL) {
        free(tree);
        return 0;
    }
    // Do a BFS to get all the nodes (less taxing on memory than a DFS).
    AVLStrNode **nodes = (AVLStrNode **) strBFS(tree, BFS_LEFT_FIRST,
                                                SEARCH_NODES);
    // Free the nodes and eventually their keys/data.
    for (unsigned long int i = 0; i < tree->nodesCount; i++) {
        if (opts & DELETE_FREE_KEYS) free((*(nodes[i]))._key);
        if (opts & DELETE_FREE_DATA) free((*(nodes[i]))._data);
        _deleteStrNode(nodes[i]);
    }
    // Free the nodes array and the tree, and that's it!
    free(nodes);
    free(tree);
    return 0;
}

/* Searches for an entry with the specified key in the tree. */
void *strSearch(AVLStrTree *tree, char *key, int opts) {
    if ((opts <= 0) || (tree == NULL)) return NULL;  // Sanity check.
    AVLStrNode *searchedNode = _searchStrNode(tree, key);
    if (searchedNode != NULL) {
        if (opts & SEARCH_DATA) return searchedNode->_data;
        if (opts & SEARCH_NODES) return searchedNode;
        return NULL;
    }
    return NULL;
}

/* Deletes an entry from the tree. */
int strDelete(AVLStrTree *tree, char *key, int opts) {
    // Sanity check on input arguments.
    if ((opts < 0) || (key == NULL) || (tree == NULL)) return 0;
    AVLStrNode *toDelete = _searchStrNode(tree, key);
    AVLStrNode *toFree;
    if (toDelete != NULL) {
        // Check whether the node has no sons or even one.
        if ((toDelete->_leftSon == NULL) || (toDelete->_rightSon == NULL)) {
            toFree = _strCutOneSonNode(toDelete);
        } else {
            // Find the node's predecessor and swap the content.
            AVLStrNode *maxLeft = _strMaxKeySon(toDelete->_leftSon);
            _strSwapInfo(toDelete, maxLeft);
            // Remove the original predecessor.
            toFree = _strCutOneSonNode(maxLeft);
        }
        // Apply eventual options to free keys and data, then free the node.
        if (opts & DELETE_FREE_DATA) free(toFree->_data);
        if (opts & DELETE_FREE_KEYS) free(toFree->_key);
        free(toFree);
        tree->nodesCount--;
        // Check if the tree is now empty and update root pointer.
        if (tree->nodesCount == 0) tree->_root = NULL;
        return 1;  // Found and deleted.
    }
    return 0;  // Not found.
}

/* Creates and inserts a new node in the tree. */
unsigned long int strInsert(AVLStrTree *tree, char *newKey, void *newData) {
    if ((newKey == NULL) || (tree == NULL)) return 0;  // Sanity check.
    if (tree->nodesCount == tree->maxNodes) return 0;  // The tree is full.
    AVLStrNode *newNode = _createStrNode(newKey, newData);
    if (tree->_root == NULL) {
        // The tree is empty.
        tree->_root = newNode;
        tree->nodesCount++;
    } else {
        // Look for the correct position and place it there.
        AVLStrNode *curr = tree->_root;
        AVLStrNode *pred = NULL;
        int comp;
        while (curr != NULL) {
            pred = curr;
            comp = strcmp(curr->_key, newKey);
            if (comp >= 0) {
                // Equals are kept in the left subtree.
                curr = curr->_leftSon;
            } else {
                curr = curr->_rightSon;
            }
        }
        comp = strcmp(pred->_key, newKey);
        if (comp >= 0) {
            _strInsertAsLeftSubtree(pred, newNode);
        } else {
            _strInsertAsRightSubtree(pred, newNode);
        }
        _strBalanceInsert(newNode);
        tree->nodesCount++;
    }
    return tree->nodesCount;  // Return the result of the insertion.
}

/* Performs a depth-first search of the tree, the type of which can be
 * specified using the options defined in the header.
 * Depending on the option specified, returns an array of:
 * - Pointers to the nodes.
 * - Keys.
 * - Data.
 * See the header for the definitions of such options.
 * Remember to free the returned array afterwards!
 */
void **strDFS(AVLStrTree *tree, int type, int opts) {
    // Sanity check for the input arguments.
    if ((type <= 0) || (opts <= 0)) return NULL;
    if ((tree == NULL) || (tree->_root == NULL)) return NULL;
    // Allocate memory according to options.
    void **dfsRes;
    int intOpt;
    if (opts & SEARCH_DATA) {
        intOpt = SEARCH_DATA;
        dfsRes = calloc(tree->nodesCount, sizeof(void *));
    } else if (opts & SEARCH_KEYS) {
        intOpt = SEARCH_KEYS;
        dfsRes = calloc(tree->nodesCount, sizeof(char *));
    } else if (opts & SEARCH_NODES) {
        intOpt = SEARCH_NODES;
        dfsRes = calloc(tree->nodesCount, sizeof(AVLStrNode *));
    } else return NULL;  // Invalid option.
    if (dfsRes == NULL) return NULL;  // calloc failed.
    // Launch the requested DFS according to type.
    void **intPtr = dfsRes;
    if (type & DFS_PRE_ORDER) {
        _strPreODFS(tree->_root, &intPtr, intOpt);
    } else if (type & DFS_IN_ORDER) {
        _strInODFS(tree->_root, &intPtr, intOpt);
    } else if (type & DFS_POST_ORDER) {
        _strPostODFS(tree->_root, &intPtr, intOpt);
    } else {
        // Invalid type.
        free(dfsRes);
        return NULL;
    }
    // The array is now filled with the requested data.
    return dfsRes;
}

/* Performs a breadth-first search of the tree, the type of which can be
 * specified using the options defined in the header (left or right son
 * visited first).
 * Depending on the option specified, returns an array of:
 * - Pointers to the nodes.
 * - Keys.
 * - Data.
 * See the header for the definitions of such options.
 * Remember to free the returned array afterwards!
 */
void **strBFS(AVLStrTree *tree, int type, int opts) {
    // Sanity check on input arguments.
    if ((tree == NULL) || (tree->_root == NULL) ||
        (type <= 0) || (opts <= 0) ||
        !((type & BFS_LEFT_FIRST) || (type & BFS_RIGHT_FIRST)) ||
        !((opts & SEARCH_KEYS) || (opts & SEARCH_DATA) ||
        (opts & SEARCH_NODES))) return NULL;
    // Allocate memory in the heap.
    void **bfsRes = NULL;
    void **intPtr;
    if (opts & SEARCH_DATA) {
        bfsRes = calloc(tree->nodesCount, sizeof(void *));
    } else if (opts & SEARCH_KEYS) {
        bfsRes = calloc(tree->nodesCount, sizeof(char *));
    } else if (opts & SEARCH_NODES) {
        bfsRes = calloc(tree->nodesCount, sizeof(AVLStrNode *));
    } else return NULL;  // Invalid option.
    if (bfsRes == NULL) return NULL;  // Calloc failed.
    intPtr = bfsRes + 1;
    *bfsRes = (void *) (tree->_root);
    AVLStrNode *curr;
    // Start the visit, using the same array to return as a temporary queue
    // for the nodes.
    for (unsigned long int i = 0; i < tree->nodesCount; i++) {
        curr = (AVLStrNode *) bfsRes[i];
        // Visit the current node.
        if (opts & SEARCH_DATA) {
            bfsRes[i] = curr->_data;
        } else if (opts & SEARCH_KEYS) {
            bfsRes[i] = curr->_key;
        } else if (opts & SEARCH_NODES) {
            bfsRes[i] = curr;
        }
        // Eventually add the sons to the array, to be visited afterwards.
        if (type & BFS_LEFT_FIRST) {
            if (curr->_leftSon != NULL) {
                *intPtr = (void *) (curr->_leftSon);
                intPtr++;
            }
            if (curr->_rightSon != NULL) {
                *intPtr = (void *) (curr->_rightSon);
                intPtr++;
            }
        } else if (type & BFS_RIGHT_FIRST) {
            if (curr->_rightSon != NULL) {
                *intPtr = (void *) (curr->_rightSon);
                intPtr++;
            }
            if (curr->_leftSon != NULL) {
                *intPtr = (void *) (curr->_leftSon);
                intPtr++;
            }
        }
    }
    return bfsRes;
}

// INTERNAL LIBRARY SUBROUTINES //
/* Creates a new AVL node in the heap. Requires a pointer to a key string and
 * some data.
 */
AVLStrNode *_createStrNode(char *newKey, void *newData) {
    AVLStrNode *newNode = (AVLStrNode *) malloc(sizeof(AVLStrNode));
    if (newNode == NULL) return NULL;
    newNode->_father = NULL;
    newNode->_leftSon = NULL;
    newNode->_rightSon = NULL;
    newNode->_key = newKey;
    newNode->_data = newData;
    newNode->_height = 0;
    return newNode;
}

/* Frees memory occupied by a node. */
void _deleteStrNode(AVLStrNode *node) {
    free(node);
}

/* Inserts a subtree rooted in a given node as the left subtree of a given
 * node.
 */
void _strInsertAsLeftSubtree(AVLStrNode *father, AVLStrNode *newSon) {
    if (newSon != NULL) newSon->_father = father;
    father->_leftSon = newSon;
}

/* Inserts a subtree rooted in a given node as the right subtree of a given
 * node.
 */
void _strInsertAsRightSubtree(AVLStrNode *father, AVLStrNode *newSon) {
    if (newSon != NULL) newSon->_father = father;
    father->_rightSon = newSon;
}

/* Cuts and returns the left subtree of a given node. */
AVLStrNode *_strCutLeftSubtree(AVLStrNode *father) {
    AVLStrNode *son = father->_leftSon;
    if (son == NULL) return NULL;  // Sanity check.
    son->_father = NULL;
    father->_leftSon = NULL;
    return son;
}

/* Cuts and returns the right subtree of a given node. */
AVLStrNode *_strCutRightSubtree(AVLStrNode *father) {
    AVLStrNode *son = father->_rightSon;
    if (son == NULL) return NULL;  // Sanity check.
    son->_father = NULL;
    father->_rightSon = NULL;
    return son;
}

/* Cuts and returns the subtree nested in a given node. */
AVLStrNode *_strCutSubtree(AVLStrNode *node) {
    if (node == NULL) return NULL;  // Sanity check.
    if (node->_father == NULL) return node;  // Asked to cut at root.
    AVLStrNode *father = node->_father;
    if ((node->_leftSon == NULL) && (node->_rightSon == NULL)) {
        // The node is a leaf: distinguish between the node being a left or
        // right son and cut accordingly.
        if (father->_rightSon == node) {
            father->_rightSon = NULL;
        } else father->_leftSon = NULL;
        node->_father = NULL;
        return node;
    } else if (father->_leftSon == node) return _strCutLeftSubtree(father);
    return _strCutRightSubtree(father);
}

/* Returns the descendant of a given node with the greatest key. */
AVLStrNode *_strMaxKeySon(AVLStrNode *node) {
    AVLStrNode *curr = node;
    while (curr->_rightSon != NULL) curr = curr->_rightSon;
    return curr;
}

/* Returns a pointer to the node with the specified key, or NULL. */
AVLStrNode *_searchStrNode(AVLStrTree *tree, char *key) {
    if (tree->_root == NULL) return NULL;
    AVLStrNode *curr = tree->_root;
    int comp;
    while (curr != NULL) {
        comp = strcmp(curr->_key, key);
        if (comp > 0) {
            curr = curr->_leftSon;
        } else if (comp < 0) {
            curr = curr->_rightSon;
        } else return curr;
    }
    return NULL;
}

/* Returns the height of a given node. */
int _strHeight(AVLStrNode *node) {
    if (node == NULL) {
        return -1;  // Useful when computing balance factors.
    }
    return node->_height;
}

/* Sets the height of the specified node to the given value. */
void _strSetHeight(AVLStrNode *node, int newHeight) {
    if (node != NULL) node->_height = newHeight;
}

/* Returns the balance factor of a given node. */
int _strBalanceFactor(AVLStrNode *node) {
    if (node == NULL) return 0;  // Consistency check.
    return _strHeight(node->_leftSon) - _strHeight(node->_rightSon);
}

/* Updates the height of a given node. */
void _strUpdateHeight(AVLStrNode *node) {
    if (node != NULL) {
        _strSetHeight(node, MAX(_strHeight(node->_leftSon),
                                _strHeight(node->_rightSon)) + 1);
    }
}

/* Swaps contents between two nodes. */
void _strSwapInfo(AVLStrNode *node1, AVLStrNode *node2) {
    char *key1 = node1->_key;
    void *data1 = node1->_data;
    char *key2 = node2->_key;
    void *data2 = node2->_data;
    node1->_key = key2;
    node2->_key = key1;
    node1->_data = data2;
    node2->_data = data1;
}

/* Performs a simple right rotation at the specified node. */
void _strRightRotation(AVLStrNode *node) {
    AVLStrNode *leftSon = node->_leftSon;
    // Swap the node and its son's contents to make it climb.
    _strSwapInfo(node, leftSon);
    // Shrink the tree portion in subtrees.
    AVLStrNode *rTree = _strCutRightSubtree(node);
    AVLStrNode *lTree = _strCutLeftSubtree(node);
    AVLStrNode *lTree_l = _strCutLeftSubtree(leftSon);
    AVLStrNode *lTree_r = _strCutRightSubtree(leftSon);
    // Recombine portions to respect the search property.
    _strInsertAsRightSubtree(lTree, rTree);
    _strInsertAsLeftSubtree(lTree, lTree_r);
    _strInsertAsRightSubtree(node, lTree);
    _strInsertAsLeftSubtree(node, lTree_l);
    // Update the height of the involved nodes.
    _strUpdateHeight(node->_rightSon);
    _strUpdateHeight(node);
}

/* Performs a simple left rotation at the specified node. */
void _strLeftRotation(AVLStrNode *node) {
    AVLStrNode *rightSon = node->_rightSon;
    // Swap the node and its son's contents to make it climb.
    _strSwapInfo(node, rightSon);
    // Shrink the tree portion in subtrees.
    AVLStrNode *rTree = _strCutRightSubtree(node);
    AVLStrNode *lTree = _strCutLeftSubtree(node);
    AVLStrNode *rTree_l = _strCutLeftSubtree(rightSon);
    AVLStrNode *rTree_r = _strCutRightSubtree(rightSon);
    // Recombine portions to respect the search property.
    _strInsertAsLeftSubtree(rTree, lTree);
    _strInsertAsRightSubtree(rTree, rTree_l);
    _strInsertAsLeftSubtree(node, rTree);
    _strInsertAsRightSubtree(node, rTree_r);
    // Update the height of the involved nodes.
    _strUpdateHeight(node->_leftSon);
    _strUpdateHeight(node);
}

/* Examines the balance factor of a given node and eventually rotates. */
void _strRotate(AVLStrNode *node) {
    int balFactor = _strBalanceFactor(node);
    if (balFactor == 2) {
        if (_strBalanceFactor(node->_leftSon) >= 0) {
            // LL displacement: rotate right.
            _strRightRotation(node);
        } else {
            // LR displacement: apply double rotation.
            _strLeftRotation(node->_leftSon);
            _strRightRotation(node);
        }
    } else if (balFactor == -2) {
        if (_strBalanceFactor(node->_rightSon) <= 0) {
            // RR displacement: rotate left.
            _strLeftRotation(node);
        } else {
            // RL displacement: apply double rotation.
            _strRightRotation(node->_rightSon);
            _strLeftRotation(node);
        }
    }
}

/* Updates heights and looks for displacements following an insertion. */
void _strBalanceInsert(AVLStrNode *newNode) {
    AVLStrNode *curr = newNode->_father;
    while (curr != NULL) {
        if (abs(_strBalanceFactor(curr)) >= 2) {
            // Unbalanced node found.
            break;
        } else {
            _strUpdateHeight(curr);
            curr = curr->_father;
        }
    }
    if (curr != NULL) _strRotate(curr);
}

/* Updates heights and looks for displacements following a deletion. */
void _strBalanceDelete(AVLStrNode *remFather) {
    AVLStrNode *curr = remFather;
    while (curr != NULL) {
        if (abs(_strBalanceFactor(curr)) >= 2) {
            // There may be more than one unbalanced node.
            _strRotate(curr);
        } else _strUpdateHeight(curr);
        curr = curr->_father;
    }
}

/* Cuts a node with a single son. */
AVLStrNode *_strCutOneSonNode(AVLStrNode *node) {
    AVLStrNode *son = NULL;
    AVLStrNode *father = node->_father;
    if (node->_leftSon != NULL) {
        son = node->_leftSon;
    } else if (node->_rightSon != NULL) {
        son = node->_rightSon;
    }
    if (son == NULL) {  // The node is a leaf.
        son = _strCutSubtree(node);  // Will be returned later.
    } else {
        // Swap the content from the son to the father.
        _strSwapInfo(node, son);
        // Cut the node and balance the deletion.
        _strCutSubtree(son);
        _strInsertAsRightSubtree(node, _strCutSubtree(son->_rightSon));
        _strInsertAsLeftSubtree(node, _strCutSubtree(son->_leftSon));
    }
    _strBalanceDelete(father);
    return son;  // Return the node to free, now totally disconnected.
}

/* Performs an in-order, recursive DFS. */
void _strInODFS(AVLStrNode *rootNode, void ***intPtr, int intOpt) {
    // Recursion base step.
    if (rootNode == NULL) {
        *(intPtr) = *(intPtr) - 1;
        return;
    }
    // Recursive step: visit the left son.
    _strInODFS(rootNode->_leftSon, intPtr, intOpt);
    *(intPtr) = *(intPtr) + 1;
    // Now visit the root node.
    if (intOpt & SEARCH_NODES) {
        **(intPtr) = rootNode;
    } else if (intOpt & SEARCH_KEYS) {
        **(intPtr) = rootNode->_key;
    } else if (intOpt & SEARCH_DATA) {
        **(intPtr) = rootNode->_data;
    }
    *(intPtr) = *(intPtr) + 1;
    // Visit the right son and return.
    _strInODFS(rootNode->_rightSon, intPtr, intOpt);
}

/* Performs a pre-order, recursive DFS. */
void _strPreODFS(AVLStrNode *rootNode, void ***intPtr, int intOpt) {
    // Recursion base step.
    if (rootNode == NULL) {
        *(intPtr) = *(intPtr) - 1;
        return;
    }
    // Recursive step.
    // Visit the root node.
    if (intOpt & SEARCH_NODES) {
        **(intPtr) = rootNode;
    } else if (intOpt & SEARCH_KEYS) {
        **(intPtr) = rootNode->_key;
    } else if (intOpt & SEARCH_DATA) {
        **(intPtr) = rootNode->_data;
    }
    *(intPtr) = *(intPtr) + 1;
    // Now visit the left son.
    _strPreODFS(rootNode->_leftSon, intPtr, intOpt);
    *(intPtr) = *(intPtr) + 1;
    // Visit the right son and return.
    _strPreODFS(rootNode->_rightSon, intPtr, intOpt);
}

/* Performs a post-order, recursive DFS. */
void _strPostODFS(AVLStrNode *rootNode, void ***intPtr, int intOpt) {
    // Recursion base step.
    if (rootNode == NULL) {
        *(intPtr) = *(intPtr) - 1;
        return;
    }
    // Recursive step.
    // Visit the left son.
    _strPostODFS(rootNode->_leftSon, intPtr, intOpt);
    *(intPtr) = *(intPtr) + 1;
    // Visit the right son.
    _strPostODFS(rootNode->_rightSon, intPtr, intOpt);
    *(intPtr) = *(intPtr) + 1;
    // Visit the root node and return.
    if (intOpt & SEARCH_NODES) {
        **(intPtr) = rootNode;
    } else if (intOpt & SEARCH_KEYS) {
        **(intPtr) = rootNode->_key;
    } else if (intOpt & SEARCH_DATA) {
        **(intPtr) = rootNode->_data;
    }
}
