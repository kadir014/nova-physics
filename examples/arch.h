/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "example.h"


void ArchExample_setup(Example *example) {
    nvSpace *space = example->space;

    // Create ground
    nvBody *ground = nv_Rect_new(
        nvBodyType_STATIC,
        NV_VEC2(64.0, 52.5),
        0.0,
        nvMaterial_CONCRETE,
        120.0, 5.0
    );

    nvSpace_add(space, ground);

    // Create arch bricks

    nvArray *vertices;
    nvVector2 center;
    nvBody *brick;

    nvBodyType type = nvBodyType_DYNAMIC;

    nvMaterial brick_material = {
        .density = nvMaterial_CONCRETE.density,
        .restitution = 0.0,
        .friction = 0.87
    };

    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.0, 10.0));
    nvArray_add(vertices, NV_VEC2_NEW(-3.0, -10.0));
    nvArray_add(vertices, NV_VEC2_NEW(3.0, -10.0));
    nvArray_add(vertices, NV_VEC2_NEW(3.0, 10.0));

    center = NV_VEC2(37.0, 40.0);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);

    vertices = nvArray_new();
    nvSpace_add(space, brick);
    nvArray_add(vertices, NV_VEC2_NEW(-3.37051, 2.01043));
    nvArray_add(vertices, NV_VEC2_NEW(-2.57128, -2.59853));
    nvArray_add(vertices, NV_VEC2_NEW(3.3123, -1.42232));
    nvArray_add(vertices, NV_VEC2_NEW(2.62949, 2.01043));

    center = NV_VEC2(37.37051247175112, 27.989574474497488);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.69994, 1.31674));
    nvArray_add(vertices, NV_VEC2_NEW(-2.00674, -3.06906));
    nvArray_add(vertices, NV_VEC2_NEW(3.52304, -0.74063));
    nvArray_add(vertices, NV_VEC2_NEW(2.18364, 2.49295));

    center = NV_VEC2(38.499172871158635, 24.07430324643775);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.88472, 0.56029));
    nvArray_add(vertices, NV_VEC2_NEW(-1.34989, -3.42758));
    nvArray_add(vertices, NV_VEC2_NEW(3.58955, -0.02142));
    nvArray_add(vertices, NV_VEC2_NEW(1.64506, 2.88872));

    center = NV_VEC2(40.37715210034996, 20.44495368947912);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.91062, -0.22805));
    nvArray_add(vertices, NV_VEC2_NEW(-0.62189, -3.65331));
    nvArray_add(vertices, NV_VEC2_NEW(3.50369, 0.70324));
    nvArray_add(vertices, NV_VEC2_NEW(1.02882, 3.17811));

    center = NV_VEC2(42.93788365978014, 17.245415320448792);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.77047, -1.01318));
    nvArray_add(vertices, NV_VEC2_NEW(0.1501, -3.72907));
    nvArray_add(vertices, NV_VEC2_NEW(3.26526, 1.39887));
    nvArray_add(vertices, NV_VEC2_NEW(0.35512, 3.34337));

    center = NV_VEC2(46.086462558558765, 14.605285192196277);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.46577, -1.75765));
    nvArray_add(vertices, NV_VEC2_NEW(0.93341, -3.64354));
    nvArray_add(vertices, NV_VEC2_NEW(2.88297, 2.0309));
    nvArray_add(vertices, NV_VEC2_NEW(-0.35061, 3.37029));

    center = NV_VEC2(49.70232959755557, 12.63387023459242);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-3.00774, -2.42417));
    nvArray_add(vertices, NV_VEC2_NEW(1.69133, -3.39353));
    nvArray_add(vertices, NV_VEC2_NEW(2.37458, 2.56744));
    nvArray_add(vertices, NV_VEC2_NEW(-1.05817, 3.25026));

    center = NV_VEC2(53.6434729985393, 11.414508451893218);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-2.41742, -2.97876));
    nvArray_add(vertices, NV_VEC2_NEW(2.38579, -2.98567));
    nvArray_add(vertices, NV_VEC2_NEW(1.76582, 2.98221));
    nvArray_add(vertices, NV_VEC2_NEW(-1.73418, 2.98221));

    center = NV_VEC2(57.75223084482941, 10.99973799079986);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-1.72449, -3.39388));
    nvArray_add(vertices, NV_VEC2_NEW(2.98065, -2.43694));
    nvArray_add(vertices, NV_VEC2_NEW(1.08829, 3.25682));
    nvArray_add(vertices, NV_VEC2_NEW(-2.34446, 2.574));

    center = NV_VEC2(61.86250626106478, 11.40794776790177);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-0.96471, -3.65106));
    nvArray_add(vertices, NV_VEC2_NEW(3.44527, -1.77374));
    nvArray_add(vertices, NV_VEC2_NEW(0.37651, 3.3821));
    nvArray_add(vertices, NV_VEC2_NEW(-2.85707, 2.0427));

    center = NV_VEC2(65.80786856008876, 12.622063106398585);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(-0.17661, -3.74221));
    nvArray_add(vertices, NV_VEC2_NEW(3.75723, -1.02955));
    nvArray_add(vertices, NV_VEC2_NEW(-0.33524, 3.35813));
    nvArray_add(vertices, NV_VEC2_NEW(-3.24538, 1.41363));

    center = NV_VEC2(69.42975471434727, 14.590528358586543);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(0.60204, -3.66947));
    nvArray_add(vertices, NV_VEC2_NEW(3.90393, -0.24184));
    nvArray_add(vertices, NV_VEC2_NEW(-1.01555, 3.19309));
    nvArray_add(vertices, NV_VEC2_NEW(-3.49042, 0.71821));

    center = NV_VEC2(72.5849420578588, 17.23044170526779);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(1.33721, -3.44374));
    nvArray_add(vertices, NV_VEC2_NEW(3.88283, 0.55123));
    nvArray_add(vertices, NV_VEC2_NEW(-1.63777, 2.90133));
    nvArray_add(vertices, NV_VEC2_NEW(-3.58227, -0.00882));

    center = NV_VEC2(75.15166146173956, 20.43234698383862);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(2.00044, -3.08245));
    nvArray_add(vertices, NV_VEC2_NEW(3.70051, 1.31358));
    nvArray_add(vertices, NV_VEC2_NEW(-2.18078, 2.50122));
    nvArray_add(vertices, NV_VEC2_NEW(-3.52017, -0.73236));

    center = NV_VEC2(77.03405638154811, 24.0660294928454);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(2.56956, -2.6071));
    nvArray_add(vertices, NV_VEC2_NEW(3.37109, 2.01328));
    nvArray_add(vertices, NV_VEC2_NEW(-2.62891, 2.01328));
    nvArray_add(vertices, NV_VEC2_NEW(-3.31173, -1.41946));

    center = NV_VEC2(78.1650095847842, 27.98671617839755);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);

    vertices = nvArray_new();
    nvArray_add(vertices, NV_VEC2_NEW(3.0, -10.0));
    nvArray_add(vertices, NV_VEC2_NEW(3.0, 10.0));
    nvArray_add(vertices, NV_VEC2_NEW(-3.0, 10.0));
    nvArray_add(vertices, NV_VEC2_NEW(-3.0, -10.0));

    center = NV_VEC2(78.536096356631, 40.0);
    brick = nv_Polygon_new(type, center, 0.0, brick_material, vertices);
    nvSpace_add(space, brick);
}