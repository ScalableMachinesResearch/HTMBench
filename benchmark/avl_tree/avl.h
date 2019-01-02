//This is from https://github.com/fragglet/c-algorithms/blob/master/src/avl-tree.h
/**
 * To create a new AVL tree, use @ref avl_tree_new.  To destroy
 * an AVL tree, use @ref avl_tree_free.
 *
 * To insert a new key-value pair into an AVL tree, use
 * @ref avl_tree_insert.  To remove an entry from an
 * AVL tree, use @ref avl_tree_remove or @ref avl_tree_remove_node.
 *
 * To search an AVL tree, use @ref avl_tree_lookup or
 * @ref avl_tree_lookup_node.
 *
 * Tree nodes can be queried using the
 * @ref avl_tree_node_child,
 * @ref avl_tree_node_parent,
 * @ref avl_tree_node_key and
 * @ref avl_tree_node_value functions.
 */

#ifndef ALGORITHM_AVLTREE_H
#define ALGORITHM_AVLTREE_H

#if defined(RTM)
#include "tm.h"
#define MY_TM_BEGIN(isWrite) TM_BEGIN()
#define MY_TM_END(isWrite) TM_END()

#elif defined(OMP_LOCK)
#define MY_TM_BEGIN(isWrite) omp_set_lock(&global_lock)
#define MY_TM_END(isWrite) omp_unset_lock(&global_lock);

#else
#define MY_TM_BEGIN(isWrite) _Pragma("omp critical") {
#define MY_TM_END(isWrite) }

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An AVL tree balanced binary tree.
 *
 * @see avl_tree_new
 */

typedef struct _AVLTree AVLTree;

/**
 * A key for an @ref AVLTree.
 */

#define AVLTreeKey int //typedef void *AVLTreeKey;

/**
 * A value stored in an @ref AVLTree.
 */

#define AVLTreeValue int//typedef void *AVLTreeValue;

/**
 * A null @ref AVLTreeValue.
 */

#define AVL_TREE_NULL ((void *) 0)

/**
 * A node in an AVL tree.
 *
 * @see avl_tree_node_left_child
 * @see avl_tree_node_right_child
 * @see avl_tree_node_parent
 * @see avl_tree_node_key
 * @see avl_tree_node_value
 */

typedef struct _AVLTreeNode AVLTreeNode;

/**
 * An @ref AVLTreeNode can have left and right children.
 */

typedef enum {
	AVL_TREE_NODE_LEFT = 0,
	AVL_TREE_NODE_RIGHT = 1
} AVLTreeNodeSide;

/**
 * Type of function used to compare keys in an AVL tree.
 *
 * @param value1           The first key.
 * @param value2           The second key.
 * @return                 A negative number if value1 should be sorted
 *                         before value2, a positive number if value2 should
 *                         be sorted before value1, zero if the two keys
 *                         are equal.
 */

typedef int (*AVLTreeCompareFunc)(AVLTreeKey k1, AVLTreeKey k2);

/**
 * Create a new AVL tree.
 *
 * @param compare_func    Function to use when comparing keys in the tree.
 * @return                A new AVL tree, or NULL if it was not possible
 *                        to allocate the memory.
 */

AVLTree *avl_tree_new(AVLTreeCompareFunc compare_func);

/**
 * Destroy an AVL tree.
 *
 * @param tree            The tree to destroy.
 */

void avl_tree_free(AVLTree *tree);

/**
 * Insert a new key-value pair into an AVL tree.
 *
 * @param tree            The tree.
 * @param key             The key to insert.
 * @param value           The value to insert.
 * @return                The newly created tree node containing the
 *                        key and value, or NULL if it was not possible
 *                        to allocate the new memory.
 */

AVLTreeNode *avl_tree_insert(AVLTree *tree, AVLTreeKey key,
                             AVLTreeValue value);

/**
 * Remove a node from a tree.
 *
 * @param tree            The tree.
 * @param node            The node to remove
 */

void avl_tree_remove_node(AVLTree *tree, AVLTreeNode *node);

/**
 * Remove an entry from a tree, specifying the key of the node to
 * remove.
 *
 * @param tree            The tree.
 * @param key             The key of the node to remove.
 * @return                Zero (false) if no node with the specified key was
 *                        found in the tree, non-zero (true) if a node with
 *                        the specified key was removed.
 */

int avl_tree_remove(AVLTree *tree, AVLTreeKey key);

/**
 * Search an AVL tree for a node with a particular key.  This uses
 * the tree as a mapping.
 *
 * @param tree            The AVL tree to search.
 * @param key             The key to search for.
 * @return                The tree node containing the given key, or NULL
 *                        if no entry with the given key is found.
 */

AVLTreeNode *avl_tree_lookup_node(AVLTree *tree, AVLTreeKey key);

/**
 * Search an AVL tree for a value corresponding to a particular key.
 * This uses the tree as a mapping.  Note that this performs
 * identically to @ref avl_tree_lookup_node, except that the value
 * at the node is returned rather than the node itself.
 *
 * @param tree            The AVL tree to search.
 * @param key             The key to search for.
 * @return                The value associated with the given key, or
 *                        @ref AVL_TREE_NULL if no entry with the given key is
 *                        found.
 */

AVLTreeValue avl_tree_lookup(AVLTree *tree, AVLTreeKey key);

/**
 * Find the root node of a tree.
 *
 * @param tree            The tree.
 * @return                The root node of the tree, or NULL if the tree is
 *                        empty.
 */

AVLTreeNode *avl_tree_root_node(AVLTree *tree);

/**
 * Retrieve the key for a given tree node.
 *
 * @param node            The tree node.
 * @return                The key to the given node.
 */

AVLTreeKey avl_tree_node_key(AVLTreeNode *node);

/**
 * Retrieve the value at a given tree node.
 *
 * @param node            The tree node.
 * @return                The value at the given node.
 */

AVLTreeValue avl_tree_node_value(AVLTreeNode *node);

/**
 * Find the child of a given tree node.
 *
 * @param node            The tree node.
 * @param side            Which child node to get (left or right)
 * @return                The child of the tree node, or NULL if the
 *                        node has no child on the given side.
 */

AVLTreeNode *avl_tree_node_child(AVLTreeNode *node, AVLTreeNodeSide side);

/**
 * Find the parent node of a given tree node.
 *
 * @param node            The tree node.
 * @return                The parent node of the tree node, or NULL if
 *                        this is the root node.
 */

AVLTreeNode *avl_tree_node_parent(AVLTreeNode *node);

/**
 * Find the height of a subtree.
 *
 * @param node            The root node of the subtree.
 * @return                The height of the subtree.
 */

int avl_tree_subtree_height(AVLTreeNode *node);

/**
 * Convert the keys in an AVL tree into a C array.  This allows
 * the tree to be used as an ordered set.
 *
 * @param tree            The tree.
 * @return                A newly allocated C array containing all the keys
 *                        in the tree, in order.  The length of the array
 *                        is equal to the number of entries in the tree
 *                        (see @ref avl_tree_num_entries).
 */

AVLTreeValue *avl_tree_to_array(AVLTree *tree);

/**
 * Retrieve the number of entries in the tree.
 *
 * @param tree            The tree.
 * @return                The number of key-value pairs stored in the tree.
 */

unsigned int avl_tree_num_entries(AVLTree *tree);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_AVLTREE_H */
