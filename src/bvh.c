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


nvBVHNode *nvBVHNode_new(nv_bool is_leaf, nvArray *bodies) {
    nvBVHNode *node = NV_NEW(nvBVHNode);
    if (!node) return NULL;

    node->is_leaf = is_leaf;
    node->left = NULL;
    node->right = NULL;
    node->bodies = bodies;

    return node;
}

void nvBVHNode_free(nvBVHNode *node) {
    if (!node) return;

    nvArray_free(node->bodies);

    if (!node->is_leaf) {
        nvBVHNode_free(node->left);
        nvBVHNode_free(node->right);
    }

    NV_FREE(node);
}

void nvBVHNode_build_aabb(nvBVHNode *node) {
    if (!node) return;
    if (!node->bodies) return;

    if (node->bodies->size > 0) {
        node->aabb = (nvAABB){NV_INF, NV_INF, -NV_INF, -NV_INF};

        for (size_t i = 0; i < node->bodies->size; i++) {
            nvRigidBody *body = node->bodies->data[i];
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

    nvArray *lefts = nvArray_new();
    nvArray *rights = nvArray_new();

    // Current splitting method is midway trough the longest axis

    if (width > height) {
        nv_float split = 0.0;
        for (size_t i = 0; i < node->bodies->size; i++) {
            nvRigidBody *body = node->bodies->data[i];
            split += body->bvh_median_x;
        }
        split /= (nv_float)node->bodies->size;

        for (size_t i = 0; i < node->bodies->size; i++) {
            nvRigidBody *body = node->bodies->data[i];
            nv_float c = body->bvh_median_x;

            if (c <= split)
                nvArray_add(lefts, body);
            else
                nvArray_add(rights, body);
        }
    }
    else {
        nv_float split = 0.0;
        for (size_t i = 0; i < node->bodies->size; i++) {
            nvRigidBody *body = node->bodies->data[i];;
            split += body->bvh_median_y;
        }
        split /= (nv_float)node->bodies->size;

        for (size_t i = 0; i < node->bodies->size; i++) {
            nvRigidBody *body = node->bodies->data[i];
            nv_float c = body->bvh_median_y;

            if (c <= split)
                nvArray_add(lefts, body);
            else
                nvArray_add(rights, body);
        }
    }

    // Do not split if one of the sides is empty
    if ((lefts->size == 0) || (rights->size == 0)) {
        node->is_leaf = true;
        nvArray_free(lefts);
        nvArray_free(rights);
        return;
    }

    nv_bool left_leaf = lefts->size <= NV_BVH_LEAF_THRESHOLD;
    nv_bool right_leaf = rights->size <= NV_BVH_LEAF_THRESHOLD;

    node->left = nvBVHNode_new(left_leaf, lefts);
    node->right = nvBVHNode_new(right_leaf, rights);
    if (!node->left || !node->right) return;

    nvBVHNode_build_aabb(node->left);
    nvBVHNode_build_aabb(node->right);

    nvBVHNode_subdivide(node->left);
    nvBVHNode_subdivide(node->right);
}

void nvBVHNode_collide(nvBVHNode *node, nvAABB aabb, nvArray *collided) {
    if (!node) return;

    if (!nv_collide_aabb_x_aabb(node->aabb, aabb)) return;

    if (node->is_leaf) {
        for (size_t i = 0; i < node->bodies->size; i++) {
            nvArray_add(collided, node->bodies->data[i]);
        }
    }
    else {
        nvBVHNode_collide(node->left, aabb, collided);
        nvBVHNode_collide(node->right, aabb, collided);
    }
}

size_t nvBVHNode_size(nvBVHNode *node) {
    if (!node) return 0;
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

    NV_FREE(root);
}