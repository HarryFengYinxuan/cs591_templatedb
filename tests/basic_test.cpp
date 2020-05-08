#include <iostream>
#include <vector>

#include "gtest/gtest.h"
// #include "templatedb/bplustree.hpp"
#include "templatedb/db.hpp"


class DBTest : public ::testing::Test
{
protected:
    templatedb::DB db0;
    templatedb::DB db1;
    templatedb::DB db2;

    templatedb::Value v1 = templatedb::Value({1, 2});
    templatedb::Value v2 = templatedb::Value({6, 10});
    templatedb::Value v3 = templatedb::Value({1, 1, 5, 7});
    templatedb::Value v4 = templatedb::Value({13, 176});


    void SetUp() override 
    {
        db1.put(2, v1);
        db1.put(5, v2);
        db2.put(1024, v3);
    }
};

TEST_F(DBTest, TreeInsert)
{
    // basic split
    BPlusTree *root = new_tree();
    std::vector<int> input_keys{1,3,5,7,9}; 
    for (
        std::vector<int>::iterator it = input_keys.begin(); 
        it != input_keys.end(); 
        ++it) {
        root = insert(root, *it, ValueIndex{1});
    }
    ASSERT_EQ(root -> keys[0], 5);
    ASSERT_EQ(root -> keys[1], -1);
    delete root;
}

TEST_F(DBTest, TreePersistence)
{
    // operations and then save and restore
    BPlusTree *root = new_tree();
    std::vector<int> input_keys{1,3,5,7,9}; 
    for (
        std::vector<int>::iterator it = input_keys.begin(); 
        it != input_keys.end(); 
        ++it) {
        root = insert(root, *it, ValueIndex{1});
    }
    root -> fn = "test_temp";
    save_node(root, "tree-test/");
    BPlusTree *temp = restore("test_temp", "tree-test/");
    ASSERT_EQ(temp -> keys[0], 5);
    ASSERT_EQ(temp -> keys[1], -1);
    delete root;
}

TEST_F(DBTest, TreeSearch)
{
    // operations and then save and restore
    BPlusTree *root = new_tree();
    std::srand(0); 
    // init input
    std::vector<int> input_keys; 
    std::vector<ValueIndex> input_vals;
    std::unordered_map<int, ValueIndex> result;
    int num_of_ops = 20;
    int kv_range = 100;
    int key;
    ValueIndex val;
    for (int i=0; i<num_of_ops; i++) {
        key = std::rand()%kv_range;
        val = ValueIndex{0, std::rand()%kv_range};
        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;
    }

    // inserting into tree
    for (int i=0; i<num_of_ops; i++) {
        root = insert(root, input_keys[i], input_vals[i]);
    }

    // searching in tree
    for(const auto& kv_pair : result) {
        ASSERT_EQ(
            search(root, kv_pair.first).null, 
            kv_pair.second.null
        );
        ASSERT_EQ(
            search(root, kv_pair.first).ptr, 
            kv_pair.second.ptr
        );
    }

    // search non-existent stuff, 20 is a random number
    for (int i=0; i<20; i++) {
        ASSERT_EQ(
            search(
                root, 
                std::rand()%kv_range+kv_range
            ).null, 
            true
        );
    }
    delete root;
}

TEST_F(DBTest, TreeSearchPersist)
{
    // operations and then save and restore
    BPlusTree *root = new_tree();
    std::srand(0); 
    // init input
    std::vector<int> input_keys; 
    std::vector<ValueIndex> input_vals;
    std::unordered_map<int, ValueIndex> result;
    int num_of_ops = 20;
    int kv_range = 100;
    int key;
    ValueIndex val;
    for (int i=0; i<num_of_ops; i++) {
        key = std::rand()%kv_range;
        val = ValueIndex{0, std::rand()%kv_range};
        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;
    }

    // inserting into tree
    for (int i=0; i<num_of_ops; i++) {
        root = insert(root, input_keys[i], input_vals[i]);
    }

    root -> fn = "test_temp2";
    save_node(root, "tree-test/");
    delete root;
    BPlusTree *temp = restore("test_temp2", "tree-test/");

    // searching in tree
    for( const auto& kv_pair : result ) {
        ASSERT_EQ(
            search(temp, kv_pair.first).null, 
            kv_pair.second.null
        );
        ASSERT_EQ(
            search(temp, kv_pair.first).ptr, 
            kv_pair.second.ptr
        );
    }

    // search non-existent stuff, 20 is a random number
    for (int i=0; i<20; i++) {
        ASSERT_EQ(
            search(
                temp, 
                std::rand()%kv_range+kv_range
            ).null, 
            true
        );
    }
    delete temp;
}


// // we dont care about size at this point
// TEST_F(DBTest, IsEmptyInitially)
// {
//     EXPECT_EQ(db0.size(), 0);
// }


TEST_F(DBTest, PutAndGetFunctionalityInMemory)
{
    std:srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // lets make one item per value
    while (1) {
        key = std::rand()%kv_range;
        val = templatedb::Value(
            std::vector<int>{std::rand()%kv_range}
        );

        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;

        if (result.size() < db0.maxsize) {
            db0.put(key, val);
        } else {
            // too big, lets stop
            result.erase(key);
            break;
        }
        
    }

    // searching in memory
    for( const auto& kv_pair : result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}


TEST_F(DBTest, DeleteNonExistentInMemory)
{
    std:srand(0);
    std::vector<int> input_keys;
    int kv_range = 100;
    int key, val;
    std::unordered_map<int, int> result;

    while (1) {
        key = std::rand()%kv_range;
        val = -1;

        input_keys.push_back(key);
        result[key] = val;

        if (result.size() < db0.maxsize) {
            // keep going
        } else {
            // too big, lets stop
            result.erase(key);
            break;
        }
    }

    // deleting in memory
    for( const auto& kv_pair : result) {
        db0.del(kv_pair.first);
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            false
        );
    }
    db0.clear_db();
}


TEST_F(DBTest, DeleteExistentInMemory)
{
    std:srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // lets make one item per value
    while (1) {
        key = std::rand()%kv_range;
        val = templatedb::Value(
            std::vector<int>{std::rand()%kv_range}
        );

        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;

        if (result.size() < db0.maxsize) {
            db0.put(key, val);
        } else {
            // too big, lets stop
            result.erase(key);
            break;
        }
        
    }

    // deleting in memory
    for( const auto& kv_pair : result) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        db0.del(kv_pair.first);
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            false
        );
    }
    db0.clear_db();
}

// ===== starting to use level 1 =====
TEST_F(DBTest, PutAndGetFunctionalityOnDiskLvling)
{
    std::srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // lets make one item per value
    while (1) {
        key = std::rand()%kv_range;
        val = templatedb::Value(
            std::vector<int>{std::rand()%kv_range}
        );

        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;

        if (result.size() < db0.maxsize) {
            db0.put(key, val);
        } else {
            // put one more to exceed the limit, then stop
            db0.put(key, val);
            // result.erase(key);
            break;
        }
        
    }

    // searching on disk
    for( const auto& kv_pair : result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}


TEST_F(DBTest, PutAndGetFunctionalityOnDiskTiering)
{
    db0.policy = 1; // tiering
    std:srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // lets make one item per value
    while (1) {
        key = std::rand()%kv_range;
        val = templatedb::Value(
            std::vector<int>{std::rand()%kv_range}
        );

        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;

        if (result.size() < db0.maxsize) {
            db0.put(key, val);
        } else {
            // put one more to exceed the limit, then stop
            db0.put(key, val);
            // result.erase(key);
            break;
        }
        
    }

    // searching on disk
    for( const auto& kv_pair : result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

// ===== =====

// ===== filling level 1 =====

TEST_F(DBTest, PutAndGetFunctionality2OnDiskLvling)
{
    std::srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> 
        all_result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // to mimic the behavior of components, when each result
    // table is full, it is appended to all_result, and a 
    // new table is created
    for (int i=0; i<db0.T; i++) {
        std::unordered_map<int, templatedb::Value> result;
        // fill this table
        while (1) {
            key = std::rand()%kv_range;
            val = templatedb::Value(
                std::vector<int>{std::rand()%kv_range}
            );

            input_keys.push_back(key);
            input_vals.push_back(val);
            result[key] = val;
            all_result[key] = val;

            if (result.size() < db0.maxsize) {
                db0.put(key, val);
            } else {
                // put one more to exceed the limit, then stop
                db0.put(key, val);
                // result.erase(key);
                break;
            }
        }
    }

    // searching on disk
    for( const auto& kv_pair : all_result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();
    

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

TEST_F(DBTest, PutAndGetFunctionality2OnDiskTiering)
{
    db0.policy = 1;
    std::srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> 
        all_result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // to mimic the behavior of components, when each result
    // table is full, it is appended to all_result, and a 
    // new table is created
    for (int i=0; i<db0.T; i++) {
        std::unordered_map<int, templatedb::Value> result;
        // fill this table
        while (1) {
            key = std::rand()%kv_range;
            val = templatedb::Value(
                std::vector<int>{std::rand()%kv_range}
            );

            input_keys.push_back(key);
            input_vals.push_back(val);
            result[key] = val;
            all_result[key] = val;

            if (result.size() < db0.maxsize) {
                db0.put(key, val);
            } else {
                // put one more to exceed the limit, then stop
                db0.put(key, val);
                // result.erase(key);
                break;
            }
        }
    }

    // searching on disk
    for( const auto& kv_pair : all_result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();
    

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

// ===== =====

// ===== filling level 2 =====

TEST_F(DBTest, PutAndGetFunctionality3OnDiskLvling)
{
    std::srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> 
        all_result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // to mimic the behavior of components, when each result
    // table is full, it is appended to all_result, and a 
    // new table is created
    for (int i=0; i<(db0.T+1); i++) {
        std::unordered_map<int, templatedb::Value> result;
        // fill this table
        while (1) {
            key = std::rand()%kv_range;
            val = templatedb::Value(
                std::vector<int>{std::rand()%kv_range}
            );

            input_keys.push_back(key);
            input_vals.push_back(val);
            result[key] = val;
            all_result[key] = val;

            if (result.size() < db0.maxsize) {
                db0.put(key, val);
            } else {
                // put one more to exceed the limit, then stop
                db0.put(key, val);
                // result.erase(key);
                break;
            }
        }
    }

    // searching on disk
    for( const auto& kv_pair : all_result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();
    

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

TEST_F(DBTest, PutAndGetFunctionality3OnDiskTiering)
{
    db0.policy = 1;
    std::srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> 
        all_result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // to mimic the behavior of components, when each result
    // table is full, it is appended to all_result, and a 
    // new table is created
    for (int i=0; i<(db0.T+1); i++) {
        std::unordered_map<int, templatedb::Value> result;
        // fill this table
        while (1) {
            key = std::rand()%kv_range;
            val = templatedb::Value(
                std::vector<int>{std::rand()%kv_range}
            );

            input_keys.push_back(key);
            input_vals.push_back(val);
            result[key] = val;
            all_result[key] = val;

            if (result.size() < db0.maxsize) {
                db0.put(key, val);
            } else {
                // put one more to exceed the limit, then stop
                db0.put(key, val);
                // result.erase(key);
                break;
            }
        }
    }

    // searching on disk
    for( const auto& kv_pair : all_result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();
    

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

// ===== =====

// ===== filling level mem, 1, 2 & 3 =====

TEST_F(DBTest, PutAndGetFunctionality4OnDiskLvling)
{
    std::srand(0);
    db0.T = 2;
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> 
        all_result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // to mimic the behavior of components, when each result
    // table is full, it is appended to all_result, and a 
    // new table is created
    for (int i=0; i<(11); i++) {
        std::unordered_map<int, templatedb::Value> result;
        // fill this table
        while (1) {
            key = std::rand()%kv_range;
            val = templatedb::Value(
                std::vector<int>{std::rand()%kv_range}
            );

            input_keys.push_back(key);
            input_vals.push_back(val);
            result[key] = val;
            all_result[key] = val;

            if (result.size() < db0.maxsize) {
                db0.put(key, val);
            } else {
                // put one more to exceed the limit, then stop
                db0.put(key, val);
                // result.erase(key);
                break;
            }
        }
    }
    // this is for mem
    std::unordered_map<int, templatedb::Value> result;
    while (1) {
        key = std::rand()%kv_range;
        val = templatedb::Value(
            std::vector<int>{std::rand()%kv_range}
        );

        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;
        all_result[key] = val;

        if (result.size() < db0.maxsize) {
            db0.put(key, val);
        } else {
            // put one more to exceed the limit, then stop
            all_result.erase(key);
            break;
        }
    }

    // searching
    for( const auto& kv_pair : all_result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();
    

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

TEST_F(DBTest, PutAndGetFunctionality4OnDiskTiering)
{
    db0.policy = 1;
    db0.T = 2;
    std::srand(0);
    std::vector<int> input_keys;
    std::vector<templatedb::Value> input_vals;
    std::unordered_map<int, templatedb::Value> 
        all_result;
    int kv_range = 100;
    int key;
    templatedb::Value val;
    // to mimic the behavior of components, when each result
    // table is full, it is appended to all_result, and a 
    // new table is created
    for (int i=0; i<(11); i++) {
        std::unordered_map<int, templatedb::Value> result;
        // fill this table
        while (1) {
            key = std::rand()%kv_range;
            val = templatedb::Value(
                std::vector<int>{std::rand()%kv_range}
            );

            input_keys.push_back(key);
            input_vals.push_back(val);
            result[key] = val;
            all_result[key] = val;

            if (result.size() < db0.maxsize) {
                db0.put(key, val);
            } else {
                // put one more to exceed the limit, then stop
                db0.put(key, val);
                // result.erase(key);
                break;
            }
        }
    }
    // this is for mem
    std::unordered_map<int, templatedb::Value> result;
    while (1) {
        key = std::rand()%kv_range;
        val = templatedb::Value(
            std::vector<int>{std::rand()%kv_range}
        );

        input_keys.push_back(key);
        input_vals.push_back(val);
        result[key] = val;
        all_result[key] = val;

        if (result.size() < db0.maxsize) {
            db0.put(key, val);
        } else {
            // put one more to exceed the limit, then stop
            all_result.erase(key);
            break;
        }
    }

    // searching
    for( const auto& kv_pair : all_result ) {
        ASSERT_EQ(
            db0.get(kv_pair.first).visible, 
            true
        );
        ASSERT_EQ(
            db0.get(kv_pair.first), 
            kv_pair.second
        );
    }
    db0.clear_db();
    

    // we dont search for non-existent stuff in the testcase
    // because it will try to reach the disk
}

// ===== =====

// TEST_F(DBTest, ScanFunctionalityInMemory)
// {
//     std::vector<templatedb::Value> vals;
//     vals = db2.scan();
//     ASSERT_EQ(vals.size(), 1);
//     EXPECT_EQ(vals[0], DBTest::v3);

//     vals = db1.scan(1, 3);
//     ASSERT_EQ(vals.size(), 1);
//     EXPECT_EQ(vals[0], DBTest::v1);

//     vals = db1.scan();
//     ASSERT_EQ(vals.size(), 2);
// }


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}