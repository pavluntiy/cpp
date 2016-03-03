#include "allocator.h"
#include "cstdlib"
#include <iostream>



Allocator::Allocator(void *base, size_t buf_size):base(base), buf_size(buf_size)
{
    this->total_pointers = 0;

	this->block_size = 32;
	this->memory_map = static_cast<bool*>(base);
	this->n_blocks = buf_size / (2  * this->block_size + sizeof(bool) + sizeof(MapEntry)); 
	this->list_size = this->n_blocks;
    this->hash_map = static_cast<MapEntry*>(static_cast<void*>(this->memory_map) + this->n_blocks * sizeof(bool));

	for(size_t i = 0; i < this->list_size; ++i){
		this->memory_map[i] = false;
	}

	this->begin = (this->n_blocks * sizeof(MapEntry)) + this->hash_map;
	this->end = static_cast<char*>(this->begin) + n_blocks * block_size;

}

Pointer Allocator::alloc(size_t N)
{

    // if(this->buf_size + N > this->buf_size/2)
    // {
    // 	throw AllocError(AllocErrorType::NoMemory, "Out of memory");
    // }
    // std::cout << this->buf_size << ' ' << N;

    size_t cur_block_number = 0;
    bool found = false;

    size_t required_n = (N + this->block_size - 1)/this->block_size;

    for(size_t i = 0; i < this->n_blocks; ++i)
    {
    	int free_blocks_count = 0;
    	for(size_t j = i; j < this->n_blocks; ++j){
    		if(this->memory_map[j]){
    			break;
    		}
    		else 
    		{
    			++free_blocks_count;
    		}
    	}
    	if(free_blocks_count >= required_n){
    		found = true;
    		cur_block_number = i;
    		break;
    	}
    }

    // std::cout << N << " " << (N + this->block_size - 1) << " " << required_n << std::endl;

    if(!found)
    {   
        // std::cout << "Found NoMemory\n";
    	throw AllocError(AllocErrorType::NoMemory);
    }

    for(size_t j = cur_block_number;  j - cur_block_number < required_n; ++j){
    	memory_map[j] = true;
    }


    int p_number = this->total_pointers++;
    void *pointer = this->begin + this->block_size * cur_block_number;
    this->hash_map[p_number].start_block = cur_block_number;
    this->hash_map[p_number].n_blocks = required_n;

    // std::cout << "Allocating memory: " <<  pointer << " < " << this->end << "\n";//found << " " << cur_block_number << " of " << n_blocks << "\n";
    return Pointer(this, p_number);


}

void* Allocator::resolve(int p_number)
{   
    if (p_number == -1)
    {
        return nullptr;
    }
    return this->begin + this->block_size * this->hash_map[p_number].n_blocks;
}

void Allocator::realloc(Pointer &p, size_t N)
{

}

void Allocator::free(Pointer &p)
{   
    int start = this->hash_map[p.number].start_block;
    int n_blocks = this->hash_map[p.number].n_blocks;
	for(size_t j = start;  j < start + n_blocks; ++j){
        memory_map[j] = false;
    }
    p.number = -1;
}


Pointer::Pointer(Allocator *allocator, int number): allocator(allocator), number(number)
{

}

void* Pointer::get() const
{
    return this->allocator->resolve(this->number);
}

