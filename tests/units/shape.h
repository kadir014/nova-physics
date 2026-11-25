/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "tinytest/tinytest.h"
#include "novaphysics/novaphysics.h"
#include "random.h"


void test_nvCircleShape_new(ttUnitTestSuite *test) {
    {
        nvShape *circle = nvCircleShape_new(NV_VECTOR2(3.14f, -10.1f), 5.0f);
        tt_expect(circle != NULL, test);
        tt_expect_int(circle->type, (int)nvShapeType_CIRCLE, test);
        tt_expect_float(circle->circle.center.x, 3.14f, test);
        tt_expect_float(circle->circle.center.y, -10.1f, test);
        tt_expect_float(circle->circle.radius, 5.0f, test);
        nvShape_free(circle);
    }
    {
        nvShape *circle = nvCircleShape_new(nvVector2_zero, NV_NAN);
        tt_expect(circle == NULL, test);
        nvShape_free(circle);
    }
    {
        nvShape *circle = nvCircleShape_new(NV_VECTOR2(-NV_INF, NV_INF), 100.0f);
        tt_expect(circle == NULL, test);
        nvShape_free(circle);
    }
}

void test_nvPolygonShape_new(ttUnitTestSuite *test) {
    {
        nvVector2 points[10];
        for (size_t i = 0; i < 10; i++) {
            points[i] = NV_VECTOR2(
                nv_frand(-15.0f, 15.0f),
                nv_frand(-15.0f, 15.0f)
            );
        }

        nvShape *poly = nvConvexHullShape_new(points, 10, nvVector2_zero, true);

        tt_expect(poly != NULL, test);
        tt_expect_int(poly->type, (int)nvShapeType_POLYGON, test);
        // TODO: Test vertices

        nvShape_free(poly);
    }
    {
        nvVector2 points[10];
        for (size_t i = 0; i < 10; i++) {
            points[i] = NV_VECTOR2(NV_INF, NV_NAN);
        }

        nvShape *poly = nvConvexHullShape_new(points, 10, nvVector2_zero, true);

        tt_expect(poly == NULL, test);

        nvShape_free(poly);
    }
}


void run_nvShape_tests(ttUnitTestSuite *test) {
    TT_RUN_TEST_P(test_nvCircleShape_new);
    TT_RUN_TEST_P(test_nvPolygonShape_new);
}