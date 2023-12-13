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


nvBVHNode *nvBVHNode_new(bool is_leaf, nvArray *bodies) {
    nvBVHNode *node = NV_NEW(nvBVHNode);
    if (!node) return NULL;

    node->is_leaf = is_leaf;
    node->left = NULL;
    node->right = NULL;
    node->bodies = bodies;

    return node;
}

void nvBVHNode_free(nvBVHNode *node) {
    if (node == NULL) return;

    nvArray_free(node->bodies);

    if (!node->is_leaf) {
        nvBVHNode_free(node->left);
        nvBVHNode_free(node->right);
    }

    free(node);
}

void nvBVHNode_build_aabb(nvBVHNode *node) {
    if (node == NULL) return;
    if (node->bodies == NULL) return;

    if (node->bodies->size > 0) {
        node->aabb = (nvAABB){NV_INF, NV_INF, -NV_INF, -NV_INF};

        for (size_t i = 0; i < node->bodies->size; i++) {
            nvBody *body = node->bodies->data[i];
            nvAABB aabb = nvBody_get_aabb(body);

            node->aabb.min_x = nv_fmin(node->aabb.min_x, aabb.min_x);
            node->aabb.min_y = nv_fmin(node->aabb.min_y, aabb.min_y);
            node->aabb.max_x = nv_fmax(node->aabb.max_x, aabb.max_x);
            node->aabb.max_y = nv_fmax(node->aabb.max_y, aabb.max_y);
        }
    }
}

void nvBVHNode_subdivide(nvBVHNode *node) {
    if (node == NULL) return;
    if (node->is_leaf) return;

    nv_float width = node->aabb.max_x - node->aabb.min_x;
    nv_float height = node->aabb.max_y - node->aabb.min_y;

    nvArray *lefts = nvArray_new();
    nvArray *rights = nvArray_new();

    // Split along the longest axis
    if (width > height) {
        nv_float split = 0.0;
        for (size_t i = 0; i < node->bodies->size; i++) {
            nvBody *body = node->bodies->data[i];
            split += body->position.x;
        }
        split /= (nv_float)node->bodies->size;

        for (size_t i = 0; i < node->bodies->size; i++) {
            nvBody *body = node->bodies->data[i];

            if (body->position.x <= split)
                nvArray_add(lefts, body);
            else
                nvArray_add(rights, body);
        }
    }
    else {
        nv_float split = 0.0;
        for (size_t i = 0; i < node->bodies->size; i++) {
            nvBody *body = node->bodies->data[i];
            split += body->position.y;
        }
        split /= (nv_float)node->bodies->size;

        for (size_t i = 0; i < node->bodies->size; i++) {
            nvBody *body = node->bodies->data[i];

            if (body->position.y <= split)
                nvArray_add(lefts, body);
            else
                nvArray_add(rights, body);
        }
    }

    // One of the two sides is empty
    if ((lefts->size == 0) || (rights->size == 0)) {
       nvArray_free(lefts);
       nvArray_free(rights);
       return;
    }

    bool left_leaf = lefts->size <= NV_BVH_LEAF_THRESHOLD;
    bool right_leaf = rights->size <= NV_BVH_LEAF_THRESHOLD;

    node->left = nvBVHNode_new(left_leaf, lefts);
    node->right = nvBVHNode_new(right_leaf, rights);
    if (!node->left || !node->right) return;

    nvBVHNode_build_aabb(node->left);
    nvBVHNode_build_aabb(node->right);

    nvBVHNode_subdivide(node->left);
    nvBVHNode_subdivide(node->right);
}

nvArray *nvBVHNode_collide(nvBVHNode *node, nvAABB aabb, bool *is_combined) {
    *is_combined = false;
    if (node == NULL) return NULL;

    if (node->is_leaf) {
        if (nv_collide_aabb_x_aabb(node->aabb, aabb)) {
            return node->bodies;
        }

        else {
            return NULL;
        }
    }

    if (nv_collide_aabb_x_aabb(node->aabb, aabb)) {
        bool is_left_combined;
        bool is_right_combined;
        nvArray *left = nvBVHNode_collide(node->left, aabb, &is_left_combined);
        nvArray *right = nvBVHNode_collide(node->right, aabb, &is_right_combined);

        if (left == NULL) {
            if (is_right_combined) *is_combined = true;
            return right;
        }

        else if (right == NULL) {
            if (is_left_combined) *is_combined = true;
            return left;
        }

        else {
            nvArray* combined = nvArray_new();

            for (size_t i = 0; i < left->size; i++)
                nvArray_add(combined, left->data[i]);

            for (size_t i = 0; i < right->size; i++)
                nvArray_add(combined, right->data[i]);

            if (is_left_combined) nvArray_free(left);
            if (is_right_combined) nvArray_free(right);

            *is_combined = true;
            return combined;
        }
    }

    return NULL;
}

size_t nvBVHNode_size(nvBVHNode *node) {
    if (node == NULL) return 0;
    if (node->is_leaf) return 0;
    else {
        size_t a = nvBVHNode_size(node->left);
        size_t b = nvBVHNode_size(node->right);
        return 1 + a + b;
    }
}


nvBVHNode *nvBVHTree_new(nvArray *bodies) {
    nvBVHNode *root = nvBVHNode_new(false, bodies);

    nvBVHNode_build_aabb(root);
    nvBVHNode_subdivide(root);

    return root;
}

void nvBVHTree_free(nvBVHNode *root) {
    nvBVHNode_free(root->left);
    nvBVHNode_free(root->right);

    free(root);
}