/**
 * Created by TekuConcept on August 1, 2018
 */

#include <iostream>
#include <sstream>
#include <gtest/gtest.h>
#include "async/promise.h"

using namespace impact;

#define V(x) std::cout << x << std::endl

TEST(test_promises, resolve_done) {
    // single done callback
    {
        deferred d;
        bool called = false;
        d.done(PROMISE_CALLBACK(,&) { called = true; });
        d.resolve();
        EXPECT_TRUE(called);
    }

    // multiple done callbacks
    {
        deferred d;
        bool called1 = false;
        bool called2 = false;
        d.done(
            PROMISE_CALLBACK(,&) { called1 = true; },
            PROMISE_CALLBACK(,&) { called2 = true; }
        );
        d.resolve();
        EXPECT_TRUE(called1);
        EXPECT_TRUE(called2);
    }

    // done not invoked on fail
    {
        deferred d;
        bool called = false;
        d.done(PROMISE_CALLBACK(,&) { called = true; });
        d.reject();
        EXPECT_FALSE(called);
    }

    // resolve with args
    {
        deferred d;
        int arg1;
        std::string arg2;
        d.done(PROMISE_CALLBACK(args,&) {
            ASSERT_EQ(args.size(), 2);
            arg1 = args[0].get<int>();
            arg2 = args[1].get<std::string>();
        });
        d.resolve(2, "hello");
        EXPECT_EQ(arg1, 2);
        EXPECT_EQ(arg2, "hello");
    }

    // only resolve once
    // (also cannot reject after resolving)
    {
        deferred d;
        int count = 0;
        d.done(PROMISE_CALLBACK(,&) { count++; });
        d.resolve();
        EXPECT_NE(d.state(), "pending");
        d.resolve();
        EXPECT_EQ(count, 1);
    }
}


TEST(test_promises, reject_fail) {
    // single fail callback
    {
        deferred d;
        bool called = false;
        d.fail(PROMISE_CALLBACK(,&) { called = true; });
        d.reject();
        EXPECT_TRUE(called);
    }

    // multiple fail callbacks
    {
        deferred d;
        bool called1 = false;
        bool called2 = false;
        d.fail(
            PROMISE_CALLBACK(,&) { called1 = true; },
            PROMISE_CALLBACK(,&) { called2 = true; }
        );
        d.reject();
        EXPECT_TRUE(called1);
        EXPECT_TRUE(called2);
    }

    // fail not invoked on done
    {
        deferred d;
        bool called = false;
        d.fail(PROMISE_CALLBACK(,&) { called = true; });
        d.resolve();
        EXPECT_FALSE(called);
    }

    // reject with args
    {
        deferred d;
        int arg1;
        std::string arg2;
        d.fail(PROMISE_CALLBACK(args, &) {
            ASSERT_EQ(args.size(), 2);
            arg1 = args[0].get<int>();
            arg2 = args[1].get<std::string>();
        });
        d.reject(2, "hello");
        EXPECT_EQ(arg1, 2);
        EXPECT_EQ(arg2, "hello");
    }

    // only reject once
    // (also cannot resolve after rejecting)
    {
        deferred d;
        int count = 0;
        d.fail(PROMISE_CALLBACK(,&) { count++; });
        d.reject();
        EXPECT_NE(d.state(), "pending");
        d.reject();
        EXPECT_EQ(count, 1);
    }
}


TEST(test_promises, state) {
    {
        deferred d;
        EXPECT_EQ(d.state(), "pending");
        d.resolve();
        EXPECT_EQ(d.state(), "resolved");
    }

    {
        deferred d;
        EXPECT_EQ(d.state(), "pending");
        d.reject();
        EXPECT_EQ(d.state(), "rejected");
    }
}


TEST(test_promises, always) {
    // single always callback resolve
    {
        deferred d;
        bool called = false;
        d.always(PROMISE_CALLBACK(,&) { called = true; });
        d.resolve();
        EXPECT_TRUE(called);
    }

    // single always callback reject
    {
        deferred d;
        bool called = false;
        d.always(PROMISE_CALLBACK(,&) { called = true; });
        d.reject();
        EXPECT_TRUE(called);
    }

    // multiple always callbacks
    {
        deferred d;
        bool called1 = false;
        bool called2 = false;
        d.always(
            PROMISE_CALLBACK(,&) { called1 = true; },
            PROMISE_CALLBACK(,&) { called2 = true; }
        );
        d.resolve();
        EXPECT_TRUE(called1);
        EXPECT_TRUE(called2);
    }

    // always callback with args
    {
        deferred d;
        int arg1;
        std::string arg2;
        d.always(PROMISE_CALLBACK(args, &) {
            ASSERT_EQ(args.size(), 2);
            arg1 = args[0].get<int>();
            arg2 = args[1].get<std::string>();
        });
        d.resolve(2, "hello");
        EXPECT_EQ(arg1, 2);
        EXPECT_EQ(arg2, "hello");
    }
}


TEST(test_promises, notify_progress) {
    // single progress callback
    {
        deferred d;
        bool called = false;
        d.progress(PROMISE_CALLBACK(,&) { called = true; });
        d.notify();
        EXPECT_TRUE(called);
    }

    // multiple progress callbacks
    {
        deferred d;
        bool called1 = false;
        bool called2 = false;
        d.progress(
            PROMISE_CALLBACK(,&) { called1 = true; },
            PROMISE_CALLBACK(,&) { called2 = true; }
        );
        d.notify();
        EXPECT_TRUE(called1);
        EXPECT_TRUE(called2);
    }

    // notify with args
    {
        deferred d;
        int arg1;
        std::string arg2;
        d.progress(PROMISE_CALLBACK(args, &) {
            ASSERT_EQ(args.size(), 2);
            arg1 = args[0].get<int>();
            arg2 = args[1].get<std::string>();
        });
        d.notify(2, "hello");
        EXPECT_EQ(arg1, 2);
        EXPECT_EQ(arg2, "hello");
    }

    // notify only if pending
    {
        deferred d;
        bool called = false;
        d.resolve();
        EXPECT_NE(d.state(), "pending");
        d.progress(PROMISE_CALLBACK(,&) { called = true; });
        d.notify();
        EXPECT_FALSE(called);
    }

    // chain notify
    {
        deferred a;
        std::ostringstream os;
        a
        .progress(PROMISE_CALLBACK(,&) { os << "A"; })
        .then(nullptr, nullptr, PROMISE_CALLBACK(,&) { os << "B"; });
        a.notify();
        EXPECT_EQ(os.str(), "AB");
    }
}


TEST(test_promises, chaining) {
    // single callback
    {
        deferred d;
        bool called = false;
        d.then(PROMISE_CALLBACK(,&) { called = true; });
        d.resolve();
        EXPECT_TRUE(called);
    }

    // then with args, done without
    {
        deferred d;
        std::string arg1, arg2;
        d.then(PROMISE_CALLBACK(args,&) {
            if (args.size() > 0)
                arg1 = args[0].get<std::string>();
        }).done(PROMISE_CALLBACK(args,&) {
            if (args.size() > 0)
                arg2 = args[0].get<std::string>();
        });
        d.resolve("test");
        EXPECT_EQ(arg1, "test");
        EXPECT_EQ(arg2, "");
    }

    // multiple callbacks
    {
        deferred d;
        bool called1 = false;
        bool called2 = false;
        d
        .then(PROMISE_CALLBACK(,&) { called1 = true; })
        .then(PROMISE_CALLBACK(,&) { called2 = true; });
        d.resolve();
        EXPECT_TRUE(called1);
        EXPECT_TRUE(called2);
    }

    // promise chains
    {
        deferred a;
        deferred b;
        bool called1 = false;
        bool called2 = false;
        a
        .then(PROMISE_CALLBACK_R(,&) {
            called1 = true;
            return b.promise();
        })
        .then(PROMISE_CALLBACK(,&) { called2 = true; });
        a.resolve();
        EXPECT_TRUE(called1);
        EXPECT_FALSE(called2);
        b.resolve();
        EXPECT_TRUE(called1);
        EXPECT_TRUE(called2);
    }

    // return value forwarding
    {
        deferred d;
        std::string value;
        d
        .then(PROMISE_CALLBACK_R() { return "test"; })
        .then(PROMISE_CALLBACK(args, &) {
            if (args.size() == 1)
                value = args[0].get<std::string>();
        })
        .then(PROMISE_CALLBACK(args) { EXPECT_EQ(args.size(), 0); });
        d.resolve();
        EXPECT_EQ(value, "test");
    }

    // chain ordering
    // chain links are concatenated to the last _referenced_ link
    {
        deferred a;
        std::ostringstream os;
        a
        .then(PROMISE_CALLBACK(,&) { os << "A"; })
        .then(PROMISE_CALLBACK(,&) { os << "B"; });
        a
        .then(PROMISE_CALLBACK(,&) { os << "X"; })
        .then(PROMISE_CALLBACK(,&) { os << "Z"; });
        a.resolve();
        EXPECT_EQ(os.str(), "ABXZ");
    }

    // complex chain ordering
    {
        deferred a, b;
        std::ostringstream os;
        a
        .then(PROMISE_CALLBACK(,&) { os << "A"; })
        .then(PROMISE_CALLBACK(,&) { os << "B"; });
        a
        .then(PROMISE_CALLBACK(,&) { os << "C"; })
        .then(PROMISE_CALLBACK_R(,&) { return b.promise(); })
        .then(PROMISE_CALLBACK(,&) { os << "D"; });
        a
        .then(PROMISE_CALLBACK(,&) { os << "E"; })
        .then(PROMISE_CALLBACK(,&) { os << "F"; });
        a.resolve();
        b.resolve();
        EXPECT_EQ(os.str(), "ABCEFD");
    }
}
