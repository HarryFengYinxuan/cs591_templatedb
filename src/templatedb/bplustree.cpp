// #include "templatedb/db.hpp"
#include "templatedb/bplustree.hpp"

using namespace std;

string fn_counter = "00000";
const int order = 4; // make it even to be safe
bool persistence = false;

void increment_fn_counter() {
    // increment fn_counter
    string num_repr = fn_counter.substr(
        fn_counter.length()-5, fn_counter.length());
    int num = std::stoi(num_repr);
    if (++num < 10000) {
        // size is safe, we just add 1
        num_repr = std::to_string(num + 1);
        // pad the substring
        while (num_repr.length() < 4) {
            // this condition is satisfied from the start
            num_repr = '0' + num_repr;
        }
        // replace the substring
        for (
            int i=fn_counter.length()-5; 
            i < fn_counter.length();
            i ++) {
                fn_counter[i] = 
                    num_repr[i-(fn_counter.length()-5)];
            }
    } else { // must == 10000
        // new substring
        fn_counter = fn_counter + "_00000";
    }

}

// init tree
BPlusTree* new_tree() {
    BPlusTree *node = new BPlusTree;
    for (int i=0; i<order; i++) {
        node -> keys.push_back(-1);
        node -> values.push_back(ValueIndex{true, 0});
        node -> children.push_back(NULL);
        node -> files.push_back("null");
    }
    node -> children.push_back(NULL);
    node -> files.push_back("null");
    node -> is_leaf = true;
    node -> num_keys = 0;
    node -> fn = fn_counter;

    increment_fn_counter();
    return node;
}

// find the valid child from node
int find_child(BPlusTree *node, int key) {
    // corner cases:
    // no children
    if (node -> num_keys == 0) {
        // suppose it is leaf
        return 0;
        // node -> keys[0] = key;
        // node -> num_keys ++;
    } else if (node -> num_keys < 0) {
        return -1; // invalid
    } else { // positive
        // at least 1 key

        // first child
        if (key < node -> keys[0]) { // less than first key
            return 0;
        }
        // last child
        if (key >= node -> keys[node -> num_keys - 1]) {
            // no less than last key

            return node -> num_keys;
        }

        for (int i = 0; i < node -> num_keys - 1; i++) {
            if (
                node->keys[i] <= key && key < node->keys[i+1]
            ) {
                return i+1;
            }
        }
        return -1;
    }
}

// resotre a bplustree from file recursively
BPlusTree* restore(string fn) {
    BPlusTree *ret = new BPlusTree;
    string line;
    ifstream infile(fn+".bpt", ios::binary);

    getline(infile, line);
    ret -> is_leaf = line[0] == '1';
    getline(infile, line);
    ret -> fn = line;
    getline(infile, line);
    ret -> num_keys = stoi(line);

    // keys
    getline(infile, line);
    stringstream linestream;
    linestream = stringstream(line);
    string item;

    while(getline(linestream, item, ','))
    {
        ret -> keys.push_back(stoi(item));
    }
    for (int i=ret -> num_keys; i < order; i++) {
        ret -> keys.push_back(-1);
    }

    // files
    if (!ret -> is_leaf) {
        getline(infile, line);
        linestream = stringstream(line);
        while(getline(linestream, item, ','))
        {
            ret -> files.push_back(item);
        }
        for (int i=ret -> num_keys; i < order; i++) {
            ret -> files.push_back("null");
        }
    }
    
    // values
    getline(infile, line);
    linestream = stringstream(line);
    while(getline(linestream, item, ','))
    {
        if (item == "null") {
            ret -> values.push_back(ValueIndex{true, 0});
        } else {
            ret -> values.push_back(
                ValueIndex{false, stoi(item)});
        }
    }
    for (int i=ret -> num_keys; i < order; i++) {
        ret -> values.push_back(ValueIndex{true, 0});
    }

    for (int i=0; i < ret -> num_keys+1; i++) {
        if (!ret -> is_leaf) {
            string f = ret -> files[i];
            ret -> children.push_back(restore(f));
        } else {
            ret -> children.push_back(NULL);
        }
    }
    for (int i=ret -> num_keys; i < order; i++) {
        ret -> children.push_back(NULL);
    }
    return ret;
}

// // getting child node from node, because it could be a file
// BPlusTree* get_child(BPlusTree *node, int child) {
//     // if a child is in disk but not in memory, then its
//     // chidlren ptr should be NULL, and its files[chidl]
//     // is a valid file counter name, but not "null"
//     if (node -> children[child] != NULL) {
//         return node -> children[child];
//     } else {
//         // get child from file
//         string line; // buffer
//         BPlusTree *ret = new BPlusTree;
//         ifstream myfile (node -> fn, ios::binary);

//         getline(myfile, line);
//         ret -> keys.push_back();

//     }
// }

// make node and its descendants into a file
void save_node(BPlusTree *node) {
    ofstream outfile(node -> fn + ".bpt", ios::binary);
    outfile << node -> is_leaf << "\n";
    outfile << node -> fn << "\n";
    outfile << node -> num_keys << "\n";
    for (int i=0; i < node -> num_keys; i++) {
        int k = node -> keys[i];
        outfile << to_string(k);
        if (i < node -> num_keys - 1) {
            outfile << ",";
        }
    }
    outfile << "\n";

    if (!node -> is_leaf) {
        for (int i=0; i < node -> num_keys + 1; i++) {
            BPlusTree *c = node -> children[i];
            outfile << c -> fn;

            if (i < node -> num_keys) {
                outfile << ",";
            }
            save_node(c);
        }
        outfile << "\n";
    }

    for (int i=0; i < node -> num_keys; i++) {
        ValueIndex v = node -> values[i];
        if (v.null == true) {
            outfile << "null"; 
        } else {
            outfile << to_string(v.ptr);
        }
        if (i < node -> num_keys - 1) {
            outfile << ",";
        }
    }
    outfile << "\n";
}
// if key in vector, find its location, only for leaf
// if not found, return len
int find_key(BPlusTree *node, int key) {
    vector<int>::iterator begin = node -> keys.begin();
    vector<int>::iterator end = 
        next(begin, node->num_keys);
    int loc = 0;
    for (; loc < node -> num_keys; loc ++) {
        if (node -> keys[loc] == key) {
            break;
        }
    }
    return loc;
}

// sort a leaf's keys and values
void _sort_kv(BPlusTree *node) {
    // assuming that the number of keys and values are the same
    vector<pair<int, ValueIndex>> kv_pairs;
    for (int i=0; i < node -> num_keys; i++){
        kv_pairs.push_back(make_pair(
            node -> keys[i],
            node -> values[i]));
    }
    sort(
        kv_pairs.begin(), kv_pairs.end(), 
        [](pair<int, ValueIndex> a, pair<int, ValueIndex> b) {
            return a.first < b.first;
        }
    );
    for (int i=0; i < node -> num_keys; i++){
        node -> keys[i] = kv_pairs[i].first;
        node -> values[i] = kv_pairs[i].second;
    }
}

// insert key into leaf node, if leaf is not full
void _insert2leaf(BPlusTree *node, int key, ValueIndex value) {
    // if duplicates, update
    vector<int>::iterator begin = node->keys.begin();
    vector<int>::iterator end = 
        next(begin, node->num_keys);
    int loc = find_key(node, key);
    // else, try just adding and sorting
    node -> keys[loc] = key;
    node -> values[loc] = value;
    if (loc == node -> num_keys) { // no duplicate
        node -> num_keys ++;
        // sort(node->keys.begin(), end);
        _sort_kv(node);
    }
}

// for internal node, add child, and shift
void _insert_child(BPlusTree* node, BPlusTree* new_child) {
    // child was full, find index for new key
    int new_key = new_child -> keys[0]; 
    // it should have at least 1 key
    int new_index;
    for (
        new_index = 0; 
        new_index < node -> num_keys; 
        new_index ++) 
    {
        if (new_key < node->keys[new_index]) break;
    }

    // shifting everything to the right
    node -> num_keys ++;
    int temp_key; // = new_key;
    BPlusTree *temp_tree; // = new BPlusTree; // placeholder
    for (
        ; 
        new_index < node -> num_keys; 
        new_index ++) 
    {
        temp_key = node -> keys[new_index];
        temp_tree = node -> children[new_index+1];
        // last temp is NULL

        node -> keys[new_index] = new_key;
        node -> children[new_index+1] = new_child;

        new_key = temp_key;
        new_child = temp_tree;
    }
}

// insert into a full leaf
void _insert_split_leaf(
    BPlusTree *node, int key, ValueIndex value, ReturnStruct *ret) 
    {
    // create a temp key value pair, then sort
    // add to it, and sort
    node -> keys.push_back(key);
    node -> values.push_back(value);
    node -> num_keys = order + 1; // or just ++
    // sort(node -> keys.begin(), node -> keys.end());
    _sort_kv(node);
    // split to left and right
    vector<int> left_keys;
    vector<int> right_keys;
    vector<ValueIndex> left_values;
    vector<ValueIndex> right_values;
    int total_len = order+1;
    int mid = total_len/2; // implicit floor
    int i = 0;
    for (; i < mid; i++) {
        left_keys.push_back(node -> keys[i]);
        left_values.push_back(node -> values[i]);
    }
    for (; i < total_len; i++) {
        right_keys.push_back(node -> keys[i]);
        right_values.push_back(node -> values[i]);
    }
    // padding left
    for (int j=mid; j < total_len; j++) {
        left_keys.push_back(-1);
        left_values.push_back(ValueIndex{0});
    }
    // padding right
    for (int j=0; j < mid; j++) {
        right_keys.push_back(-1);
        right_values.push_back(ValueIndex{0});
    }
    // old leaf node
    node -> keys = left_keys;
    node -> values = left_values;
    node -> num_keys = mid;
    // new leaf node with no parent :(
    BPlusTree *new_child = new_tree();
    new_child -> keys = right_keys;
    new_child -> values = right_values;
    new_child -> num_keys = total_len-mid;

    ret -> node = new_child;
    ret -> key = new_child -> keys[0]; // at least one key
    return;
}

// call this when we need to add child to internal
// but internal is full, we split
void _insert_split_internal(
    BPlusTree *node, int key, ValueIndex value, 
    BPlusTree *old_child, ReturnStruct *ret) {

    // append normally, and then split, effectively sort
    node -> keys.push_back(-1); // dummy 
    node -> children.push_back(NULL); // dummy
    node -> num_keys ++;
    _insert_child(node, old_child);

    // spliting
    vector<int> left_keys;
    vector<int> right_keys;
    vector<BPlusTree*> left_children;
    vector<BPlusTree*> right_children;
    int total_len = order + 1;
    int mid = total_len / 2; // implicit floor
    int i = 0;
    for (; i < mid; i++) {
        left_keys.push_back(node -> keys[i]);
        left_children.push_back(node -> children[i]);
    }
    left_children.push_back(node -> children[i]);
    i ++; // skipping mid
    right_children.push_back(node -> children[i]);
    for (; i < total_len; i++) {
        right_keys.push_back(node -> keys[i]);
        right_children.push_back(node -> children[i+1]);
    }

    // old internal node
    node -> keys = left_keys;
    node -> num_keys = mid;
    node -> children = left_children;

    // new leaf node with no parent :(
    BPlusTree *new_child = new_tree();
    new_child -> keys = right_keys;
    new_child -> num_keys = total_len-mid-1; // = mid
    new_child -> children = right_children;
    ret -> node = new_child;
    ret -> key = node -> keys[mid];
    return;
}

// if max, return the newly allocated node ptr
void _insert(
    BPlusTree *node, int key, ValueIndex value,
    ReturnStruct *ret) {
    if (node -> num_keys < order) { // have room
        // load all chidlren
        if (node -> is_leaf) { // leaf
            _insert2leaf(node, key, value);
        } else { // not leaf
            int child_index = find_child(node, key);
            ReturnStruct *res = new ReturnStruct;
            _insert(node->children[child_index], 
                key, value, res);
            node -> is_leaf = false;
            int new_key = res -> key;
            BPlusTree *new_child = res -> node;

            if (new_child != NULL) {
                // child splited, but we have space
                _insert_child(node, new_child);
            }
        }
        ret -> node = NULL; // no split
        // persistence
        return; 
    } else { // no room
        if (node -> is_leaf) {
            ReturnStruct *res = new ReturnStruct;
            _insert_split_leaf(node, key, value, res);
            ret -> key = res -> key;
            ret -> node = res -> node;
            return;
        } else {
            int child_index = find_child(node, key);
            ReturnStruct *res = new ReturnStruct;
            _insert(node->children[child_index], 
                key, value, res);
            node -> is_leaf = false;
            BPlusTree *new_child = res -> node;
            if (new_child != NULL) {
                // splited
                _insert_split_internal(
                    node, res->key, value, res->node, ret);
                // add node to self and split
                // ret = res;
                return;
            } else { 
                // no more action needed
                ret -> node = NULL;
                return;
            }
        }
    }
}

// interface
BPlusTree* insert(BPlusTree *root, int key, ValueIndex value) {
    if (root == NULL) { // initialize tree
        root = new_tree();
    }
    ReturnStruct *res = new ReturnStruct;
    _insert(root, key, value, res);
    if (res -> node != NULL) {
        // if inserted into root, and splited
        BPlusTree *temp = root;
        root = new_tree();
        root -> keys[0] = res -> key;
        root -> children[0] = temp;
        root -> children[1] = res -> node;
        root -> num_keys = 1;
        root -> is_leaf = false;
    }
    return root;
}

ValueIndex search(BPlusTree *node, int key) {
    if (node -> is_leaf) {
        for (int i=0; i < node -> num_keys; i++) {
            if (node -> keys[i] == key) {
                return node -> values[i];
            }
        }
        ValueIndex ret = {
            .null = true,
        };
        return ret;
    } else {
        // find in children
        return search(
            node -> children[find_child(node, key)], 
            key);
    }
}