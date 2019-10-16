/**
 * Created by TekuConcept on October 14, 2019
 */

#include <gtest/gtest.h>

#include "utils/batch.h"

using namespace impact;


TEST(test_batch, batch)
{
    // constructors limit the initialization size
    // excess initialization items are dropped
    batch<int, 15> batch1;
    batch<int, 15> batch2(5, 8);
    EXPECT_EQ(batch2.size(), 5);
    batch<int, 15> batch3(9);
    EXPECT_EQ(batch3.size(), 9);
    batch<int, 15> batch4(18);
    EXPECT_EQ(batch4.size(), 15);
    batch<int, 15> batch5(batch2.begin(), batch2.end());
    EXPECT_EQ(batch5.size(), 5);
    batch<int, 15> batch6(batch5.raw());
    batch<int, 15> batch7({ 1, 2, 3, 4, 5 });
    batch<int, 15> batch8(batch7);
    batch<int, 15> batch9(std::move(batch7));
}


TEST(test_batch, push_back)
{
    // every function except for push_back and insert
    // aliases are left unmodified from the STL

    batch<int, 5> batch({ 1, 2, 3, 4, 5 });
    EXPECT_EQ(batch.size(), 5);

    batch.allow_dropping() = false;
    try {
        batch.push_back(6);
        FAIL();
    } catch (...) { }
    batch.allow_dropping() = true;
    try {
        batch.push_back(6);
        EXPECT_EQ(batch.size(), 5);
    } catch (...) { FAIL(); }
}


TEST(test_batch, insert)
{
    // every function except for push_back and insert
    // aliases are left unmodified from the STL

    std::vector<int> items = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };

    batch<int, 5> batch(3);
    EXPECT_EQ(batch.size(), 3);

    // insert(pos, first, last).size() <= limit
    batch.allow_dropping() = false;
    try {
        batch.insert(batch.begin(), items.begin(), items.end());
        FAIL();
    } catch (...) { }
    batch.allow_dropping() = true;
    try {
        batch.insert(batch.begin(), items.begin(), items.end());
        EXPECT_EQ(batch.size(), 5);
        EXPECT_EQ(batch[0], items[0]);
        EXPECT_EQ(batch[1], items[1]);
    } catch (...) { FAIL(); }

    // insert(pos, initializer_list).size() <= limit
    batch.resize(3);
    EXPECT_EQ(batch.size(), 3);
    batch.allow_dropping() = false;
    try {
        batch.insert(batch.begin(), { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
        FAIL();
    } catch (...) { }
    batch.allow_dropping() = true;
    try {
        batch.insert(batch.begin(), { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
        EXPECT_EQ(batch.size(), 5);
        EXPECT_EQ(batch[0], 0);
        EXPECT_EQ(batch[1], 1);
    } catch (...) { FAIL(); }
}


TEST(test_batch, assignment)
{
    {
        std::vector<int> vector = { 1, 2, 3, 4, 5 };
        batch<int, 15> batch1 = vector;
        EXPECT_EQ(batch1.size(), vector.size());
        for (size_t i = 0; i < batch1.size(); i++)
        { EXPECT_EQ(batch1[i], vector[i]); }

        batch<int, 15> batch2 = std::move(vector);
        EXPECT_EQ(vector.size(), 0);
        EXPECT_EQ(batch2.size(), batch1.size());
        for (size_t i = 0; i < batch2.size(); i++)
        { EXPECT_EQ(batch2[i], batch1[i]); }
    }

    {
        batch<int, 15> batch = { 1, 2, 3, 4, 5 };
        std::vector<int> vector1 = batch.raw();
        EXPECT_EQ(batch.size(), vector1.size());
        for (size_t i = 0; i < batch.size(); i++)
        { EXPECT_EQ(vector1[i], batch[i]); }
    }
}
