#include "templatedb/db.hpp"

using namespace std; 
using namespace templatedb;

// Compares two intervals according to staring times. 
bool compare_key(pair<int, Value> i1, pair<int, Value> i2) 
{ 
    return (i1.first < i2.first); 
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

// find the value of the corresponding key in level
Value DB::get_from_level(int key, int level) {
    // todo
    // find in this level by binary search
    // if fails, try next level

    // use bloom filter to check
    return Value(false);
}

// write arr as a component to level
void DB::add2level(
    vector<pair<int, Value>> arr, 
    int level) 
    {
        // appending a file to a level
        // are we just changing its name?
        if (policy == 0) {
            // leveling
            // we added to the new level, so just merge
            merge_level(level);
            // the # of cmpnnts is always 1
            // if too big, add to next level
        } else if (policy == 1) {
            // tiering
            // we merged and then adding to the next level
            // as a component, we are done
            level2components[level]++;
            // if too big, merge and add to next level
        }
        // // put the whole arr at the end of the level
        // level2components[nth_level]++;
        // write2level(arr, nth_level, level2components[nth_level]);
        
        // if (level2components[nth_level] < T - 1) 
        // {
        //     // if that level now has t parts, add the whole
        //     // arr to the next level
        //     vector<pair<int, Value>> new_arr = 
        //         merge_level(n);
        //     write2level(new_arr, nth_level+1, 
        //         level2components[nth_level+1]);
        //     // delete all files of this level
        //     level2components[nth_level] = 0;
        // } 
    }

void DB::merge_level(int n)
{
    // todo
    // if leveling, then try putting level L in the end
    // of level L+1, and then merge that level

    // i guess this merges in place, and then it requires
    // putting this into the next level if tiering

    // open all components on the same level, open a new file
    // repeatedly find the smallest yet to append 

    // anti matter?
}

// // maybe no need for this func
// void DB::write2level(
//     vector<pair<int, Value>> arr, 
//     int nth_level, int part_num) 
// {
//     // todo: partitioning?
//     // fstream levelout is open file corresponding to 
//     // nth_level and part_num

//     // copied from write_to_file()
//     level_out.clear();
//     level_out.seekg(0, ios::beg);

//     string header = 
//         to_string(table.size()) + 
//         ',' + to_string(value_dimensions) + '\n';
//     // level_out << header;
//     level_out << to_string(arr.size());
//     for(auto item: arr)
//     {
//         ostringstream line;
//         copy(
//             item.second.items.begin(), 
//             item.second.items.end() - 1, 
//             ostream_iterator<int>(line, ","));
//         line << item.second.items.back();
//         string value(line.str());
//         level_out << item.first << ':' << value << '\n';
//     }

//     return true;
// }


void DB::put(int key, Value val)
{
    // store at memory component
    table[key] = val;
    // if it is full, merge and check next level
    if (table.size() >= maxsize) { // it should never be gt
        // map to key value pairs
        vector<pair<int, Value>> arr;
        for (auto it = table.begin(); 
            it != table.end(); 
            ++it ) 
            {
            arr.push_back(make_pair(it->first, it->second));
        }
        // sort arr
        sort(arr.begin(), arr.end(), compare_key);
        // write 2 a temp memory file, and append that file
        // to level 1
        // todo
        // open level 1 file, write to it

        // flush
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
    table.erase(key);
    // and insert anti-matter
    put(key, Value(false));
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

void DB::write2component(
    vector<pair<int, Value>> arr, string fn) {
    // write to the actual file and to the B+ tree for indexing

    ofstream myfile (fn, ios::binary);
    string header = 
        to_string(table.size()) + 
        ',' + to_string(value_dimensions) + '\n';
    myfile << header;
    // todo get b+ tree

    streampos pos;
    for (auto item: arr) {
        pos = myfile.tellp();
        // insert key, pos to b+ tree
        // write 2 file
        ostringstream line;
        copy(
            item.second.items.begin(), 
            item.second.items.end() - 1, 
            ostream_iterator<int>(line, ","));
        line << item.second.items.back();
        string value(line.str());
        file << item.first << ',' << value << '\n';
    }
} 