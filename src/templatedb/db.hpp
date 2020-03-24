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


class Value
{
public:
    std::vector<int> items;
    bool visible = true;

    Value() {}
    Value(bool _visible) {visible = _visible;}
    Value(std::vector<int> _items) { items = _items;}

    bool operator ==(Value const & other) const
    {
        return (visible == other.visible) && (items == other.items);
    }
};


class DB
{
public:
    db_status status;

    DB() {};
    ~DB() {close();};

    Value get(int key);
    void put(int key, Value val);
    std::vector<Value> scan();
    std::vector<Value> scan(int min_key, int max_key);
    void del(int key);
    size_t size();

    db_status open(std::string & fname);
    bool close();

    bool load_data_file(std::string & fname);

    std::vector<Value> execute_op(Operation op);

    // added
    int T = 2;
    int maxsize = 10; // size of nth layer = maxsize * n * T

private:
    std::fstream file;
    std::unordered_map<int, Value> table;
    size_t value_dimensions = 0;
    
    bool write_to_file();

    // added
    // adding an array in nth layer
    void add2layer(
        std::vector<std::pair<int, Value>> arr, int nth_layer);

    void write2layer(int n);
    void merge_layer(int n);

    std::unordered_map<int, int> layer2parts;
    // how many parts does the nth layer have
};

}   // namespace templatedb

#endif /* _TEMPLATEDB_DB_H_ */