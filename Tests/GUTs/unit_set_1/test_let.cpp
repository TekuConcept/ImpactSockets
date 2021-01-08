/**
 * Created by TekuConcept on January 4, 2021
 */

#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "utils/let.h"

using namespace impact;


TEST(test_let, abstract_variable) {
    abstract_variable arg;
    EXPECT_FALSE(arg.truthy());
    EXPECT_EQ(arg.class_name(), "undefined");
}


TEST(test_let, impact_variable_constructor) {
    // default constructor
    {
        impact_variable<int> var1;
        impact_variable<std::string> var2;
        impact_variable<std::vector<int>> var3;

        EXPECT_EQ(var1.class_name(), "int");
        EXPECT_EQ(var2.class_name(), "std::string");
        // not all compilers fully expand default types, eg. GCC no-rtti yields
        // std::vector<int> instead of std::vector<int, std::allocator<int> >
        EXPECT_NE(var3.class_name().find("std::vector<int"), std::string::npos);
    }

    // construct with value
    {
        impact_variable<int> var1(5);
        impact_variable<std::string> var2("test");
        impact_variable<std::vector<int>> var3({ 1, 2, 3 });

        EXPECT_EQ(var1.value(), 5);
        EXPECT_EQ(var2.value(), "test");
        EXPECT_EQ(var3.value()[0], 1);
        EXPECT_EQ(var3.value()[1], 2);
        EXPECT_EQ(var3.value()[2], 3);
    }

    // move construct
    {
        int var1 = 5;
        std::string var2 = "test";

        impact_variable<int> var3(std::move(var1));
        EXPECT_EQ(var3.value(), 5);
        impact_variable<std::string> var4(std::move(var2));
        EXPECT_EQ(var4.value(), "test");
        impact_variable<std::string> var5(std::move(var4));
        EXPECT_EQ(var4.value(), "");
        EXPECT_EQ(var5.value(), "test");
    }

    // copy construct
    {
        impact_variable<int> var1(5);
        impact_variable<int> var2(var1);
        EXPECT_EQ(var1.value(), var2.value());
    }
}


TEST(test_let, impact_variable_assignment) {
    // copy assign
    {
        impact_variable<std::string> var1("test");
        impact_variable<std::string> var2 = var1;
        EXPECT_EQ(var1.value(), "test");
        EXPECT_EQ(var2.value(), "test");
    }

    // move assign
    {
        impact_variable<std::string> var1("test");
        impact_variable<std::string> var2 = std::move(var1);
        EXPECT_EQ(var1.value(), "");
        EXPECT_EQ(var2.value(), "test");
    }
}


TEST(test_let, impact_variable_create) {
    // default create
    {
        auto var = impact_variable<int>::create();
        EXPECT_EQ(var->class_name(), "int");
    }

    // copy create
    {
        auto var = impact_variable<int>::create(5);
        EXPECT_EQ(var->value(), 5);
    }

    // move create
    {
        std::string test = "test";
        auto var = impact_variable<std::string>::create(std::move(test));
        EXPECT_EQ(test, "");
        EXPECT_EQ(var->value(), "test");
    }
}


TEST(test_let, let_constructor) {
    // default constructor
    {
        let x;
        EXPECT_EQ(x.class_name(), "undefined");
    }

    // copy variable
    {
        let x(5);
        EXPECT_EQ(x.class_name(), "int");
        EXPECT_EQ(x.get<int>(), 5);
    }

    // move variable
    {
        std::string test = "test";
        let str(std::move(test));
        EXPECT_EQ(str.class_name(), "std::string");
        EXPECT_EQ(str.get<std::string>(), "test");
        EXPECT_EQ(test, "");
    }

    // copy string
    {
        let str("test");
        EXPECT_EQ(str.class_name(), "std::string");
        EXPECT_EQ(str.get<std::string>(), "test");
    }

    // copy constructor
    {
        let x(5);
        let y(x);
        EXPECT_EQ(x.get<int>(), 5);
        EXPECT_EQ(y.get<int>(), 5);
    }

    // move constructor
    {
        let str1("test");
        let str2(std::move(str1));
        EXPECT_EQ(str1.class_name(), "undefined");
        EXPECT_EQ(str2.get<std::string>(), "test");
    }

    // construct from abstract_variable ptr
    {
        auto ptr = std::shared_ptr<abstract_variable>(
            new impact_variable<int>(5)
        );
        let x(ptr);
        EXPECT_EQ(x.get<int>(), 5);
        EXPECT_EQ(ptr->class_name(), "int");
    }

    // construct from impact_variable ptr
    {
        auto ptr = impact_variable<int>::create(5);
        let x(ptr);
        EXPECT_EQ(x.get<int>(), 5);
        EXPECT_EQ(ptr->class_name(), "int");
    }
}


TEST(test_let, let_assignment) {
    // string assignment
    {
        let s = "test";
        EXPECT_EQ(s.class_name(), "std::string");
        EXPECT_EQ(s.get<std::string>(), "test");
    }

    // variable assignment
    {
        let x = 5;
        EXPECT_EQ(x.class_name(), "int");
        EXPECT_EQ(x.get<int>(), 5);
    }

    // copy assignment
    {
        let x(5);
        let y = x;
        EXPECT_EQ(y.get<int>(), 5);
    }

    // move assignment
    {
        let str1("test");
        let str2 = std::move(str1);
        EXPECT_EQ(str2.get<std::string>(), "test");
        EXPECT_EQ(str1.class_name(), "undefined");
    }

    // abstract_variable ptr assignment
    {
        let x = std::shared_ptr<abstract_variable>(
            new impact_variable<int>(5)
        );
        EXPECT_EQ(x.get<int>(), 5);
    }

    // impact_variable ptr assignment
    {
        let x = impact_variable<int>::create(5);
        EXPECT_EQ(x.get<int>(), 5);
    }
}


TEST(test_let, let_get) {
    // get valid value
    {
        let x(5);
        EXPECT_EQ(x.get<int>(), 5);
    }

    // get invalid value
    {
        try {
            let x;
            EXPECT_EQ(x.get<int>(), 0);
            FAIL();
        }
        catch (...) { SUCCEED(); }
    }
}


TEST(test_let, let_info) {
    // variable class name
    {
        // WARNING: not all compilers fully expand default template types
        // For example, std::vector<T,Allocator> will be fully expanded to
        // std::vector<int, std::allocator<int> > with T = int, but may
        // sometimes only be expanded to std::vector<int> with the default
        // Allocator type hidden.
        let x;
        let y(5);
        let s("test");
        EXPECT_EQ(x.class_name(), "undefined");
        EXPECT_EQ(y.class_name(), "int");
        EXPECT_EQ(s.class_name(), "std::string");
    }
}
