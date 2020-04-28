#include <iostream>
// #include <bits/stdc++.h> 
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
// #include "templatedb/db.hpp"


struct ValueIndex {
    bool null;
    std::streampos ptr;
};

struct BPlusTree {
   std::vector<int> keys; // keys
   std::vector<BPlusTree*> children; // ptrs
   std::vector<ValueIndex> values; // only for leaves
   std::vector<std::string> files; // fn of children
   std::string fn;
   bool is_leaf; // is leaf
   int num_keys; // active children
};

struct ReturnStruct {
    int key;
    BPlusTree *node;
};


BPlusTree* new_tree();
BPlusTree* insert(BPlusTree *root, int key, 
    ValueIndex value);
ValueIndex search(BPlusTree *node, int key);
BPlusTree* restore(std::string fn);
void save_node(BPlusTree *node);


