#include "templatedb/db.hpp"

using namespace templatedb;

// Compares two intervals according to staring times. 
bool compare_key(
    std::vector<std::pair<int, Value>> i1, 
    std::vector<std::pair<int, Value>> i2) 
{ 
    return (i1.first < i2.first); 
} 

Value DB::get(int key)
{
    if (table.count(key))
        return table[key];
    
    return Value(false);
}

void DB::add2layer(
    std::vector<std::pair<int, Value>> arr, 
    int nth_layer) 
    {
        // put the whole arr at the end of the layer
        layer2parts[nth_layer]++;
        write2layer(arr, nth_layer, layer2parts[nth_layer]);
        
        if (layer2parts[nth_layer] < T - 1) 
        {
            // if that layer now has t parts, add the whole
            // arr to the next layer
            std::vector<std::pair<int, Value>> new_arr = 
                merge_layer(n);
            write2layer(new_arr, nth_layer+1, 
                layer2parts[nth_layer+1]);
            // delete all files of this layer
            layer2parts[nth_layer] = 0;
        } 
    }

void merge_layer(int n)
{
    // implement this

    // get all files from this layer and merge
}

void DB:write2layer(
    std::vector<std::pair<int, Value>> arr, 
    int nth_layer, int part_num) 
{
    // fstream layerout is open file corresponding to 
    // nth_layer and part_num

    // copied from write_to_file()
    layer_out.clear();
    layer_out.seekg(0, std::ios::beg);

    std::string header = 
        std::to_string(table.size()) + 
        ',' + std::to_string(value_dimensions) + '\n';
    // layer_out << header;
    layer_out << to_string(arr.size());
    for(auto item: arr)
    {
        std::ostringstream line;
        std::copy(
            item.second.items.begin(), 
            item.second.items.end() - 1, 
            std::ostream_iterator<int>(line, ","));
        line << item.second.items.back();
        std::string value(line.str());
        layer_out << item.first << ':' << value << '\n';
    }

    return true;
}


void DB::put(int key, Value val)
{
    table[key] = val;
    // store it at the first layer
    // if it is full, merge and check next layer
    if (table.size() >= maxsize) { // it should never be gt
        // map to key value pairs
        std::vector<Value> arr;
        for ( auto it = table.begin(); it != table.end(); ++it )
            arr.push_back(make_pair(it->first, it->second));
        // sort arr
        sort(arr, arr+sizeof(arr)/sizeof(arr[0]), compare_key);
        add2layer(arr, 1);
        // clean the dict
        table.clear()
    }
}


std::vector<Value> DB::scan()
{
    std::vector<Value> return_vector;
    for (auto pair: table)
    {
        return_vector.push_back(pair.second);
    }

    return return_vector;
}


std::vector<Value> DB::scan(int min_key, int max_key)
{
    std::vector<Value> return_vector;
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


void DB::del(int key)
{
    table.erase(key);
}


size_t DB::size()
{
    return table.size();
}


std::vector<Value> DB::execute_op(Operation op)
{
    std::vector<Value> results;
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


bool DB::load_data_file(std::string & fname)
{
    std::ifstream fid(fname);
    if (fid.is_open())
    {
        int key;
        std::string line;
        std::getline(fid, line); // First line is rows, col
        while (std::getline(fid, line))
        {
            std::stringstream linestream(line);
            std::string item;

            std::getline(linestream, item, ',');
            key = stoi(item);
            std::vector<int> items;
            while(std::getline(linestream, item, ','))
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


db_status DB::open(std::string & fname)
{
    this->file.open(fname, std::ios::in | std::ios::out);
    if (file.is_open())
    {
        this->status = OPEN;
        // New file implies empty file
        if (file.peek() == std::ifstream::traits_type::eof())
            return this->status;

        int key;
        std::string line;
        std::getline(file, line); // First line is rows, col
        while (std::getline(file, line))
        {
            std::stringstream linestream(line);
            std::string item;

            std::getline(linestream, item, ',');
            key = stoi(item);
            std::vector<int> items;
            while(std::getline(linestream, item, ','))
            {
                items.push_back(stoi(item));
            }
            this->put(key, Value(items));
            if (value_dimensions == 0)
                value_dimensions = items.size();
        }
    }
    else if (!file) // File does not exist
    {
        this->file.open(fname, std::ios::out);
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
    file.seekg(0, std::ios::beg);

    std::string header = 
        std::to_string(table.size()) + 
        ',' + std::to_string(value_dimensions) + '\n';
    file << header;
    for(auto item: table)
    {
        std::ostringstream line;
        std::copy(
            item.second.items.begin(), 
            item.second.items.end() - 1, 
            std::ostream_iterator<int>(line, ","));
        line << item.second.items.back();
        std::string value(line.str());
        file << item.first << ',' << value << '\n';
    }

    return true;
}