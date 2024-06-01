/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/collision.h"
#include "novaphysics/array.h"
#include "novaphysics/math.h"
#include "novaphysics/constants.h"
#include "novaphysics/aabb.h"


/**
 * @file collision.c
 * 
 * @brief Collision detection and contact point generation functions.
 */


// nvResolution nv_collide_circle_x_circle(nvRigidBody *a, nvRigidBody *b) {
//     nvResolution res = {
//         .collision = false,
//         .a = a,
//         .b = b,
//         .normal = nvVector2_zero,
//         .depth = 0.0
//     };

//     nv_float dist2 = nvVector2_dist2(b->position, a->position);
//     nv_float dist = nv_sqrt(dist2);
//     nv_float radii = a->shape->radius + b->shape->radius;
//     nv_float radii2 = radii * radii;

//     // Circles aren't colliding
//     if (dist2 >= radii2) return res;

//     res.collision = true;

//     nvVector2 normal = nvVector2_sub(b->position, a->position);
//     // If the bodies are in the exact same position, direct the normal upwards
//     if (nvVector2_len2(normal) == 0.0) normal = NV_VEC2(0.0, 1.0);
//     else normal = nvVector2_normalize(normal);

//     res.normal = normal;
//     res.depth = radii - dist;

//     return res;
// }

// nv_bool nv_collide_circle_x_point(nvRigidBody *circle, nvVector2 point) {
//     return nvVector2_len2(
//         nvVector2_sub(circle->position, point)) <= circle->shape->radius * circle->shape->radius;
// }


// nvResolution nv_collide_polygon_x_circle(nvRigidBody *polygon, nvRigidBody *circle) {
//     nvResolution res = {
//         .collision = false,
//         .a = polygon,
//         .b = circle,
//         .normal = nvVector2_zero,
//         .depth = NV_INF
//     };

//     nvRigidBody_local_to_world(polygon);
//     nvArray *vertices = polygon->shape->trans_vertices;

//     size_t n = vertices->size;

//     nv_float min_a, min_b, max_a, max_b;

//     for (size_t i = 0; i < n; i++) {
//         nvVector2 va = NV_TO_VEC2(vertices->data[i]);
//         nvVector2 vb = NV_TO_VEC2(vertices->data[(i + 1) % n]);

//         nvVector2 edge = nvVector2_sub(vb, va);
//         nvVector2 axis = nvVector2_normalize(nvVector2_perp(edge));

//         nv_project_polyon(vertices, axis, &min_a, &max_a);
//         nv_project_circle(circle->position, circle->shape->radius, axis, &min_b, &max_b);

//         // Doesn't collide
//         if (min_a >= max_b || min_b >= max_a) {
//             return res;
//         }

//         nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

//         if (axis_depth < res.depth) {
//             res.depth = axis_depth;
//             res.normal = axis;
//         }
//     }

//     nvVector2 cp = nv_polygon_closest_vertex_to_circle(circle->position, vertices);

//     nvVector2 axis = nvVector2_normalize(nvVector2_sub(cp, circle->position));

//     nv_project_polyon(vertices, axis, &min_a, &max_a);
//     nv_project_circle(circle->position, circle->shape->radius, axis, &min_b, &max_b);

//     // Doesn't collide
//     if (min_a >= max_b || min_b >= max_a) {
//         return res;
//     }

//     nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

//     if (axis_depth < res.depth) {
//         res.depth = axis_depth;
//         res.normal = axis;
//     }

//     nvVector2 direction = nvVector2_sub(polygon->position, circle->position);

//     if (nvVector2_dot(direction, res.normal) > 0.0)
//         res.normal = nvVector2_neg(res.normal);

//     res.collision = true;

//     return res;
// }


static nvPersistentContactPair clip_polygons(
    nvPolygon a,
    nvPolygon b,
    int edge_a,
    int edge_b,
    nv_bool flip
) {
    /*
        https://box2d.org/files/ErinCatto_ContactManifolds_GDC2007.pdf
        -
        Also see nv_collide_polygon_x_polygon for the reference.
    */

    // Reference polygon
    nvPolygon ref_polygon;
    int i11, i12;

    // Incident polygon
    nvPolygon inc_polygon;
    int i21, i22;

    if (flip) {
        ref_polygon = b;
        inc_polygon = a;
        i11 = edge_b;
        i12 = edge_b + 1 < b.num_vertices ? edge_b + 1 : 0;
		i21 = edge_a;
	    i22 = edge_a + 1 < a.num_vertices ? edge_a + 1 : 0;
    }
    else {
        ref_polygon = a;
		inc_polygon = b;
		i11 = edge_a;
		i12 = edge_a + 1 < a.num_vertices ? edge_a + 1 : 0;
		i21 = edge_b;
		i22 = edge_b + 1 < b.num_vertices ? edge_b + 1 : 0;
    }

    nvVector2 normal = ref_polygon.normals[i11];
    nvVector2 tangent = nvVector2_perp(normal);

    // Reference edge vertices
    nvVector2 v11 = ref_polygon.vertices[i11];
    nvVector2 v12 = ref_polygon.vertices[i12];

    // Incident edge vertices
    nvVector2 v21 = inc_polygon.vertices[i21];
    nvVector2 v22 = inc_polygon.vertices[i22];

    nv_float lower1 = 0.0;
    nv_float upper1 = nvVector2_dot(nvVector2_sub(v12, v11), tangent);
    nv_float upper2 = nvVector2_dot(nvVector2_sub(v21, v11), tangent);
    nv_float lower2 = nvVector2_dot(nvVector2_sub(v22, v11), tangent);
    nv_float d = upper2 - lower2;

    nvVector2 v_lower;
    if (lower2 < lower1 && upper2 - lower2 > NV_FLOAT_EPSILON)
        v_lower = nvVector2_lerp(v22, v21, (lower1 - lower2) / d);
    else
        v_lower = v22;

    nvVector2 v_upper;
    if (upper2 > upper1 && upper2 - lower2 > NV_FLOAT_EPSILON)
        v_upper = nvVector2_lerp(v22, v21, (upper1 - lower2) / d);
    else
        v_upper = v21;

    nv_float separation_lower = nvVector2_dot(nvVector2_sub(v_lower, v11), normal);
    nv_float separation_upper = nvVector2_dot(nvVector2_sub(v_upper, v11), normal);

    // Put contact points at midpoint
    nv_float lower_mid_scale = -separation_lower * 0.5;
    nv_float upper_mid_scale = -separation_upper * 0.5;
    v_lower = NV_VEC2(
        v_lower.x + lower_mid_scale * normal.x,
        v_lower.y + lower_mid_scale * normal.y
    );
    v_upper = NV_VEC2(
        v_upper.x + upper_mid_scale * normal.x,
        v_upper.y + upper_mid_scale * normal.y
    );

    nvPersistentContactPair pcp;

    if (!flip) {
        pcp.normal = normal;

        pcp.contacts[0].anchor_a = v_lower;
        pcp.contacts[0].separation = separation_lower;
        pcp.contacts[0].id = nv_u32pair(i11, i22);

        pcp.contacts[1].anchor_a = v_upper;
        pcp.contacts[1].separation = separation_upper;
        pcp.contacts[1].id = nv_u32pair(i12, i21);

        pcp.contact_count = 2;
    }
    else {
        pcp.normal = nvVector2_neg(normal);

        pcp.contacts[0].anchor_a = v_upper;
        pcp.contacts[0].separation = separation_upper;
        pcp.contacts[0].id = nv_u32pair(i21, i12);

        pcp.contacts[1].anchor_a = v_lower;
        pcp.contacts[1].separation = separation_lower;
        pcp.contacts[1].id = nv_u32pair(i22, i11);

        pcp.contact_count = 2;
    }

    return pcp;
}

static void find_max_separation(
    int *edge,
    nv_float *separation,
    nvPolygon a,
    nvPolygon b
) {
    /*
        Find the max separation between two polygons using edge normals of first polygon.
        See nv_collide_polygon_x_polygon for the reference.
    */

    int best_index = 0;
    nv_float max_separation = -NV_INF;

    for (int i = 0; i < a.num_vertices; i++) {
        nvVector2 n = a.normals[i];
        nvVector2 v1 = a.vertices[i];

        nv_float si = NV_INF;
        for (int j = 0; j < b.num_vertices; j++) {
            nv_float sij = nvVector2_dot(n, nvVector2_sub(b.vertices[j], v1));
            if (sij < si)
                si = sij;
        }

        if (si > max_separation) {
            max_separation = si;
            best_index = i;
        }
    }

    *edge = best_index;
    *separation = max_separation;
}

static nvPersistentContactPair SAT(nvPolygon a, nvPolygon b) {
    /*
        See nv_collide_polygon_x_polygon for the reference.
    */

    int edge_a = 0;
    nv_float separation_a;
    find_max_separation(&edge_a, &separation_a, a, b);

    int edge_b = 0;
    nv_float separation_b;
    find_max_separation(&edge_b, &separation_b, b, a);

    nv_bool flip;

    if (separation_b > separation_a) {
        flip = true;
        nvVector2 search_dir = b.normals[edge_b];
        nv_float min_dot = NV_INF;
        edge_a = 0;

        // Find the incident edge on poygon A
        for (int i = 0; i < a.num_vertices; i++) {
            nv_float dot = nvVector2_dot(search_dir, a.normals[i]);
            if (dot < min_dot) {
                min_dot = dot;
                edge_a = i;
            }
        }
    }
    else {
        flip = false;
        nvVector2 search_dir = a.normals[edge_a];
        nv_float min_dot = NV_INF;
        edge_b = 0;

        // Find the incident edge on poygon B
        for (int i = 0; i < b.num_vertices; i++) {
            nv_float dot = nvVector2_dot(search_dir, b.normals[i]);
            if (dot < min_dot) {
                min_dot = dot;
                edge_b = i;
            }
        }
    }

    return clip_polygons(a, b, edge_a, edge_b, flip);
}

nvPersistentContactPair nv_collide_polygon_x_polygon(
    nvShape *polygon_a,
    nvTransform xform_a,
    nvShape *polygon_b,
    nvTransform xform_b
) {
    /*
        Box2D V3's one-shot contact point generation algorithm for convex polygons.
        https://github.com/erincatto/box2c/blob/main/src/manifold.c

        Corner rounding and GJK is not included, Nova only uses SAT.
    */

    // TODO: Number of trig calls could definitely be lowered

    nvPolygon a = polygon_a->polygon;
    nvPolygon b = polygon_b->polygon;

    nvVector2 origin = a.vertices[0];

    // Shift polygon A to origin
    nvTransform xform_a_translated = {
        nvVector2_add(xform_a.position, nvVector2_rotate(origin, xform_a.angle)),
        xform_a.angle
    };
    // Inverse multiply transforms
    nvTransform xform;
    {
        nv_float sa = nv_sin(xform_a_translated.angle);
        nv_float ca = nv_cos(xform_a_translated.angle);
        nv_float sb = nv_sin(xform_b.angle);
        nv_float cb = nv_cos(xform_b.angle);

        // Inverse rotate
        nvVector2 d = nvVector2_sub(xform_b.position, xform_a_translated.position);
        nvVector2 p = NV_VEC2(ca * d.x + sa * d.y, -sa * d.x + ca * d.y);

        // Inverse multiply rotations
        nv_float is = ca * sb - sa * cb;
        nv_float ic = ca * cb + sa * sb;
        nv_float ia = nv_atan2(is, ic);

        xform = (nvTransform){p, ia};
    }

    nvPolygon a_local;
    a_local.num_vertices = a.num_vertices;
    a_local.vertices[0] = nvVector2_zero;
    a_local.normals[0] = a.normals[0];
    for (size_t i = 1; i < a_local.num_vertices; i++) {
        a_local.vertices[i] = nvVector2_sub(a.vertices[i], origin);
        a_local.normals[i] = a.normals[i];
    }

    nvPolygon b_local;
    b_local.num_vertices = b.num_vertices;
    for (size_t i = 0; i < b_local.num_vertices; i++) {
        nvVector2 xv = nvVector2_add(nvVector2_rotate(b.vertices[i], xform.angle), xform.position);

        b_local.vertices[i] = xv;
        b_local.normals[i] = nvVector2_rotate(b.normals[i], xform.angle);
    }

    nvPersistentContactPair pcp = SAT(a_local, b_local);

    if (pcp.contact_count > 0) {
        pcp.normal = nvVector2_rotate(pcp.normal, xform_a.angle);

        for (size_t i = 0; i < pcp.contact_count; i++) {
            nvContact *contact = &pcp.contacts[i];

            contact->anchor_a = nvVector2_rotate(
                nvVector2_add(contact->anchor_a, origin), xform_a.angle);
            contact->anchor_b = nvVector2_add(
                contact->anchor_a, nvVector2_sub(xform_a.position, xform_b.position));
            contact->is_persisted = false;

            contact->solver_info = nvContactSolverInfo_zero;
        }
    }

    return pcp;
}

nv_bool nv_collide_polygon_x_point(
    nvShape *polygon,
    nvTransform xform,
    nvVector2 point
) {
    // https://stackoverflow.com/a/48760556

    nvPolygon_transform(polygon, xform);
    nvVector2 *vertices = polygon->polygon.xvertices;

    size_t n = polygon->polygon.num_vertices;
    size_t i = 0;
    size_t j = n - 1;
    nv_bool inside = false;

    while (i < n) {
        nvVector2 vi = vertices[i];
        nvVector2 vj = vertices[j];

        nvVector2 diri = nvVector2_normalize(nvVector2_sub(vi, xform.position));
        nvVector2 dirj = nvVector2_normalize(nvVector2_sub(vj, xform.position));

        vi = nvVector2_add(vi, nvVector2_mul(diri, 0.1));
        vj = nvVector2_add(vj, nvVector2_mul(dirj, 0.1));

        if ((vi.y > point.y) != (vj.y > point.y) && (point.x < (vj.x - vi.x) * 
            (point.y - vi.y) / (vj.y - vi.y) + vi.x )) {
                inside = !inside;
            }

        j = i;
        i += 1;
    }

    return inside;
}


nv_bool nv_collide_aabb_x_aabb(nvAABB a, nvAABB b) {
    return (!(a.max_x <= b.min_x || b.max_x <= a.min_x ||
              a.max_y <= b.min_y || b.max_y <= a.min_y));
}

nv_bool nv_collide_aabb_x_point(nvAABB aabb, nvVector2 point) {
    return (aabb.min_x <= point.x && point.x <= aabb.max_x &&
            aabb.min_y <= point.y && point.y <= aabb.max_y);
}