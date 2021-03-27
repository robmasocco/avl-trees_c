/* Roberto Masocco
 * Creation Date: 6/8/2018
 * Latest Version: 26/7/2019
 * ----------------------------------------------------------------------------
 * This file contains type definitions and declarations for the AVL Tree data
 * structure. See the source file for brief descriptions of what each function
 * does. Note that functions which names start with "_" are meant for internal
 * use only, and only those without it should be used by the actual programmer.
 * Many functions require dynamic memory allocation in the heap, and many
 * exposed methods require pointers or return some which refer to the heap: see
 * the source file to understand what needs to be freed after use.
 */
/* This code is released under the MIT license.
 * See the attached LICENSE.txt file.
 */

#ifndef _AVLTREES_STRINGKEYS_H
#define _AVLTREES_STRINGKEYS_H

/* These options can be OR'd in a call to the delete functions to specify
 * if also the keys and/or the data in the nodes must be freed in the heap.
 * If nothing is specified, only the nodes are freed.
 */
#define DELETE_FREE_DATA 0x1
#define DELETE_FREE_KEYS 0x2

/* These options can be specified to tell the search functions what data to
 * return from the trees.
 * Only one at a time is allowed.
 */
#define SEARCH_DATA 0x4
#define SEARCH_KEYS 0x8
#define SEARCH_NODES 0x10

/* These options can be used to specify the desired kind of depth-first search.
 * Only one at a time is allowed.
 */
#define DFS_PRE_ORDER 0x20
#define DFS_IN_ORDER 0x40
#define DFS_POST_ORDER 0x80

/* These options can be used to specify the desired kind of breadth-first
 * search. Only one at a time is allowed.
 */
#define BFS_LEFT_FIRST 0x100
#define BFS_RIGHT_FIRST 0x200

/* An AVL Tree's node stores pointers to its "father" node and to its sons.
 * To calculate the balance factor, the height of the node is also stored.
 * In this implementation, ASCII strings are used as keys in the dictionary,
 * but there are no buffers for those so pointers must be provided.
 * The data kept inside the node can be everything, as long as it's at most
 * 64-bits wide. These can be pointers, too.
 * Note that, as per the deletion options, is not possible to have only SOME
 * data/keys in the heap: either all or none, so think about the data you're
 * providing to these functions.
 */
typedef struct _avlStrNode {
    struct _avlStrNode *_father;
    struct _avlStrNode *_leftSon;
    struct _avlStrNode *_rightSon;
    int _height;
    char *_key;
    void *_data;
} AVLStrNode;

/* An AVL Tree stores a pointer to its root node and a counter which keeps
 * track of the number of nodes in the structure, to get an idea of its "size"
 * and be able to efficiently perform searches.
 * AVL trees implemented like this have a size limit set by the maximum
 * amount representable with an unsigned long integer, automatically set (as
 * long as you compile this code on the same machine you're going to use it on).
 */
typedef struct {
    AVLStrNode *_root;
    unsigned long int nodesCount;
    unsigned long int maxNodes;
} AVLStrTree;

/* Library functions. */
AVLStrTree *createStrTree(void);
int deleteStrTree(AVLStrTree *tree, int opts);
void *strSearch(AVLStrTree *tree, char *key, int opts);
unsigned long int strInsert(AVLStrTree *tree, char *newKey, void *newData);
int strDelete(AVLStrTree *tree, char *key, int opts);
void **strDFS(AVLStrTree *tree, int type, int opts);
void **strBFS(AVLStrTree *tree, int type, int opts);

#endif
