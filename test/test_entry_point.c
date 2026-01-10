#include "unity.h"

// Forward declarations for test runner functions
void run_math_tests(void);
void run_core_tests(void);

// Global setUp/tearDown for Unity (called between tests)
void setUp(void) {
    // Individual test modules handle their own setup
}

void tearDown(void) {
    // Individual test modules handle their own teardown
}

int main(void) {
    UNITY_BEGIN();

    run_math_tests();
    run_core_tests();

    return UNITY_END();
}
