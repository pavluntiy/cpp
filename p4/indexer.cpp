#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>

#include <exception>
#include <algorithm>
#include <set>

using namespace std;






using doc_id_t = unsigned long long;
using word_id_t = unsigned long long;
using word_cnt_t = unsigned int;
using offset_t = unsigned long long;


class InvalidRead:public exception
{

};


template<typename T>
void my_write(FILE *f, T var)
{
    char* arr = (char*)(void*) &var;
    for(int i = 0; i < sizeof(var); ++i)
    {   
        fwrite(arr + i, sizeof(char), 1, f);
    }
}


template<typename T>
T my_read(FILE *f)
{
    T res;
    char *arr = (char *)(void *) &res;

    for(int i = sizeof(T) - 1; i >= 0; --i)
    {
        if(!fread(arr + i, sizeof(char), 1, f))
        {
            throw InvalidRead();
        }
    }

    return res;
}



struct MyFile
{
    FILE *f;
    const size_t BUFF_SIZE = 1024;
    size_t bytes_read = 0;
    char *buff;
    string fname;

    MyFile(string fname)
    {   
        this->fname = fname;
        f = fopen(fname.c_str(), "rb");
        buff = new char[BUFF_SIZE];
        setvbuf(f, buff,  _IOFBF , BUFF_SIZE);
    }

    ~MyFile()
    {
        delete []buff;
        fclose(f);
    }

    bool is_open()
    {   
        return !feof(f);
    }

    void refill()
    {
        bytes_read = 0;
    }

    bool empty()
    {
        if(bytes_read >= BUFF_SIZE)
        {
            return true;
        }

        return false;
    }

    template<typename T>
    T read()
    {
        // cout << this << endl;
        T res;
        char *arr = (char *)(void *) &res;

        // auto was_read = fread((char *) (void*)&res, sizeof(res), 1,  f);

        for(int i = sizeof(T) - 1; i >= 0; --i)
        {
            // arr[i] = fgetc(f);
            // cout << (void*) (arr + i) << " " << &res << ' ';
            // int r;
            if(!fread(arr + i, sizeof(char), 1, f))
            {
                throw InvalidRead();
            }
        }


        bytes_read += sizeof(res);
        return res;
    }

    template<typename T>
    void unget(T var)
    {
        char* arr = (char*)(void*) &var;
        for(int i = 0; i < sizeof(var); ++i)
        {   
            ungetc(arr[i], f);
        }
    }

    template<typename T>
    bool can_read_n(int n)
    {
        return bytes_read + n * sizeof(T) < BUFF_SIZE;
    }
};

// struct MFile
// {
//     static int total;
//     static vector<string> names;
//     FILE *f;

//     MFile(string base = "./")
//     {   
//         stringstream ss;
//         ss << base << total;
//         total += 1;
//         names.push_back(ss.str());
//         f = fopen(ss.str().c_str(), "wb");
//     }

//     ~MFile()
//     {
//         fclose(f); 
//     }

//     template<typename T>
//     void write(T var)
//     {
//         char* arr = (char*)(void*) &var;
//         for(int i = 0; i < sizeof(var); ++i)
//         {   
//             fwrite(arr + i, sizeof(char), 1, f);
//         }
//     }



// };



// int MFile::total = 0;
// vector<string> MFile::names;

struct MapFileWriter
{
    FILE* f;
    vector<offset_t> dict_offsets;
    string fname;
    MapFileWriter(string name)
    {   
        fname = name;
        f = fopen(name.c_str(), "wb");    

    }

    ~MapFileWriter()
    {
        fclose(f); 
    }

    template<typename T>
    void write(T var)
    {
        char* arr = (char*)(void*) &var;
        for(int i = 0; i < sizeof(var); ++i)
        {   
            fwrite(arr + i, sizeof(char), 1, f);
        }
    }

    void dump(map<word_id_t, vector<doc_id_t>> &index)
    {      
    
        for(auto it: index)
        {   
            write(it.first);
            // cout << it.first << endl;
            sort(it.second.begin(), it.second.end());
            write<unsigned long long>(it.second.size());

            for(auto word:it.second)
            {
                write(word);
            }

        }
        dict_offsets.push_back(lseek(fileno(f), 0, SEEK_CUR));
    }

};


struct IndexInfo
{
    vector<offset_t> offsets;
    string tmp_fname;
    set<word_id_t> word_ids;

    IndexInfo()
    {

    }

    IndexInfo (IndexInfo &&other)
    {
        offsets = move(other.offsets);
        tmp_fname = move(other.tmp_fname);
        word_ids = move(other.word_ids);
    }
};





IndexInfo read_dicts(string fname)
{
   
    MyFile fl(fname);
    MapFileWriter fw("tmpfile.bin");

    set<word_id_t> word_ids;

    cout << &fl;
    int total  = 0;
    map<word_id_t, vector<doc_id_t>> index;
    try
    {

        while(fl.is_open())
        {   
            cout << "FILE WAS RESET" << endl;
            fl.refill();

            
            while(!fl.empty() && fl.is_open())
            {   
                // cout << fl.bytes_read << endl;
                auto doc_id = fl.read<doc_id_t>();
                // int i;

                // cout << doc_id << endl;
                // if(fl.empty())
                // {   
                    
                //     fl.unget<doc_id_t>(doc_id);
                //     break;
                // }

                auto word_cnt = fl.read<word_cnt_t>();
                // int j;
                // cout << word_cnt << ' ' << doc_id << endl;
                // if(!fl.can_read_n<word_id_t>(word_cnt))
                // {   
                //     cout << "DUMP TO FILE " << endl;  
                // }

                // cout << "LOLOLOL\n";
                for(int i = 0; i < word_cnt; ++i)
                {   
                    // cout << index.size() << ">>>>>>\n";
                    if(!fl.can_read_n<word_id_t>(1))
                    {   


                        // cout << "DUMP TO FILE " << index.size() << endl; 
                        // dump_to_file(index, total);
                        fw.dump(index);
                        total += 1;
                        fl.refill();
                        index = map<word_id_t, vector<doc_id_t>>();
                        
                    }
                    auto word = fl.read<word_id_t>();
                    // cout << word << endl;
                    index[word].push_back(doc_id);
                    word_ids.insert(word);
                   
                }

            }
       
        }
    }
    catch(InvalidRead e)
    {
        fw.dump(index);
        cout << "Read " << total << endl;
    }

    IndexInfo index_info;
    index_info.offsets = move(fw.dict_offsets);
    index_info.tmp_fname = fw.fname;
    index_info.word_ids = move(word_ids);

    return index_info; 
}

class FileProxy
{


    FILE *f;
    offset_t offset;

    word_id_t current_word;
    bool word_valid;
    doc_id_t current_doc;
    bool doc_valid;

    unsigned long long doc_counter;

public:
    FileProxy(string fname, offset_t offset)
    {
        doc_valid = false;
        word_valid = false;

        this->offset = offset;
        f = fopen(fname.c_str(), "rb");
        lseek(fileno(f), offset, SEEK_SET);
        doc_counter = 0;
    }

    ~FileProxy()
    {
        // fclose(f);
    }

    word_id_t get_current_word()
    {
        if(word_valid && doc_counter > 0)
        {
            return current_word;
        }

        current_word = my_read<word_id_t>(f);
        doc_counter = my_read<unsigned long long>(f);
    }

    doc_id_t get_next_doc()
    {
        if(doc_counter > 0)
        {
            return  my_read<doc_id_t>(f);
        }

        throw InvalidRead();
    }

    // FileProxy(FileProxy &&other)
    // {
    //     doc_valid = move(other.doc_valid);
    // }
};

void write_index(IndexInfo &index_info)
{
    map <word_id_t, offset_t> final_offsets;
    vector<FileProxy> proxies;

    for(auto& offset: index_info.offsets)
    {
        proxies.push_back(FileProxy(index_info.tmp_fname, offset));
    }

    FILE *output = fopen("result.bin", "wb");

    for(auto word: index_info.word_ids)
    {
        my_write<word_id_t>(output, word);
        my_write<offset_t>(output, 0);
    }
    my_write<word_id_t>(output, 0);
    my_write<offset_t>(output, 0);


}

int main(void)
{
    string f_name;
    cin >> f_name;

    IndexInfo index_info = read_dicts(f_name);
    write_index(index_info);



    return 0;
}