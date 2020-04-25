#include <iostream>
// #include <bits/stdc++.h> 
#include <vector>
#include <algorithm>
// #include "templatedb/db.hpp"

const int order = 4; // make it even to be safe

struct ValuePtr {
    int ptr;
};

struct BPlusTree {
   std::vector<int> keys; // keys
   std::vector<BPlusTree*> children; // ptrs
   std::vector<ValuePtr> values; // only for leaves
   std::vector<string> files; // fn of children
   string fn;
   bool is_leaf; // is leaf
   int num_keys; // active children
};

struct ReturnStruct {
    int key;
    BPlusTree *node;
};


BPlusTree* new_tree();
void insert(BPlusTree *root, int key, ValuePtr value);
ValuePtr search(int key);


