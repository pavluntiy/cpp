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

using namespace std;






using doc_id_t = unsigned long long;
using word_id_t = unsigned long long;
using word_cnt_t = unsigned int;

struct MyFile
{
    FILE *f;
    const size_t BUFF_SIZE = 1024 * 1024;
    size_t bytes_read = 0;
    char *buff;

    MyFile(string fname)
    {
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
            int r;
            if(!fread(arr + i, sizeof(char), 1, f))
            {
                throw exception();
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

struct MFile
{
    static int total;
    static vector<string> names;
    FILE *f;

    MFile(string base = "./")
    {   
        stringstream ss;
        ss << base << total;
        total += 1;
        names.push_back(ss.str());
        f = fopen(ss.str().c_str(), "wb");
    }

    ~MFile()
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



};

int MFile::total = 0;
vector<string> MFile::names;


void dump_to_file(map<word_id_t, vector<doc_id_t>> &index, int idx)
{   
    
    // ofstream out(ss.str());
    MFile f("./tosort/some");
    for(auto it: index)
    {   
        f.write(it.first);

        for(auto word:it.second)
        {
            f.write(word);
        }
        // out << "\n==========" << endl;
    }
}


void read_dicts(string fname)
{
   
    MyFile fl(fname);

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
                        dump_to_file(index, total);
                        total += 1;
                        fl.refill();
                        index = map<word_id_t, vector<doc_id_t>>();
                    }
                    auto word = fl.read<word_id_t>();
                    // cout << word << endl;
                    index[word].push_back(doc_id);
                   
                }

            }
       
        }
    }
    catch(...)
    {
        dump_to_file(index, total);
        cout << "Read " << total << endl;
    }

    
}

int main(void)
{
    string f_name;
    cin >> f_name;

    mkdir("tosort", 0777);
    read_dicts(f_name);
    // rmdir("tosort");



    return 0;
}