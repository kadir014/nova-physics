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
 * @brief Bounding Volume Hierarchy tree context.
 */
typedef struct {
    struct _nvBVHNode *nodes; /**< Flat array of nodes. */
    size_t node_count; /**< Amount of nodes. */
    size_t node_max; /**< Allocated count for the nodes array. */
    nvArray *bodies; /**< Points to space's body array. Must be untouched. */
    size_t *children; /**< Children indices for bodies. */
} nvBVHContext;

/**
 * @brief Bounding Volume Hierarchy tree node struct.
 */
struct _nvBVHNode {
    nvBVHContext *context; /**< BVH-tree context. */
    nv_bool is_leaf; /**< Is this node a leaf node? */
    size_t left; /**< Left branch of this node. */
    size_t right; /**< Right branch of this node. */
    nvAABB aabb; /**< Boundary of this node. */
    size_t start_i; /**< Starting index of this node. */
    size_t n_children; /**< Number of children this node has. */
};

typedef struct _nvBVHNode nvBVHNode;

/**
 * @brief Create a new BVH node.
 * 
 * Returns `(size_t)(-1)` on error. Use @ref nv_get_error to get more information.
 * 
 * @param context BVH-tree context
 * @param is_leaf Is this node leaf?
 * @param start_i Starting index of this node
 * @param n_children Number of children this node has
 * @return size_t
 */
size_t nvBVHNode_new(
    nvBVHContext *context,
    nv_bool is_leaf,
    size_t start_i,
    size_t n_children
);

/**
 * @brief Build this node's AABB from its bodies.
 * 
 * @param node_index BVH node index
 * @param context BVH context
 */
void nvBVHNode_build_aabb(size_t node_index, nvBVHContext *context);

/**
 * @brief Subdivide this node into two branch nodes.
 * 
 * Returns non-zero on error. Use @ref nv_get_error to get more information.
 * 
 * @param node_index BVH node index
 * @param context BVH context
 * @return int
 */
int nvBVHNode_subdivide(size_t node_index, nvBVHContext *context);

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
 * @brief Create & build a new BVH tree and return the root node.
 * 
 * Returns `NULL` on error. Use @ref nv_get_error to get more information.
 * 
 * @param context BVH-tree context.
 * @return nvBVHNode * 
 */
nvBVHNode *nvBVHTree_new(nvBVHContext *context);


#endif