// GTest
#include "gtest/gtest.h"

// PThreads
#include <pthread.h>

// Intel intrinsics
#include <immintrin.h>

// xsync
#include "../include/scope.hpp"

TEST(ScopeTest, MakesForwardProgress) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    // This test will only finish if fallback is eventually taken
    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        if (_xtest()) _xabort(0);
    }

    EXPECT_TRUE(true);
    pthread_mutex_destroy(&fallback);
}

TEST(ScopeTest, ExecutesTransactionally) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);
    unsigned char wasTransactional = 0;
    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        wasTransactional = _xtest();
    }

    EXPECT_TRUE(wasTransactional);
    pthread_mutex_destroy(&fallback);
}

TEST(ScopeTest, ExecutesCommitCallback) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    bool callback_ran = false;

    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        scope.registerCommitCallback([&callback_ran]() { callback_ran = true; });
    }

    EXPECT_TRUE(callback_ran);
    pthread_mutex_destroy(&fallback);

}

TEST(ScopeTest, ExecutesTwoCommitCallbacks) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    bool first_callback_ran = false;
    bool second_callback_ran = false;


    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        scope.registerCommitCallback([&first_callback_ran]() { first_callback_ran = true; });
        scope.registerCommitCallback([&second_callback_ran]() { second_callback_ran = true; });
    }

    EXPECT_TRUE(first_callback_ran);
    EXPECT_TRUE(second_callback_ran);
    pthread_mutex_destroy(&fallback);
}

TEST(ScopeTest, ExecutesManyCommitCallbacks) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    int cb_counter = 0;
    int expected = 25;

    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        for (int i = 0; i < expected; ++i) {
            scope.registerCommitCallback([&cb_counter]() { ++cb_counter; });
        }
    }

    EXPECT_EQ(expected, cb_counter);
    pthread_mutex_destroy(&fallback);
}

TEST(ScopeTest, CallbacksExecutedInFIFOOrder) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    int nelements = 10;
    int buf[nelements];

    int expected[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};


    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        for (int i = 0; i < nelements; ++i) {
            scope.registerCommitCallback([&buf, i]() { buf[i] = i; });
        }
    }

    for (int i = 0; i < nelements; ++i) {
        EXPECT_EQ(expected[i], buf[i]);
    }

    pthread_mutex_destroy(&fallback);
}
