#include <iostream>
#include <vector>
#include <functional>


using namespace std;

using Vec = vector<double>;

// typename C::iterator a_start, typename C::iterator a_stop, 
//         typename C::iterator b_start, typename C::iterator b_stop, 
//         typename C::iterator output,

template<typename C, class Cmp>
void merge
    (
        C a_start, C a_stop, 
        C b_start, C b_stop, 
        C output,
        Cmp cmp 
    )
{
    while(a_start < a_stop && b_start < b_stop)
    {
        if(cmp(*a_start, *b_start))
        {
            *output = *a_start;
            ++a_start;
        }
        else
        {
            *output = *b_start;
            ++b_start;
        }
        ++output;
    }

    while(a_start < a_stop)
    {
        *output = *a_start;
        ++a_start;
        ++output;   
    }

    while(b_start < b_stop)
    {
        *output = *b_start;
        ++b_start;
        ++output;   
    }
}

template<typename C, class Cmp = std::less<typename C::value_type>>
void merge_sort(C &vec, Cmp cmp)
{   
    auto start = begin(vec);
    auto stop = end(vec);



    if(vec.size() == 1)
    {
        return;
    }

    auto medium = start + (stop - start)/2;

    auto left =  C(start, medium);
    auto right = C(medium, stop);

    #pragma omp parallel
    {
        #pragma omp sections
        {   
            #pragma omp section
            {
                // cout << "A" << endl;
                merge_sort(left, cmp);
                // cout << "/A" << endl;
            }

            #pragma omp section
            {   
                // cout << "B" << endl;
                merge_sort(right, cmp);
                // cout << "/B" << endl;
            }
        }
    }

    // cout << "Going to merge it " << endl;

    merge(begin(left), end(left), begin(right), end(right), begin(vec), cmp);

    // for(auto &it: vec)
    // {
    //     cout << it << ' ';
    // }
    // cout << endl;

    // cout << "Finished" << endl;

}

int main()
{
    Vec v;

    double in;
    while(cin >> in)
    // for(int i = 0; i < 100 * 1000 * 1000; ++i)
    {
        v.push_back(in);
    }

    merge_sort(v, std::less<double>());

    for(auto &it: v)
    {
        cout << it << ' ';
    }
    cout << endl;
}