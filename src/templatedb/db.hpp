#ifndef _TEMPLATEDB_DB_H_
#define _TEMPLATEDB_DB_H_

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <utility> 
#include <unistd.h>
#include <math.h> 
#include <boost/lambda/lambda.hpp>
#include <boost/filesystem.hpp>
#include <cstdio>
// #include <direct.h>
#include <stdlib.h>
// #include <filesystem> 
// #include <bits/stdc++.h> 
#include "templatedb/operation.hpp"
#include "templatedb/bplustree.hpp"

#define MAX_INT 2147483647

namespace templatedb
{

typedef enum _status_code
{
    OPEN = 0,
    CLOSED = 1,
    ERROR_OPEN = 100,
} db_status;

// value for the key value pair
class Value
{
public:
    std::vector<int> items;
    // does this represent anti-matter?
    bool visible = true;
    bool null = false;

    Value() {}
    // Value(bool _visible) {visible = _visible;}
    Value(bool _null) {null = _null;}
    Value(std::vector<int> _items) { items = _items;}

    bool operator ==(Value const & other) const
    {
        return (visible == other.visible) && 
            (items == other.items) &&
            (null == other.null);
    }
};


class DB
{
public:
    db_status status;

    DB() {init(0);};
    ~DB() {close();};

    // basic operations
    Value get(int key);
    void put(int key, Value val);
    std::vector<Value> scan(); // in memory?
    std::vector<Value> scan(int min_key, int max_key);
    void del(int key);

    size_t size();

    // file related
    db_status open(std::string & fname); 
    // init value_dimensions, and put

    bool close();
    bool load_data_file(std::string & fname); 
    void clear_db();
    // one off open?
    // when do you load? only memory?

    // general operation function
    std::vector<Value> execute_op(Operation op);

    // added
    int policy; // 0 = leveling; 1 = tiering
    int T = 2; // the parameter
    int maxsize = 10; // # of value in memory component
    int maxlvl = 3;
    // int max_level; // need this if realistic

private:
    std::fstream file; // persistence for memory
    // memory component
    std::unordered_map<int, Value> table;
    size_t value_dimensions = 0; // init inside open
    
    bool write_to_file();

    void init(int p); // choose policy

    // added
    // adding an array in nth level
    void add2level(
        std::vector<std::pair<int, Value>> arr, int level);

    // void write2component(
    //     std::vector<std::pair<int, Value>> arr, std::string fn);
    void merge_level(int n);
    void clear_level(int level);
    std::vector<std::string> get_level_fn(int level); 
    Value get_from_level(int key, int level);
    // std::string get_fn(int level, int component); // get filename
    // std::pair<int, int> parse_fn(string fn); // parse filename
    // list files. it return {fn}. {fn}.bpt will be the tree
    // {fn}.data will be the actual data
    void move2level(std::string comp, int from_level, 
        int to_level);
    // void rolling_add(std::string from, int level);
    void check_roll(int level);
    void delete_persist_node(
        BPlusTree *node, std::string path);
    void copy_node(BPlusTree *node, int level);

    // how many components does the nth level have
    // std::unordered_map<int, int> level2components;
    Value get_from_file(std::string fn, ValueIndex ptr);
    void update_meta(int level, 
        std::vector<std::string> new_meta);

};

}   // namespace templatedb

int read_component_size(std::string fn);
void copy_file(std::string path1, std::string path2);

#endif /* _TEMPLATEDB_DB_H_ */