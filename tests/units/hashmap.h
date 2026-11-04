/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#include "tinytest/tinytest.h"
#include "novaphysics/novaphysics.h"


typedef struct {
    nv_uint64 id;
    char *name;
} TestEntry;

nv_uint64 test_hasher(void *item) {
    TestEntry *entry = item;

    return nv_hash_u64_to_u64(entry->id);
}

nv_bool test_comparer(void *a, void *b) {
    TestEntry *entry_a = a;
    TestEntry *entry_b = b;

    return entry_a->id == entry_b->id;
}

void test_nvHashMap_new(ttUnitTestSuite *test) {
    {
        nvHashMap *hashmap = nvHashMap_new(
            sizeof(TestEntry), test_hasher, test_comparer
        );
        tt_expect(nvHashMap_valid(hashmap), test);
        tt_expect_size_t(hashmap->size, 0, test);
        nvHashMap_free(hashmap);
    }

    {
        nvHashMap *hashmap = nvHashMap_new_ex(
            sizeof(TestEntry), test_hasher, test_comparer, 42, 3.0f, 0.75f
        );
        tt_expect(nvHashMap_valid(hashmap), test);
        tt_expect_size_t(hashmap->size, 0, test);
        tt_expect_size_t(hashmap->capacity, 42, test);
        tt_expect_float(hashmap->growth_factor, 3.0f, test);
        tt_expect_float(hashmap->growth_rule, 0.75f, test);
        nvHashMap_free(hashmap);
    }
}

void test_nvHashMap_set(ttUnitTestSuite *test) {
    nvHashMap *hashmap = nvHashMap_new(
        sizeof(TestEntry), test_hasher, test_comparer
    );

    void *item = nvHashMap_set(hashmap, &(TestEntry){.id = 42, .name = "Hey!"});
    tt_expect(item != NULL, test);

    TestEntry *entry = item;
    tt_expect_uint64_t(entry->id, 42, test);
    tt_expect_string(entry->name, "Hey!", test);

    nvHashMap_free(hashmap);
}

void test_nvHashMap_get(ttUnitTestSuite *test) {
    nvHashMap *hashmap = nvHashMap_new(
        sizeof(TestEntry), test_hasher, test_comparer
    );

    nvHashMap_set(hashmap, &(TestEntry){.id = 42, .name = "Hey!"});
    
    void *item0 = nvHashMap_get(hashmap, &(TestEntry){.id = 42});
    tt_expect(item0 != NULL, test);

    TestEntry *entry0 = item0;
    tt_expect_uint64_t(entry0->id, 42, test);
    tt_expect_string(entry0->name, "Hey!", test);

    void *item1 = nvHashMap_get(hashmap, &(TestEntry){0});
    tt_expect(item1 == NULL, test);

    void *item2 = nvHashMap_get(hashmap, &(TestEntry){.name = "Hey!"});
    tt_expect(item2 == NULL, test);

    nvHashMap_free(hashmap);
}

void test_nvHashMap_stress_get(ttUnitTestSuite *test) {
    nvHashMap *hashmap = nvHashMap_new(
        sizeof(TestEntry), test_hasher, test_comparer
    );

    for (nv_uint64 i = 0; i < 1000000; i++) {
        nvHashMap_set(hashmap, &(TestEntry){.id = i, .name = "Hey!"});
    }

    void *item0 = nvHashMap_get(hashmap, &(TestEntry){.id = 0});
    tt_expect(item0 != NULL, test);

    void *item1 = nvHashMap_get(hashmap, &(TestEntry){.id = 1000000 - 1});
    tt_expect(item1 != NULL, test);

    void *item2 = nvHashMap_get(hashmap, &(TestEntry){.id = 1000000});
    tt_expect(item2 == NULL, test);

    nvHashMap_free(hashmap);
}

void test_nvHashMap_clear(ttUnitTestSuite *test) {
    nvHashMap *hashmap = nvHashMap_new(
        sizeof(TestEntry), test_hasher, test_comparer
    );

    for (nv_uint64 i = 0; i < 10; i++) {
        nvHashMap_set(hashmap, &(TestEntry){.id = i, .name = "Hey!"});
    }
    
    void *item = nvHashMap_get(hashmap, &(TestEntry){.id = 5});
    tt_expect(item != NULL, test);

    nvHashMap_clear(hashmap);

    item = nvHashMap_get(hashmap, &(TestEntry){.id = 5});
    tt_expect(item == NULL, test);

    tt_expect_size_t(hashmap->size, 0, test);

    nvHashMap_free(hashmap);
}

void test_nvHashMap_remove(ttUnitTestSuite *test) {
    nvHashMap *hashmap = nvHashMap_new(
        sizeof(TestEntry), test_hasher, test_comparer
    );

    const size_t n_items = 10;
    for (nv_uint64 i = 0; i < n_items; i++) {
        nvHashMap_set(hashmap, &(TestEntry){.id = i, .name = "Hey!"});
    }

    void *non_item = nvHashMap_remove(hashmap, &(TestEntry){.id = 99999});
    tt_expect(non_item == NULL, test);
    
    void *item = nvHashMap_remove(hashmap, &(TestEntry){.id = 5});
    tt_expect(item != NULL, test);

    TestEntry *entry = item;
    tt_expect_uint64_t(entry->id, 5, test);
    tt_expect_string(entry->name, "Hey!", test);

    tt_expect_size_t(hashmap->size, n_items - 1, test);

    nvHashMap_free(hashmap);
}

void test_nvHashMap_contains(ttUnitTestSuite *test) {
    nvHashMap *hashmap = nvHashMap_new(
        sizeof(TestEntry), test_hasher, test_comparer
    );

    nvHashMap_set(hashmap, &(TestEntry){.id = 42, .name = "Hey!"});
    
    tt_expect(
        nvHashMap_contains(hashmap, &(TestEntry){.id = 42}) == true,
        test
    );

    tt_expect(
        nvHashMap_contains(hashmap, &(TestEntry){.id = 62}) == false,
        test
    );

    tt_expect(
        nvHashMap_contains(hashmap, &(TestEntry){.name = "Hey!"}) == false,
        test
    );

    nvHashMap_free(hashmap);
}


void run_nvHashMap_tests(ttUnitTestSuite *test) {
    TT_RUN_TEST_P(test_nvHashMap_new);
    TT_RUN_TEST_P(test_nvHashMap_set);
    TT_RUN_TEST_P(test_nvHashMap_get);
    TT_RUN_TEST_P(test_nvHashMap_stress_get);
    TT_RUN_TEST_P(test_nvHashMap_clear);
    TT_RUN_TEST_P(test_nvHashMap_remove);
    TT_RUN_TEST_P(test_nvHashMap_contains);
}