#include "templatedb/db.hpp"

using namespace std; 
using namespace templatedb;

string keyvalue2str(pair<int,Value> p) {
    ostringstream line;
    line << p.first << ",";
    copy(
        p.second.items.begin(), 
        p.second.items.end() - 1, 
        ostream_iterator<int>(line, ","));
    line << p.second.items.back();
    return line.str();
}

pair<int,Value> parse2keyvalue(string str) {
    stringstream linestream(str);
    string item;

    getline(linestream, item, ',');
    int key = stoi(item);
    vector<int> items;
    while(getline(linestream, item, ','))
    {
        items.push_back(stoi(item));
    }
    Value v(items);
    v.null = false;
    return make_pair(key, v);
}

// standard api
Value DB::get(int key)
{
    // if in memory, return
    if (table.count(key))
        return table[key];
    
    // else, try rolling get
    return get_from_level(key, 1);
}

// read a line from ptr of datafile fn, implies not null
Value DB::get_from_file(string fn, ValueIndex ptr) {
    // open data file
    ifstream datafile (fn, ios::binary);
    // seek ptr
    datafile.seekg(ptr.ptr);
    // read line, parse it
    string line;
    getline(datafile, line);
    datafile.close();
    return parse2keyvalue(line).second;
}

vector<string> DB::get_level_fn(int level) {
    // a folder for each level
    // read the meta info, each line has a file
    string folder = to_string(level);
    ifstream myfile (folder + "/.meta", ios::binary);
    string item;
    vector<string> ret;
    while(getline(myfile, item))
    {
        ret.push_back(item);
    }
    myfile.close();
    return ret;
}

void DB::update_meta(int level, vector<string> new_meta) {
    // a folder for each level
    // read the meta info, each line has a file
    string folder = to_string(level);
    ofstream myfile (folder + "/.meta", ios::binary);
    for (auto item: new_meta) {
        myfile << item << endl;
    }
    myfile.close();
}

// find the value of the corresponding key in level
Value DB::get_from_level(int key, int level) {
    // todo
    // get all components of the level
    vector<string> fn_arr = get_level_fn(level);
    string path = to_string(level) + "/";
    // from the last one, use b+ tree
    while (!fn_arr.empty())
    {
        // lets skip bloomfilter for now
        // restore from {fn_arr.back()}.bpt
        BPlusTree* root = restore(fn_arr.back(), path); 
        // search the tree
        ValueIndex ptr = search(root, key);
        delete root;
        // if found, return. else keep going
        if (!ptr.null) {
            return get_from_file(
                path + fn_arr.back() + ".data", 
                ptr
            );
        }
        fn_arr.pop_back();
    }
    // if not found, check if there is another level
    if (level+1<=maxlvl) {
        return get_from_level(key, level+1);
    }
    // checked the last level, nothing found
    return Value(false);
}

// write arr as a component to level
// mainly called from the memory to append to level 1
void DB::add2level(vector<pair<int, Value>> arr, int level) {
    // find the name for new component
    vector<string> files = get_level_fn(level);
    string path = to_string(level) + "/";
    string comp_name = to_string(files.size());
    // writing a new file to level, and make tree
    streampos pos;
    ofstream comp_file (path + comp_name+".data", ios::binary);
    BPlusTree *root = new_tree();
    comp_file << arr.size() << endl;
    for (auto pair: arr) {
        pos = comp_file.tellp();
        comp_file << keyvalue2str(pair) << "\n";
        ValueIndex v = {
            .null = false,
            .ptr = pos,
        };
        root = insert(root, pair.first, v);
    }
    comp_file.close();
    root -> fn = comp_name;
    save_node(root, path);
    delete root;
    // update meta
    files.push_back(comp_name);
    update_meta(level, files);
    check_roll(level);
}

void DB::check_roll(int level) {
    if (level > maxlvl) {
        cout << "DB FULL" << endl;
        return;
    }
    vector<string> files = get_level_fn(level);
    string path = to_string(level) + "/";
    // actions depending on policy
    if (policy == 0) {
        // leveling
        // we added to the new level, so just merge
        merge_level(level);
        // the # of cmpnnts is always 1
        clear_level(level);
        move2level("temp_merge", level, level);
        int comp_size = read_component_size(path+"0");
        int max_lvl_size = maxsize * pow(T, level);
        if (comp_size > max_lvl_size) {
            // maxsize * pow(T, level) is max for this level
            // > maxsize*(pow(T, level)-1) then no more adding
            // cannot fit another one, roll
            move2level("0", level, level+1);
            update_meta(level, vector<string>({}));
            check_roll(level + 1);
        }
        // if too big, add to next level
    } else if (policy == 1) {
        // tiering
        // we merged and then adding to the next level
        // as a component, we are done
        if (files.size() > T) {
            // if too big, merge and add to next level
            merge_level(level);
            clear_level(level);
            move2level("temp_merge", level, level+1);
            // update_meta(level, vector<string>({}));
            check_roll(level + 1);
        }

    }
}

// fn is the component name
int read_component_size(string fn) {
    ifstream comp_file (fn+".data", ios::binary);
    string line;
    getline(comp_file, line);
    comp_file.close();
    return stoi(line);
}

// // add comp to level, and check for rolling
// void DB::rolling_add(string comp, int level) {
//     move2level(comp, level-1, level);

//     // we moved one component to next level
//     // that only happens after leveling full or
//     // tiering full, so last level is empty
//     update_meta(level-1, vector<string>({}));

//     string path = to_string(level) + "/";

//     // actions depending on policy
//     if (policy == 0) {
//         // leveling
//         // we added to the new level, so just merge
//         merge_level(level);
//         // the # of cmpnnts is always 1
//         clear_level(level);
//         move2level("temp_merge", level, level);
//         int comp_size = read_component_size(path+"0");
//         if (comp_size <= (maxsize * pow(T, level)-1)) {
//             // maxsize * pow(T, level) is max for this level
//             // > maxsize*(pow(T, level)-1) then no more adding
//             // cannot fit another one, roll
//             rolling_add("0", level + 1);
//         }
//         // if too big, add to next level
//     } else if (policy == 1) {
//         // tiering
//         // we merged and then adding to the next level
//         // as a component, we are done
//         vector<string> files = get_level_fn(level);
//         if (files.size() > T) {
//             // if too big, merge and add to next level
//             merge_level(level);
//             clear_level(level);
//             rolling_add("temp_merge", level+1);
//         }

//     }
// }

// move a preexisting component to next level
// same merge policy here
// we move comp from level-1 to level
void DB::move2level(string comp, int from_level, 
int to_level) {
    // handle possible merge
    vector<string> files = get_level_fn(to_level);
    string path = to_string(from_level) + "/"; // from
    // moving node
    BPlusTree *root = restore(comp, path);
    string old_fn = root -> fn;
    root -> fn = to_string(files.size());
    copy_node(root, to_level); // copy node to next level
    root -> fn = old_fn;
    if (from_level != to_level) {
        delete_persist_node(root, path); // del node for cur
        // otherwise it stores the same nodes in the same
        // files, and they will be deleted by this
    } else { // same level
        remove((path + comp + ".bpt").c_str());
    }
    
    delete root;
    // moving data
    copy_file(
        path + comp + ".data", 
        to_string(to_level) + "/" + 
        to_string(files.size()) + ".data"
    );
    files.push_back(to_string(files.size()));
    update_meta(to_level, files);
    remove((path + comp + ".data").c_str());
}

void copy_file(string path1, string path2) {
    std::ifstream  src(path1, std::ios::binary);
    std::ofstream  dst(path2, std::ios::binary);
    dst << src.rdbuf();
    src.close();
    dst.close();
}

// merge all data files in this level to temp, remake tree
void DB::merge_level(int level)
{
    string path = to_string(level) + "/";
    ofstream out(to_string(level)+"/temp_merge.data");
    streampos pos;
    string line;
    vector<string> files = get_level_fn(level);
    vector<ifstream> streams;
    vector<pair<int, Value>> pairs; 
    int total_size = 0;
    // pairs[i] is the current read from streams[i]

    for (auto file: files) {
        streams.push_back(
            ifstream(path+file+".data", ios::binary));
        getline(streams[streams.size()-1], line); // meta
        total_size += stoi(line);
        getline(streams[streams.size()-1], line);
        pairs.push_back(parse2keyvalue(line));
    }

    int active_streams = files.size();
    BPlusTree *root = new_tree();
    out << total_size << endl;
    // change this into something that compares the pairs
    // with the same key from  different components but only
    // inserts one
    while (active_streams > 0) {
        pair<int, Value> min_pair = 
            make_pair(MAX_INT, Value(true));
        int min_index = -1;
        // we find the latest one with smallest key
        for (int i=0; i < files.size(); i++) {
            if (pairs[i].first <= min_pair.first) {
                if (
                    pairs[i].first == min_pair.first &&
                    pairs[i].first != MAX_INT
                    // basically not dummy
                ) {
                    // same key as last time
                    // the last thing should be discarded
                    // we read the next thing from the stream

                    if (getline(streams[min_index], line)) {
                        // more to read from here
                        pairs[min_index] = parse2keyvalue
                            (line);
                    } else {
                        // this one is done, we use a dummy
                        pairs[min_index] = make_pair
                            (MAX_INT, Value(true));
                        active_streams --;
                        streams[min_index].close();
                    }
                }

                // updating the smallest
                min_index = i;
                min_pair = pairs[i];
            }
        }
        pos = out.tellp();
        ValueIndex v = {
            .null = false, .ptr = pos,
        };
        out << keyvalue2str(min_pair) << endl;
        root = insert(root, min_pair.first, v);

        // when getting the next stream, we get it for 
        // the one we chose, and the one we discard, 
        // which has the smallest key but old
        if (getline(streams[min_index], line)) {
            // more to read from here
            pairs[min_index] = parse2keyvalue(line);
        } else {
            // this one is done, we use a dummy
            pairs[min_index] = make_pair(MAX_INT, Value(true));
            active_streams --;
            streams[min_index].close();
        }
    }
    root -> fn = "temp_merge";
    save_node(root, path);
    delete root;
    out.close();
}

void DB::put(int key, Value val)
{
    // store at memory component
    table[key] = val;
    // if it is full, merge and check next level
    if (table.size() >= maxsize) { // should never be gt
        // map to key value pairs
        vector<pair<int, Value>> arr;
        for (auto it = table.begin(); 
            it != table.end(); 
            ++it ) 
            {
            arr.push_back(make_pair(it->first, it->second));
        }
        // sort arr
        sort(
            arr.begin(), arr.end(), 
            [](pair<int, Value> a, pair<int, Value> b) {
                return a.first < b.first;
            });
        // write 2 a temp memory file, and append that file
        add2level(arr, 1);
        table.clear();
    }
}


vector<Value> DB::scan()
{
    // get priority queue
    // for each level
    vector<Value> return_vector;
    for (auto pair: table)
    {
        return_vector.push_back(pair.second);
    }

    return return_vector;
}

vector<Value> DB::scan(int min_key, int max_key)
{
    vector<Value> return_vector;
    for (auto pair: table)
    {
        if (
            (pair.first >= min_key) && 
            (pair.first <= max_key)
        )
            return_vector.push_back(pair.second);
    }

    return return_vector;
}

// if delete before put, do we care?
void DB::del(int key)
{
    // if in table
    // table.erase(key);
    // and insert anti-matter
    Value v;
    v.visible = false;
    table[key] = v;
}

size_t DB::size()
{
    return table.size();
}

vector<Value> DB::execute_op(Operation op)
{
    vector<Value> results;
    if (op.type == GET)
    {
        results.push_back(this->get(op.key));
    }
    else if (op.type == PUT)
    {
        this->put(op.key, Value(op.args));
    }
    else if (op.type == SCAN)
    {
        results = this->scan(op.key, op.args[0]);
    }
    else if (op.type == DELETE)
    {
        this->del(op.key);
    }

    return results;
}


bool DB::load_data_file(string & fname)
{
    ifstream fid(fname);
    if (fid.is_open())
    {
        int key;
        string line;
        getline(fid, line); // First line is rows, col
        while (getline(fid, line))
        {
            stringstream linestream(line);
            string item;

            getline(linestream, item, ',');
            key = stoi(item);
            vector<int> items;
            while(getline(linestream, item, ','))
            {
                items.push_back(stoi(item));
            }
            this->put(key, Value(items));
        }
    }
    else
    {
        fprintf(stderr, "Unable to read %s\n", fname.c_str());
        return false;
    }

    return true;
}

db_status DB::open(string & fname)
{
    this->file.open(fname, ios::in | ios::out);
    if (file.is_open())
    {
        this->status = OPEN;
        // New file implies empty file
        if (file.peek() == ifstream::traits_type::eof())
            return this->status;

        int key;
        string line;
        getline(file, line); // First line is rows, col
        while (getline(file, line))
        {
            stringstream linestream(line);
            string item;

            getline(linestream, item, ',');
            key = stoi(item);
            vector<int> items;
            while(getline(linestream, item, ','))
            {
                items.push_back(stoi(item));
            }
            this->put(key, Value(items));
            if (value_dimensions == 0)
                // size of first item? why?
                value_dimensions = items.size();
        }
    }
    else if (!file) // File does not exist
    {
        this->file.open(fname, ios::out);
        this->status = OPEN;
    }
    else
    {
        file.close();
        this->status = ERROR_OPEN;
    }

    return this->status; 
}

bool DB::close()
{
    if (file.is_open())
    {
        this->write_to_file();
        file.close();
    }
    this->status = CLOSED;
    // clear_db();

    return true;
}

bool DB::write_to_file()
{
    file.clear();
    file.seekg(0, ios::beg);

    string header = 
        to_string(table.size()) + 
        ',' + to_string(value_dimensions) + '\n';
    file << header;
    for(auto item: table)
    {
        ostringstream line;
        copy(
            item.second.items.begin(), 
            item.second.items.end() - 1, 
            ostream_iterator<int>(line, ","));
        line << item.second.items.back();
        string value(line.str());
        file << item.first << ',' << value << '\n';
    }

    return true;
}

void DB::clear_level(int level) {
    string path = to_string(level) + "/";
    vector<string> files = get_level_fn(level);
    while (!files.empty()) {

        remove((path+files.back()+".data").c_str());
        BPlusTree *root = restore(files.back(), path);
        delete_persist_node(root, path);
        delete root;
        // remove((path+files.back()+".bpt").c_str());
        files.pop_back();
    }
    update_meta(level, files);
}

void DB::clear_db() {
    for (int i=1; i<=maxlvl; i++) {
        clear_level(i);
    }
    table.clear();
}

void DB::init(int p) {
    // init meta files
    // boost::filesystem::exists(boost::filesystem::path("0"));
    policy = p;
    for (int level=1; level<=maxlvl; level++) {
        string folder_name = to_string(level);
        boost::filesystem::path folder = 
            boost::filesystem::path(folder_name);
        boost::filesystem::create_directories(folder);
        ofstream myfile (folder_name + "/.meta", ios::binary);
        myfile.close();
    }
}

void DB::delete_persist_node(BPlusTree *node, string path) {
    // delete children
    if (!node -> is_leaf) {
        for (int i=0; i<node -> num_keys+1; i++) {
            delete_persist_node(node -> children[i], path);
        }
    }
    
    // delete self
    remove((path+node->fn+".bpt").c_str());
    // int a = 1;
}

void DB::copy_node(BPlusTree *node, int level) {
    string path = to_string(level) + "/";
    save_node(node, path);
}