/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_BOUNDING_VOLUME_HIERARCHY_TREE_H
#define NOVAPHYSICS_BOUNDING_VOLUME_HIERARCHY_TREE_H

#include "novaphysics/internal.h"
#include "novaphysics/core/array.h"
#include "novaphysics/aabb.h"
#include "novaphysics/body.h"


/**
 * @file bvh.h
 * 
 * @brief Bounding Volume Hierarchy Tree implementation.
 */


struct _nvBVHNode;

/**
 * @brief Bounding Volume Hierarchy tree node struct.
 */
struct _nvBVHNode {
    nv_bool is_leaf; /**< Is this node a leaf node? */
    struct _nvBVHNode *left; /**< Left branch of this node. */
    struct _nvBVHNode *right; /**< Right branch of this node. */
    nvAABB aabb; /**< Boundary of this node. */
    nvArray *bodies; /**< Array of bodies, untouched. */
    size_t *children; /**< Array of indices to bodies, altered. */
    size_t start_i; /**< Starting index of this node. */
    size_t n_children; /**< Number of children this node has. */
    size_t depth; // TODO: This is for debugging, remove!
};

typedef struct _nvBVHNode nvBVHNode;

/**
 * @brief Create a new BVH node.
 * 
 * @param is_leaf Is this node leaf?
 * @param bodies Array of bodies
 * @param children Children indices
 * @param start_i Starting index of this node
 * @param n_children Number of children this node has
 * @return nvBVHNode *
 */
nvBVHNode *nvBVHNode_new(
    nv_bool is_leaf,
    nvArray *bodies,
    size_t *children,
    size_t start_i,
    size_t n_children
);

/**
 * @brief Free BVH node.
 * 
 * @param node BVH node
 */
void nvBVHNode_free(nvBVHNode *node);

/**
 * @brief Build this node's AABB from its bodies.
 * 
 * @param node BVH node
 */
void nvBVHNode_build_aabb(nvBVHNode *node);

/**
 * @brief Subdivide this node into two branch nodes.
 * 
 * @param node BVH node
 */
void nvBVHNode_subdivide(nvBVHNode *node);

/**
 * @brief Traverse trough the BVH tree and find collided bodies.
 */
void nvBVHNode_collide(nvBVHNode *node, nvAABB aabb, nvArray *collided);

/**
 * @brief Get the size of the BVH tree.
 * 
 * @note If you don't pass the root node only the branching nodes will be calculated.
 * 
 * @param node BVH ndoe
 * @return size_t 
 */
size_t nvBVHNode_size(nvBVHNode *node);

/**
 * @brief Get the total amount of memory used by this node instance in bytes.
 * 
 * @param node BVH node
 * @return size_t 
 */
size_t nvBVHNode_total_memory_used(nvBVHNode *node);

/**
 * @brief Create & build a new BVH tree and return the root node.
 * 
 * @param bodies Array of bodies
 * @param children Children indices
 * @param n_children Number of children
 * @return nvBVHNode * 
 */
nvBVHNode *nvBVHTree_new(nvArray *bodies, size_t *children, size_t children_n);

/**
 * @brief Free BVH tree.
 * 
 * @param root Root node
 */
void nvBVHTree_free(nvBVHNode *root);


#endif