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

#include "templatedb/operation.hpp"

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

    Value() {}
    Value(bool _visible) {visible = _visible;}
    Value(std::vector<int> _items) { items = _items;}

    bool operator ==(Value const & other) const
    {
        return (visible == other.visible) && 
            (items == other.items);
    }
};


class DB
{
public:
    db_status status;

    DB() {};
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
    // one off open?
    // when do you load? only memory?

    // general operation function
    std::vector<Value> execute_op(Operation op);

    // added
    int policy; // 0 = leveling; 1 = tiering
    int T = 2;
    int maxsize = 10; // # of value in memory component
    // int max_level; // need this if realistic

private:
    std::fstream file;
    // memory component
    std::unordered_map<int, Value> table;
    size_t value_dimensions = 0; // init inside open
    
    bool write_to_file();

    // added
    // adding an array in nth level
    void add2level(
        std::vector<std::pair<int, Value>> arr, int level);

    void write2component(int n);
    void merge_level(int n);
    Value get_from_level(int key, int level);
    string get_fn(int level, int component); // get filename
    pair<int, int> parse_fn(string fn); // parse filename
    vector<string> get_level_fn(int level); // list files

    // how many components does the nth level have
    std::unordered_map<int, int> level2components;

};

}   // namespace templatedb

#endif /* _TEMPLATEDB_DB_H_ */