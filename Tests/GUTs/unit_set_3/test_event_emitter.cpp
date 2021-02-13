/**
 * Created by TekuConcept on January 18, 2021
 */

#include <iostream>
#include <gtest/gtest.h>
#include "utils/event_emitter.h"

using namespace impact;


TEST(event_emitter, max_listeners) {
    { // default max listeners
        auto value = event_emitter::default_max_listeners;
        event_emitter events;
        EXPECT_EQ(events.get_max_listeners(), value);
        event_emitter::default_max_listeners = 20;
        event_emitter events2;
        EXPECT_EQ(events2.get_max_listeners(), 20);
        // reset for future tests
        event_emitter::default_max_listeners = value;
    }

    { // max listeners
        event_emitter events;
        EXPECT_EQ(events.get_max_listeners(),
            event_emitter::default_max_listeners);
        events.set_max_listeners(20);
        EXPECT_EQ(events.get_max_listeners(), 20);
    }
}


TEST(event_emitter, add_listener) {
    event_emitter events;
    std::vector<std::string> names;

    { // empty names ignored
        events.add_listener("", EVENT_LISTENER() { });
        names = events.event_names();
        EXPECT_EQ(names.size(), 0);
    }

    { // undefined callbacks ignored
        events.add_listener("test", nullptr);
        names = events.event_names();
        EXPECT_EQ(names.size(), 0);
    }

    { // add_listener(event, listener)
        events.add_listener("test", EVENT_LISTENER() { });
        names = events.event_names();
        ASSERT_EQ(names.size(), 1);
        EXPECT_EQ(names.front(), "test");
    }

    { // on(event, listener)
        events.add_listener("on", EVENT_LISTENER() { });
        names = events.event_names();
        ASSERT_EQ(names.size(), 2);
        // NOTE: names are in alphabetical order
        EXPECT_EQ(names.front(), "on");
    }

    { // listeners ignored when max count exceeded
        events.set_max_listeners(2);
        for (int i = 0; i < 5; i++)
            events.add_listener("test", EVENT_LISTENER() { });
        EXPECT_EQ(events.listener_count("test"), 2);
    }
}


TEST(event_emitter, remove_listener) {
    { // remove all listeners
        event_emitter events;
        events.add_listener("test", EVENT_LISTENER() { });
        events.add_listener("test", EVENT_LISTENER() { });
        EXPECT_EQ(events.listener_count("test"), 2);
        events.remove_all_listeners("test");
        EXPECT_EQ(events.listener_count("test"), 0);
    }

    { // remove single listener
        event_emitter::callback_t listener = EVENT_LISTENER() { };
        event_emitter events;
        events.on("test", listener);
        EXPECT_EQ(events.listener_count("test"), 1);
        events.off("test", listener);
        EXPECT_EQ(events.listener_count("test"), 0);
    }
}


TEST(event_emitter, prepend_listener) {
    event_emitter::callback_t cb1 = EVENT_LISTENER() { };
    event_emitter::callback_t cb2 = EVENT_LISTENER() { };

    { // listeners appended
        event_emitter events;
        events.add_listener("test", cb1);
        events.add_listener("test", cb2);
        const std::vector<event_emitter::callback_info>& listeners =
            events.listeners("test");
        ASSERT_EQ(listeners.size(), 2);
        EXPECT_EQ(listeners[0].id, (size_t)&cb1);
        EXPECT_EQ(listeners[1].id, (size_t)&cb2);
    }

    { // listeners prepended
        event_emitter events;
        events.prepend_listener("test", cb1);
        events.prepend_listener("test", cb2);
        const std::vector<event_emitter::callback_info>& listeners =
            events.listeners("test");
        ASSERT_EQ(listeners.size(), 2);
        EXPECT_EQ(listeners[0].id, (size_t)&cb2);
        EXPECT_EQ(listeners[1].id, (size_t)&cb1);
    }
}


TEST(event_emitter, emit) {
    event_emitter events;

    std::string arg1;
    int arg2;

    events.on("test", EVENT_LISTENER(args, &) {
        arg1 = args[0].get<std::string>();
        arg2 = args[1].get<int>();
    });

    events.emit("test", "hello", 2 /*, ...*/);

    EXPECT_EQ(arg1, "hello");
    EXPECT_EQ(arg2, 2);
}


TEST(event_emitter, once) {
    event_emitter::callback_t cb1 = EVENT_LISTENER() { };
    event_emitter::callback_t cb2 = EVENT_LISTENER() { };
    event_emitter::callback_t cb3 = EVENT_LISTENER() { };

    event_emitter events;
    events.on("test", cb3);
    events.once("test", cb1);
    events.prepend_once_listener("test", cb2);

    const std::vector<event_emitter::callback_info>& listeners =
        events.listeners("test");
    
    ASSERT_EQ(listeners.size(), 3);
    EXPECT_EQ(listeners[0].id, (size_t)&cb2);
    EXPECT_EQ(listeners[1].id, (size_t)&cb3);
    EXPECT_EQ(listeners[2].id, (size_t)&cb1);
    EXPECT_TRUE(listeners[0].once);  // cb2
    EXPECT_FALSE(listeners[1].once); // cb3
    EXPECT_TRUE(listeners[2].once);  // cb1

    events.emit("test", "demo");

    ASSERT_EQ(listeners.size(), 1);
    EXPECT_EQ(listeners[0].id, (size_t)&cb3);
}
