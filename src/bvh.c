/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/internal.h"
#include "novaphysics/bvh.h"
#include "novaphysics/math.h"
#include "novaphysics/body.h"
#include "novaphysics/collision.h"


/**
 * @file bvh.c
 * 
 * @brief Bounding Volume Hierarchy Tree implementation.
 */


 size_t nvBVHNode_new(
    nvBVHContext *context,
    nv_bool is_leaf,
    size_t start_i,
    size_t n_children
){
    size_t node_index = context->node_count++;

    if (context->node_count >= context->node_max) {
        context->node_max *= 2;
        context->nodes = NV_REALLOC(context->nodes, sizeof(nvBVHNode) * context->node_max);
        
        if (!context->nodes) {
            nv_set_error("Failed to allocate memory.");
            return (size_t)-1;
        }
    }

    nvBVHNode *node = &context->nodes[node_index];
    node->context = context;

    node->is_leaf = is_leaf;
    node->left = (size_t)-1;
    node->right = (size_t)-1;

    node->start_i = start_i;
    node->n_children = n_children;

    return node_index;
}

void nvBVHNode_build_aabb(size_t node_index, nvBVHContext *context) {
    nvBVHNode *node = &context->nodes[node_index];

    if (node->n_children > 0) {
        node->aabb = (nvAABB){NV_INF, NV_INF, -NV_INF, -NV_INF};

        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvRigidBody *body = node->context->bodies->data[node->context->children[i]];
            nvAABB aabb = nvRigidBody_get_aabb(body);

            node->aabb.min_x = nv_fmin(node->aabb.min_x, aabb.min_x);
            node->aabb.min_y = nv_fmin(node->aabb.min_y, aabb.min_y);
            node->aabb.max_x = nv_fmax(node->aabb.max_x, aabb.max_x);
            node->aabb.max_y = nv_fmax(node->aabb.max_y, aabb.max_y);
        }
    }
}

int nvBVHNode_subdivide(size_t node_index, nvBVHContext *context) {
    nvBVHNode *node = &context->nodes[node_index];
    if (node->is_leaf) return 0;

    nv_float width = node->aabb.max_x - node->aabb.min_x;
    nv_float height = node->aabb.max_y - node->aabb.min_y;

    size_t mid = 0;

    // Current splitting method is midway trough the longest axis

    if (width > height) {
        nv_float split = 0.0;
        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvRigidBody *body = node->context->bodies->data[node->context->children[i]];
            split += body->bvh_median_x;
        }
        split /= (nv_float)node->n_children;
        
        size_t i = node->start_i;
        size_t j = node->start_i + node->n_children - 1;

        while (i <= j) {
            nvRigidBody *body = node->context->bodies->data[node->context->children[i]];
            nv_float c = body->bvh_median_x;

            if (c < split) {
                i++;
            }
            else {
                size_t k = j--;
                size_t temp = node->context->children[i];
                node->context->children[i] = node->context->children[k];
                node->context->children[k] = temp; 
            }
        }

        mid = i;
    }
    else {
        nv_float split = 0.0;
        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvRigidBody *body = node->context->bodies->data[node->context->children[i]];
            split += body->bvh_median_y;
        }
        split /= (nv_float)node->n_children;

        size_t i = node->start_i;
        size_t j = node->start_i + node->n_children - 1;

        while (i <= j) {
            nvRigidBody *body = node->context->bodies->data[node->context->children[i]];
            nv_float c = body->bvh_median_y;

            if (c < split) {
                i++;
            }
            else {
                size_t k = j--;
                size_t temp = node->context->children[i];
                node->context->children[i] = node->context->children[k];
                node->context->children[k] = temp; 
            }
        }

        mid = i;
    }

    // Do not split if one of the sides is empty
    size_t left_n = mid - node->start_i;
    if (left_n == 0 || left_n == node->n_children) {
        node->is_leaf = true;
        return 0;
    }

    size_t right_n = node->n_children - left_n;

    nv_bool left_leaf = left_n <= NV_BVH_LEAF_THRESHOLD;
    nv_bool right_leaf = right_n <= NV_BVH_LEAF_THRESHOLD;

    size_t left_index = nvBVHNode_new(context, left_leaf, node->start_i, left_n);
    if (left_index == (size_t)-1) return 1;

    size_t right_index = nvBVHNode_new(context, right_leaf, mid, right_n);
    if (right_index == (size_t)-1) return 1;

    // Reassigning node because realloc may make the pointer invalid.
    node = &context->nodes[node_index];

    node->left = left_index;
    node->right = right_index;

    nvBVHNode_build_aabb(node->left, context);
    nvBVHNode_build_aabb(node->right, context);

    int left_subdivide = nvBVHNode_subdivide(node->left, context);

    // Recursive subdivide call may realloc again and make the pointer invalid!
    node = &context->nodes[node_index];

    int right_subdivide = nvBVHNode_subdivide(node->right, context);

    if (left_subdivide || right_subdivide) return 1;

    return 0;
}

void nvBVHNode_collide(nvBVHNode *node, nvAABB aabb, nvArray *collided) {
    if (!node) return;

    if (!nv_collide_aabb_x_aabb(node->aabb, aabb)) return;

    if (node->is_leaf) {
        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvArray_add(collided, node->context->bodies->data[node->context->children[i]]);
        }
    }
    else {
        nvBVHNode *left_node = &node->context->nodes[node->left];
        nvBVHNode *right_node = &node->context->nodes[node->right];
        nvBVHNode_collide(left_node, aabb, collided);
        nvBVHNode_collide(right_node, aabb, collided);
    }
}

size_t nvBVHNode_size(nvBVHNode *node) {
    if (!node) return 0;

    if (node->is_leaf) return 1;

    else {
        nvBVHNode *left_node = &node->context->nodes[node->left];
        nvBVHNode *right_node = &node->context->nodes[node->right];
        size_t a = nvBVHNode_size(left_node);
        size_t b = nvBVHNode_size(right_node);
        return 1 + a + b;
    }
}

nvBVHNode *nvBVHTree_new(nvBVHContext *context) {
    size_t root_index = nvBVHNode_new(context, false, 0, context->bodies->size);
    if (root_index == (size_t)-1) return NULL;

    nvBVHNode_build_aabb(root_index, context);
    
    if (nvBVHNode_subdivide(root_index, context)) return NULL;

    return &context->nodes[root_index];
}