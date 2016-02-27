#include "allocator.h"
#include "cstdlib"
#include <iostream>



Allocator::Allocator(void *base, size_t buf_size):base(base), buf_size(buf_size)
{

	this->block_size = 32;
	this->memory_map = static_cast<bool*>(base);
	this->n_blocks = buf_size / 2 / this->block_size;
	this->list_size = this->n_blocks;

	for(size_t i = 0; i < this->list_size; ++i){
		this->memory_map[i] = false;
	}

	this->begin = (this->list_size * sizeof(bool)) + static_cast<char*>(this->base);
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
    	throw AllocError(AllocErrorType::NoMemory);
    }

    for(size_t j = cur_block_number;  j - cur_block_number < required_n; ++j){
    	memory_map[j] = true;
    }


    return Pointer(this->begin + this->block_size * cur_block_number);


}

void Allocator::realloc(Pointer &p, size_t N)
{

}

void Allocator::free(Pointer &p)
{
	// p.set(nullptr);
}


Pointer::Pointer(void* pointer):pointer(pointer)
{

}

void* Pointer::get() const
{
    return this->pointer;
}

void Pointer::set(void *pointer)
{
    this->pointer = pointer;
}
