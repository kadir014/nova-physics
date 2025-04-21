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


nvBVHNode *nvBVHNode_new(
    nv_bool is_leaf,
    nvArray *bodies,
    size_t *children,
    size_t start_i,
    size_t n_children
) {
    nvBVHNode *node = NV_NEW(nvBVHNode);
    NV_MEM_CHECK(node);

    node->is_leaf = is_leaf;
    node->left = NULL;
    node->right = NULL;

    node->bodies = bodies;
    node->children = children;
    node->start_i = start_i;
    node->n_children = n_children;

    node->depth = 0;

    return node;
}

void nvBVHNode_free(nvBVHNode *node) {
    if (!node) return;

    if (!node->is_leaf) {
        nvBVHNode_free(node->left);
        nvBVHNode_free(node->right);
    }

    NV_FREE(node);
}

void nvBVHNode_build_aabb(nvBVHNode *node) {
    if (!node) return;

    if (node->n_children > 0) {
        node->aabb = (nvAABB){NV_INF, NV_INF, -NV_INF, -NV_INF};

        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvRigidBody *body = node->bodies->data[node->children[i]];
            nvAABB aabb = nvRigidBody_get_aabb(body);

            node->aabb.min_x = nv_fmin(node->aabb.min_x, aabb.min_x);
            node->aabb.min_y = nv_fmin(node->aabb.min_y, aabb.min_y);
            node->aabb.max_x = nv_fmax(node->aabb.max_x, aabb.max_x);
            node->aabb.max_y = nv_fmax(node->aabb.max_y, aabb.max_y);
        }
    }
}

void nvBVHNode_subdivide(nvBVHNode *node) {
    if (!node) return;
    if (node->is_leaf) return;

    nv_float width = node->aabb.max_x - node->aabb.min_x;
    nv_float height = node->aabb.max_y - node->aabb.min_y;

    size_t mid = 0;

    // Current splitting method is midway trough the longest axis

    if (width > height) {
        nv_float split = 0.0;
        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvRigidBody *body = node->bodies->data[node->children[i]];
            split += body->bvh_median_x;
        }
        split /= (nv_float)node->n_children;
        
        size_t i = node->start_i;
        size_t j = node->start_i + node->n_children - 1;

        while (i <= j) {
            nvRigidBody *body = node->bodies->data[node->children[i]];
            nv_float c = body->bvh_median_x;

            if (c < split) {
                i++;
            }
            else {
                size_t k = j--;
                size_t temp = node->children[i];
                node->children[i] = node->children[k];
                node->children[k] = temp; 
            }
        }

        mid = i;
    }
    else {
        nv_float split = 0.0;
        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvRigidBody *body = node->bodies->data[node->children[i]];
            split += body->bvh_median_y;
        }
        split /= (nv_float)node->n_children;

        size_t i = node->start_i;
        size_t j = node->start_i + node->n_children - 1;

        while (i <= j) {
            nvRigidBody *body = node->bodies->data[node->children[i]];
            nv_float c = body->bvh_median_y;

            if (c < split) {
                i++;
            }
            else {
                size_t k = j--;
                size_t temp = node->children[i];
                node->children[i] = node->children[k];
                node->children[k] = temp; 
            }
        }

        mid = i;
    }

    // Do not split if one of the sides is empty
    size_t left_n = mid - node->start_i;
    if (left_n == 0 || left_n == node->n_children) {
        node->is_leaf = true;
        return;
    }

    size_t right_n = node->n_children - left_n;

    nv_bool left_leaf = left_n <= NV_BVH_LEAF_THRESHOLD;
    nv_bool right_leaf = right_n <= NV_BVH_LEAF_THRESHOLD;

    node->left = nvBVHNode_new(left_leaf, node->bodies, node->children, node->start_i, left_n);
    node->right = nvBVHNode_new(right_leaf, node->bodies, node->children, mid, right_n);

    nvBVHNode_build_aabb(node->left);
    nvBVHNode_build_aabb(node->right);

    nvBVHNode_subdivide(node->left);
    nvBVHNode_subdivide(node->right);
}

void nvBVHNode_collide(nvBVHNode *node, nvAABB aabb, nvArray *collided) {
    if (!node) return;

    if (!nv_collide_aabb_x_aabb(node->aabb, aabb)) return;

    if (node->is_leaf) {
        for (size_t i = node->start_i; i < node->start_i + node->n_children; i++) {
            nvArray_add(collided, node->bodies->data[node->children[i]]);
        }
    }
    else {
        nvBVHNode_collide(node->left, aabb, collided);
        nvBVHNode_collide(node->right, aabb, collided);
    }
}

size_t nvBVHNode_size(nvBVHNode *node) {
    if (!node) return 0;
    if (node->is_leaf) return 1;
    else {
        size_t a = nvBVHNode_size(node->left);
        size_t b = nvBVHNode_size(node->right);
        return 1 + a + b;
    }
}

size_t nvBVHNode_total_memory_used(nvBVHNode *node) {
    if (!node) return 0;

    size_t node_s = sizeof(nvBVHNode);

    if (node->is_leaf) return node_s;

    size_t a = nvBVHNode_total_memory_used(node->left);
    size_t b = nvBVHNode_total_memory_used(node->right);

    return node_s + a + b;
}


nvBVHNode *nvBVHTree_new(nvArray *bodies, size_t *children, size_t children_n) {
    nvBVHNode *root = nvBVHNode_new(false, bodies, children, 0, children_n);

    nvBVHNode_build_aabb(root);
    nvBVHNode_subdivide(root);

    return root;
}

void nvBVHTree_free(nvBVHNode *root) {
    if (!root) return;
    
    nvBVHNode_free(root->left);
    nvBVHNode_free(root->right);
}