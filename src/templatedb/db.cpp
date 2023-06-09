#include "templatedb/db.hpp"

using namespace templatedb;


Value DB::get(int key)
{
    // query the buffer
    Value result = this->buffer.get(key);

    // query the disk
    if ((result.visible == false) && (result.range == -404)) {
        result = this->level.get(key);
    }
    return result;
    /*
    if (table.count(key))
        return table[key];
    
    return Value(false);*/
}


void DB::put(int key, templatedb::Value val)
{
    this->buffer.put(key, val);
    if (this->buffer.isFull()) this->flush();
    /* table[key] = val; */
}

void DB::flush() {
    std::tuple<run, int, int> oldBuffer = this->buffer.flushOut();
    this->level.flushIn(oldBuffer);
}
std::vector<Value> DB::scan()
{
    std::vector<Pair> bufferResult = this->buffer.scan();
    std::vector<Pair> diskResult = this->level.scan();
    std::vector<Value> finalResult = this->finalMerge(bufferResult, diskResult);
    return std::move(finalResult);
    return std::vector<Value>(0);
    /* std::vector<Value> return_vector;
    for (auto pair: table)
    {
        return_vector.push_back(pair.second);
    }

    return return_vector; */
}


std::vector<Value> DB::scan(int min_key, int max_key)
{
    std::vector<Pair> bufferResult = this->buffer.scan(min_key, max_key);
    //std::cout<<"############################"<<std::endl;
    std::vector<Pair> diskResult = this->level.scan(min_key, max_key);
    std::vector<Value> finalResult = this->finalMerge(bufferResult, diskResult);
    return std::move(finalResult);
    //std::merge();

    /*
    std::vector<Value> return_vector;
    for (auto pair: table)
    {
        if ((pair.first >= min_key) && (pair.first <= max_key))
            return_vector.push_back(pair.second);
    }

    return return_vector;*/
}


void DB::del(int key)
{
    this->buffer.del(key);
    //table.erase(key);
}


void DB::del(int min_key, int max_key)
{
    this->buffer.del(min_key, max_key);
    /*for (auto it = table.begin(); it != table.end(); ) {
        if ((it->first >= min_key) && (it->first <= max_key)){
            table.erase(it++);
        } else { 
            ++it;
        }
    }*/
}


size_t DB::size()
{
    this->buffer.size();
    //return table.size();
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
        if ( op.args.size()>0 ){
            this->del(op.key, op.args[0]);
        }
        else
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
        int line_num = 0;
        std::string line;
        std::getline(fid, line); // First line is rows, col
        while (std::getline(fid, line))
        {
            line_num++;
            std::stringstream linestream(line);
            std::string item;

            std::getline(linestream, item, ' ');
            std::string op_code = item;

            std::getline(linestream, item, ' ');
            key = stoi(item);
            std::vector<int> items;
            while(std::getline(linestream, item, ' '))
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

    std::string header = std::to_string(table.size()) + ',' + std::to_string(value_dimensions) + '\n';
    file << header;
    for(auto item: table)
    {
        std::ostringstream line;
        std::copy(item.second.items.begin(), item.second.items.end() - 1, std::ostream_iterator<int>(line, ","));
        line << item.second.items.back();
        std::string value(line.str());
        file << item.first << ',' << value << '\n';
    }

    return true;
}
std::vector<Value> DB::finalMerge(run higher, run lower) {

    /*
    run resultSet(higher.size() + lower.size());
    std::merge(higher.begin(), higher.end(), lower.begin(), lower.end(), std::back_inserter(resultSet), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    return resultSet;
    */
    size_t i = 0;
    size_t j = 0;
    run resultSet;
    resultSet.reserve(higher.size() + lower.size());
    while (i < higher.size() && j < lower.size()) {
        if (higher.at(i).first == lower.at(j).first) {
            resultSet.push_back(higher.at(i++));
            if (lower.at(j).second.visible || lower.at(j).second.range <= 0) {
                // Not a range tombstone
                j++;
            }
        } else if (higher.at(i).first < lower.at(j).first) {
            resultSet.push_back(higher.at(i++));
        } else {
            resultSet.push_back(lower.at(j++));
        }
    }

    if (i < higher.size()) {
        resultSet.insert(resultSet.end(), higher.begin() + i, higher.end());
    } else if (j < lower.size()) {
        resultSet.insert(resultSet.end(), higher.begin() + j, lower.end());
    }

    vector<Value> finalResult(resultSet.size());
    for (auto &pair: resultSet) {
        finalResult.push_back(pair.second);
    }
    return std::move(finalResult);
}
