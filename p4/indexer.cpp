#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;






using doc_id_t = unsigned long long;
using word_id_t = unsigned long long;
using word_cnt_t = unsigned long;

struct MyFile
{
    FILE *f;
    const size_t BUFF_SIZE = 1024 * 1024;
    size_t bytes_read = 0;
    char *buff;

    MyFile(string fname)
    {
        FILE *f = fopen(fname.c_str(), "b");
        buff = new char[BUFF_SIZE];
        setbuf(f, buff);
    }

    ~MyFile()
    {
        delete []buff;
        fclose(f);
    }

    bool is_open()
    {
        return feof(f);
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
        T res;
        fgets((char *) (void*)&res, sizeof(res), f);
        bytes_read += sizeof(res);
        return res;
    }

    // doc_id_t read_doc_id()
    // {
    //     doc_id_t doc_id;
    //     fgets((char *) (void*)&doc_id, sizeof(long long), f);
    //     return doc_id;
    // }

    // word_id_t read_word_id()
    // {
    //     word_id_t word_id;
    //     fgets((char *) (void*)&word_id, sizeof(long long), f);
    //     return word_id;
    // }

    // word_cnt_t read_word_cnt()
    // {
    //     word_cnt_t word_cnt;
    //     fgets((char *) (void*)&word_id, sizeof(long long), f);
    //     return word_cnt;
    // }

    template<typename T>
    void unget(T var)
    {
        char* arr = (char*)(void*) var;

        for(int i = (int)sizeof(var) - 1; i > 0; --i)
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



void read_dicts(string fname)
{
   
    MyFile fl(fname);
    while(fl.is_open())
    {   

        map<word_id_t, vector<doc_id_t>> index;
        while(!fl.empty())
        {   

            auto doc_id = fl.read<doc_id_t>();

            if(fl.empty())
            {
                fl.unget<doc_id_t>(doc_id);
                break;
            }

            auto word_cnt = fl.read<word_cnt_t>();

            if(!fl.can_read_n<word_id_t>(word_cnt))
            {
                fl.unget<word_cnt_t>(word_cnt);
                fl.unget<doc_id_t>(doc_id);
                break;
            }

            for(int i = 0; i < word_cnt; ++i)
            {
                auto word = fl.read<word_id_t>();
                index[word].push_back(doc_id);
            }


        }
   
    }
    
}

int main(void)
{
    string f_name;
    cin >> f_name;




    return 0;
}