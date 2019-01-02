// GTest
#include "gtest/gtest.h"

// Pthreads
#include <pthread.h>

// STL
#include <thread>
#include <vector>
#include <chrono>

// xsync
#include "../include/condition_variable.hpp"

// Signal to no waiters, should have no effect
TEST(ConditionVariableTest, SignalNoWaiter) {
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    xsync::XCondVar<pthread_mutex_t> cv;

    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        cv.signal(scope);
    }

    SUCCEED();
}


TEST(ConditionVariableTest, SignalOneWaiter) {
    // Threads share the same fallback lock
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    // We'll let the thread append to this vector and check
    // the resulting order
    int buf[3];
    int *cur = buf;

    // All threads are synchronized on this cv
    xsync::XCondVar<pthread_mutex_t> cv;
    // All threads share the same fallback lock

    std::thread waiter([&cv, &cur, &fallback]() {
        {
            // execute transactionally
            xsync::XScope<pthread_mutex_t> scope(fallback);
            // This should be the first value
            *cur = 1; ++cur;
            // wait
            cv.wait(scope);
            // this should be the third value
            *cur = 3; ++cur;
        }
    });

    // Let the waiter get to the CV
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        // This should be the second value
        *cur = 2; ++cur;
        // Signal to the waiter
        cv.signal(scope);
    }

    // Let the waiter finish
    waiter.join();

    int expected[] = {1, 2, 3};

    // Check that vectors have same contents
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(expected[i], buf[i]);
    }
}

/*
TEST(ConditionVariableTest, SignalTwoWaiters) {
    // Threads share the same fallback lock
    pthread_mutex_t fallback;
    pthread_mutex_init(&fallback, nullptr);

    // We'll let the thread append to this vector and check
    // the resulting order
    std::vector<int> vec;

    // All threads are synchronized on this cv
    xsync::XCondVar<pthread_mutex_t> cv;
    // All threads share the same fallback lock

    std::thread waiter1([&cv, &vec, &fallback]() {
        {
            // execute transactionally
            xsync::XScope<pthread_mutex_t> scope(fallback);
            // This should be the first value
            vec.push_back(1);
            // wait
            cv.wait(scope);
            // this should be the third value
            vec.push_back(3);
        }
    });

    std::thread waiter2([&cv, &vec, &fallback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            // execute transan
            xsync::XScope<pthread_mutex_t> scope(fallback);
            // This should be the first value
            vec.push_back(1);
            // wait
            cv.wait(scope);
            // this should be the third value
            vec.push_back(3);
         }
     });

    // Let the waiter get to the CV
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
        xsync::XScope<pthread_mutex_t> scope(fallback);
        // This should be the second value
        vec.push_back(2);
        // Signal to the waiter
        cv.signal(scope);
    }

    // Let the waiter finish
    waiter.join();

    std::vector<int> expected = {1, 2, 3};

    // Check that vectors have same size
    ASSERT_EQ(expected.size(), vec.size());
    // Check that vectors have same contents
    for (int i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(expected[i], vec[i]);
    }
}
*/
