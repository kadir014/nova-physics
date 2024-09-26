/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "novaphysics/collision.h"
#include "novaphysics/core/array.h"
#include "novaphysics/math.h"
#include "novaphysics/constants.h"
#include "novaphysics/aabb.h"


/**
 * @file collision.c
 * 
 * @brief Collision detection and contact point generation functions.
 */


nvPersistentContactPair nv_collide_circle_x_circle(
    nvShape *circle_a,
    nvTransform xform_a,
    nvShape *circle_b,
    nvTransform xform_b
) {
    nvPersistentContactPair pcp = {
        .contact_count = 0,
        .normal = nvVector2_zero
    };

    // Transform circle centers
    nvVector2 ca = nvVector2_add(nvVector2_rotate(circle_a->circle.center, xform_a.angle), xform_a.position);
    nvVector2 cb = nvVector2_add(nvVector2_rotate(circle_b->circle.center, xform_b.angle), xform_b.position);

    nvVector2 delta = nvVector2_sub(cb, ca);
    nv_float dist = nvVector2_len(delta);
    nv_float radii = circle_a->circle.radius + circle_b->circle.radius;

    // Distance is over radii combined, not colliding
    if (dist > radii) return pcp;

    if (dist == 0.0)
        pcp.normal = NV_DEGENERATE_NORMAL;
    else
        pcp.normal = nvVector2_div(delta, dist);

    // Midway contact
    nvVector2 a_support = nvVector2_add(ca, nvVector2_mul(pcp.normal, circle_a->circle.radius));
    nvVector2 b_support = nvVector2_add(cb, nvVector2_mul(pcp.normal, -circle_b->circle.radius));
    nvVector2 contact = nvVector2_mul(nvVector2_add(a_support, b_support), 0.5);

    pcp.contact_count = 1;
    pcp.contacts[0].separation = -(radii - dist);
    pcp.contacts[0].id = 0;
    pcp.contacts[0].anchor_a = nvVector2_sub(contact, xform_a.position);
    pcp.contacts[0].anchor_b = nvVector2_sub(contact, xform_b.position);
    pcp.contacts[0].is_persisted = false;
    pcp.contacts[0].remove_invoked = false;
    pcp.contacts[0].solver_info = nvContactSolverInfo_zero;

    return pcp;
}

nv_bool nv_collide_circle_x_point(
    nvShape *circle,
    nvTransform xform,
    nvVector2 point
) {
    nvVector2 c = nvVector2_add(xform.position, nvVector2_rotate(circle->circle.center, xform.angle));
    nvVector2 delta = nvVector2_sub(c, point);
    return nvVector2_len2(delta) <= circle->circle.radius * circle->circle.radius;
}

/**
 * @brief Project circle onto axis and return extreme points.
 */
static inline void nv_project_circle(
    nvVector2 center,
    nv_float radius,
    nvVector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nvVector2 a = nvVector2_mul(nvVector2_normalize(axis), radius);

    nvVector2 p1 = nvVector2_add(center, a);
    nvVector2 p2 = nvVector2_sub(center, a);

    nv_float min = nvVector2_dot(p1, axis);
    nv_float max = nvVector2_dot(p2, axis);

    if (min > max) {
        nv_float temp = max;
        max = min;
        min = temp;
    }

    *min_out = min;
    *max_out = max;
}

/**
 * @brief Project polygon onto axis and return extreme points.
 */
static inline void nv_project_polyon(
    nvVector2 *vertices,
    size_t num_vertices,
    nvVector2 axis,
    nv_float *min_out,
    nv_float *max_out
) {
    nv_float min = NV_INF;
    nv_float max = -NV_INF;

    for (size_t i = 0; i < num_vertices; i++) {
        nv_float projection = nvVector2_dot(vertices[i], axis);
        
        if (projection < min) min = projection;

        if (projection > max) max = projection;
    }

    *min_out = min;
    *max_out = max;
}

/**
 * @brief Find closest vertex of the polygon to the circle.
 */
static inline nvVector2 nv_polygon_closest_vertex_to_circle(
    nvVector2 center,
    nvVector2 *vertices,
    size_t num_vertices
) {
    size_t closest = 0;
    nv_float min_dist = NV_INF;
    
    for (size_t i = 0; i < num_vertices; i++) {
        nv_float dist = nvVector2_dist2(vertices[i], center);

        if (dist < min_dist) {
            min_dist = dist;
            closest = i;
        }
    }

    return vertices[closest];
}

/**
 * @brief Perpendicular distance between point and line segment.
 */
static inline void nv_point_segment_dist(
    nvVector2 center,
    nvVector2 a,
    nvVector2 b,
    nv_float *dist_out,
    nvVector2 *contact_out
) {
    nvVector2 ab = nvVector2_sub(b, a);
    nvVector2 ap = nvVector2_sub(center, a);

    nv_float projection = nvVector2_dot(ap, ab);
    nv_float ab_len = nvVector2_len2(ab);
    nv_float dist = projection / ab_len;
    nvVector2 contact;

    if (dist <= 0.0) contact = a;

    else if (dist >= 1.0) contact = b;

    else contact = nvVector2_add(a, nvVector2_mul(ab, dist));

    *dist_out = nvVector2_dist2(center, contact);
    *contact_out = contact;
}

nvPersistentContactPair nv_collide_polygon_x_circle(
    nvShape *polygon,
    nvTransform xform_poly,
    nvShape *circle,
    nvTransform xform_circle,
    nv_bool flip_anchors
) {
    nvPolygon poly = polygon->polygon;
    nvCircle circ = circle->circle;
    nvPolygon_transform(polygon, xform_poly);
    nvVector2 p = nv_polygon_centroid(poly.xvertices, poly.num_vertices);
    nvVector2 c = nvVector2_add(xform_circle.position, nvVector2_rotate(circ.center, xform_circle.angle));
    size_t n = poly.num_vertices;
    nvVector2 *vertices = poly.xvertices;
    nv_float separation = NV_INF;
    nvVector2 normal = nvVector2_zero;

    nvPersistentContactPair pcp = {
        .contact_count = 0,
        .normal = nvVector2_zero
    };

    nv_float min_a, min_b, max_a, max_b;

    // Check each axes of polygon edges x circle

    for (size_t i = 0; i < n; i++) {
        nvVector2 va = vertices[i];
        nvVector2 vb = vertices[(i + 1) % n];

        nvVector2 edge = nvVector2_sub(vb, va);
        nvVector2 axis = nvVector2_normalize(nvVector2_perp(edge));

        nv_project_polyon(vertices, n, axis, &min_a, &max_a);
        nv_project_circle(c, circ.radius, axis, &min_b, &max_b);

        // Doesn't collide
        if (min_a >= max_b || min_b >= max_a) {
            return pcp;
        }

        nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

        if (axis_depth < separation) {
            separation = axis_depth;
            normal = axis;
        }
    }

    nvVector2 cp = nv_polygon_closest_vertex_to_circle(c, vertices, n);
    nvVector2 axis = nvVector2_normalize(nvVector2_sub(cp, c));

    nv_project_polyon(vertices, n, axis, &min_a, &max_a);
    nv_project_circle(c, circ.radius, axis, &min_b, &max_b);

    // Doesn't collide
    if (min_a >= max_b || min_b >= max_a) {
        return pcp;
    }

    nv_float axis_depth = nv_fmin(max_b - min_a, max_a - min_b);

    if (axis_depth < separation) {
        separation = axis_depth;
        normal = axis;
    }
    separation = -separation;

    // Flip normal
    if (nvVector2_dot(nvVector2_sub(p, c), normal) > 0.0) {
        normal = nvVector2_neg(normal);
    }
    if (flip_anchors) {
        normal = nvVector2_neg(normal);
    }

    // Get the contact on the closest edge
    nv_float dist;
    nv_float min_dist = NV_INF;
    nvVector2 contact = nvVector2_zero;
    nvVector2 new_contact = nvVector2_zero;
    for (size_t i = 0; i < n; i++) {
        nvVector2 va = vertices[i];
        nvVector2 vb = vertices[(i + 1) % n];

        nv_point_segment_dist(c, va, vb, &dist, &new_contact);

        if (dist < min_dist) {
            min_dist = dist;
            contact = new_contact;
        }
    }

    // Midpoint contact
    nvVector2 circle_contact = nvVector2_add(contact, nvVector2_mul(normal, separation));
    nvVector2 half_contact = nvVector2_mul(nvVector2_add(contact, circle_contact), 0.5);

    nvVector2 poly_anchor = nvVector2_sub(half_contact, xform_poly.position);
    nvVector2 circle_anchor = nvVector2_sub(half_contact, xform_circle.position);

    pcp.normal = normal;
    pcp.contact_count = 1;
    pcp.contacts[0].id = 0;
    pcp.contacts[0].is_persisted = false;
    pcp.contacts[0].remove_invoked = false;
    pcp.contacts[0].solver_info = nvContactSolverInfo_zero;
    pcp.contacts[0].separation = separation;

    if (flip_anchors) {
        pcp.contacts[0].anchor_a = circle_anchor;
        pcp.contacts[0].anchor_b = poly_anchor;
    }
    else {
        pcp.contacts[0].anchor_a = poly_anchor;
        pcp.contacts[0].anchor_b = circle_anchor;
    }

    return pcp;
}


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
    v_lower = NV_VECTOR2(
        v_lower.x + lower_mid_scale * normal.x,
        v_lower.y + lower_mid_scale * normal.y
    );
    v_upper = NV_VECTOR2(
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

    nvPersistentContactPair pcp;
    pcp.contact_count = 0;
    pcp.normal = nvVector2_zero;

    int edge_a = 0;
    nv_float separation_a;
    find_max_separation(&edge_a, &separation_a, a, b);

    int edge_b = 0;
    nv_float separation_b;
    find_max_separation(&edge_b, &separation_b, b, a);

    // Shapes are only overlapping if both separations are negative
    if (separation_a > 0.0 || separation_b > 0.0) return pcp;

    nv_bool flip;

    if (separation_b > separation_a) {
        flip = true;
        nvVector2 search_dir = b.normals[edge_b];
        nv_float min_dot = NV_INF;
        edge_a = 0;

        // Find the incident edge on polygon A
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

        // Find the incident edge on polygon B
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
        nvVector2 p = NV_VECTOR2(ca * d.x + sa * d.y, -sa * d.x + ca * d.y);

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
            contact->remove_invoked = false;

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
    /*
        Algorithm from "Real-Time Collision Detection", Christer Ericson
        Chapter 5, Page 202
    */

    nvPolygon_transform(polygon, xform);
    nvVector2 *vertices = polygon->polygon.xvertices;
    size_t n = polygon->polygon.num_vertices;
    
    // Do binary search over polygon vertices to find the fan triangle
    // (v[0], v[low], v[high]) the point p lies within the near sides of
    int low = 0;
    int high = (int)n;
    do {
        int mid = (low + high) / 2;
        if (nv_triangle_winding((nvVector2[3]){vertices[0], vertices[mid], point}) == 1)
            low = mid;
        else
            high = mid;
    } while (low + 1 < high);

    // If point outside last (or first) edge, then it is not inside the n-gon
    if (low == 0 || high == n) return false;

    // p is inside the polygon if it is left of
    // the directed edge from v[low] to v[high]
    return nv_triangle_winding((nvVector2[3]){vertices[low], vertices[high], point}) == 1;
}


nv_bool nv_collide_aabb_x_aabb(nvAABB a, nvAABB b) {
    return (!(a.max_x <= b.min_x || b.max_x <= a.min_x ||
              a.max_y <= b.min_y || b.max_y <= a.min_y));
}

nv_bool nv_collide_aabb_x_point(nvAABB aabb, nvVector2 point) {
    return (aabb.min_x <= point.x && point.x <= aabb.max_x &&
            aabb.min_y <= point.y && point.y <= aabb.max_y);
}


nv_bool nv_collide_ray_x_circle(
    nvRayCastResult *result,
    nvVector2 origin,
    nvVector2 dir,
    nv_float maxsq,
    nvShape *shape,
    nvTransform xform
) {
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html

    nvCircle circle = shape->circle;
    nvVector2 center = nvVector2_add(nvVector2_rotate(circle.center, xform.angle), xform.position);
    nv_float rsq = circle.radius * circle.radius;
    nvVector2 delta = nvVector2_sub(center, origin);

    nv_float tca = nvVector2_dot(delta, dir);
    nv_float d2 = nvVector2_dot(delta, delta) - tca * tca;
    if (d2 > rsq) return false;
    nv_float thc = nv_sqrt(rsq - d2);
    nv_float t0 = tca - thc;
    nv_float t1 = tca + thc;
    
    if (t0 > t1) {
        nv_float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    if (t0 < 0.0) {
        t0 = t1;
        if (t0 < 0.0) return false; // Intersection behind ray origin
    }

    nv_float t = t0;

    nvVector2 hitpoint = nvVector2_add(origin, nvVector2_mul(dir, t));

    // Out of ray's range
    if (nvVector2_len2(nvVector2_sub(hitpoint, origin)) > maxsq) return false;

    *result = (nvRayCastResult){
        .position = hitpoint,
        .normal = nvVector2_normalize(nvVector2_sub(hitpoint, center)),
        .shape = shape
    };
    return true;
}

nv_bool nv_collide_ray_x_polygon(
    nvRayCastResult *result,
    nvVector2 origin,
    nvVector2 dir,
    nv_float maxsq,
    nvShape *shape,
    nvTransform xform
) {
    // https://rootllama.wordpress.com/2014/06/20/ray-line-segment-intersection-test-in-2d/
    // https://stackoverflow.com/a/29020182

    nvPolygon poly = shape->polygon;

    nvVector2 hits[NV_POLYGON_MAX_VERTICES];
    size_t normal_idxs[NV_POLYGON_MAX_VERTICES];
    size_t hit_count = 0;

    nvPolygon_transform(shape, xform);
    for (size_t i = 0; i < poly.num_vertices; i++) {
        nvVector2 va = poly.xvertices[i];
        nvVector2 vb = poly.xvertices[(i + 1) % poly.num_vertices];

        nvVector2 v1 = nvVector2_sub(origin, va);
        nvVector2 v2 = nvVector2_sub(vb, va);
        nvVector2 v3 = nvVector2_perp(dir);

        nv_float dot = nvVector2_dot(v2, v3);
        if (nv_fabs(dot) < NV_FLOAT_EPSILON) continue;;

        nv_float t1 = nvVector2_cross(v2, v1) / dot;
        nv_float t2 = nvVector2_dot(v1, v3) / dot;

        if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0)) {
            hits[hit_count++] = nvVector2_add(origin, nvVector2_mul(dir, t1));
            normal_idxs[hit_count - 1] = i;
        }
    }

    if (hit_count == 0) return false;

    nvVector2 closest_hit;
    nvVector2 normal = nvVector2_zero;
    nv_float min_dist = NV_INF;
    for (size_t i = 0; i < hit_count; i++) {
        nv_float dist = nvVector2_len2(nvVector2_sub(hits[i], origin));
        if (dist < min_dist) {
            min_dist = dist;
            closest_hit = hits[i];
            normal = poly.normals[normal_idxs[i]];
        }
    }

    // Out of ray's range
    if (min_dist > maxsq) return false;

    *result = (nvRayCastResult){
        .position = closest_hit,
        .normal = nvVector2_rotate(normal, xform.angle),
        .shape = shape
    };
    return true;
}