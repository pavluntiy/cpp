#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

#include <iostream>
#include <vector>
#include <string>
#include <map>

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
        setbuf(f, buff);
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
        T res;
        char *arr = (char *)(void *) &res;

        // auto was_read = fread((char *) (void*)&res, sizeof(res), 1,  f);

        for(int i = sizeof(T) - 1; i >= 0; --i)
        {
            // arr[i] = fgetc(f);
            if(!fread(arr + i, 1, 1, f))
            {
                throw exception();
            }
        }


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
        char* arr = (char*)(void*) &var;

        // cout << var << "||||||" << endl;
        for(int i = (int)sizeof(var) - 1; i >= 0; --i)
        {   
            // cout << arr[i] << endl;
            ungetc(arr[i], f);
            // cout << "FSDDSFSDF\n" << endl;
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

    try
    {
        while(fl.is_open())
        {   
            cout << "FILE WAS RESET" << endl;
            fl.refill();

            map<word_id_t, vector<doc_id_t>> index;
            while(!fl.empty() && fl.is_open())
            {   
                cout << fl.bytes_read << endl;
                auto doc_id = fl.read<doc_id_t>();

                // cout << doc_id << endl;
                if(fl.empty())
                {   
                    
                    fl.unget<doc_id_t>(doc_id);
                    break;
                }

                auto word_cnt = fl.read<word_cnt_t>();
                // cout << word_cnt << endl;
                if(!fl.can_read_n<word_id_t>(word_cnt))
                {   
                    fl.unget<word_cnt_t>(word_cnt);
                    fl.unget<doc_id_t>(doc_id);
                    break;
                }

                // cout << "LOLOLOL\n";
                for(int i = 0; i < word_cnt; ++i)
                {
                    auto word = fl.read<word_id_t>();
                    // cout << word << endl;
                    index[word].push_back(doc_id);
                }

                cout << "Read index entry!" << endl;

                // for(auto it: index)
                // {   
                //     cout << it.first << endl;
                //     for(auto word:it.second)
                //     {
                //         cout << word << ' ';
                //     }
                //     cout << "\n==========" << endl;
                // }

            }
       
        }
    }
    catch(...)
    {

    }

    
}

int main(void)
{
    string f_name;
    cin >> f_name;

    read_dicts(f_name);



    return 0;
}