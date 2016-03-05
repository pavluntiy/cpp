#include "allocator.h"
#include "cstdlib"
#include <iostream>
#include <cstring>



Allocator::Allocator(void *base, size_t buf_size):base(base), buf_size(buf_size)
{
    this->total_pointers = 0;

	this->block_size = 32;
	this->memory_map = static_cast<bool*>(base);
	this->n_blocks = buf_size / (2  * this->block_size + 2 * sizeof(bool) + sizeof(MapEntry)); 
	this->list_size = this->n_blocks;
    this->hash_map = static_cast<MapEntry*>(static_cast<void*>(this->memory_map) + 2 * this->n_blocks * sizeof(bool));

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


    index_t pointer_id = this->total_pointers++;
    // void *pointer = this->begin + this->block_size * cur_block_number;
    this->hash_map[pointer_id].start_block = cur_block_number;
    this->hash_map[pointer_id].n_blocks = required_n;

    // std::cout << "Allocating memory: " <<  pointer << " < " << this->end << "\n";//found << " " << cur_block_number << " of " << n_blocks << "\n";
    return Pointer(this, pointer_id);


}

void* Allocator::__resolve__(index_t pointer_id)
{   
    if (pointer_id == -1)
    {
        return nullptr;
    }
    return this->begin + __get_size_bytes__(pointer_id);
}

size_t Allocator::__get_size_bytes__(index_t pointer_id)
{
    return this->block_size * __get_size_blocks__(pointer_id);
}

size_t Allocator::__get_size_blocks__(index_t pointer_id)
{   
    if (pointer_id == -1)
    {
        return 0;
    }
    return this->hash_map[pointer_id].n_blocks;
}

index_t Allocator::__get_start_block__(index_t pointer_id)
{
    return hash_map[pointer_id].start_block;
}

void Allocator::realloc(Pointer &p, size_t new_size)
{

    size_t orig_size = __get_size_blocks__(p.get_id());
    index_t start = __get_start_block__(p.get_id());
    size_t n_blocks = (new_size + this->block_size - 1)/this->block_size;

    if(n_blocks <= orig_size){
        std::cout << "LOLOL\n";
        for(index_t i = start + n_blocks; i < start + orig_size; ++i){
            this->memory_map[i] = false;
        }
        this->hash_map[p.get_id()].n_blocks = n_blocks;

    }
    else{

        Pointer moved = this->alloc(new_size);

        void *dist = moved.get();
        void *orig = p.get();

        memcpy(dist, orig, __get_size_bytes__(p.pointer_id));

        p = moved;
    }

}

void Allocator::unregister(index_t pointer_id)
{

    hash_map[pointer_id].n_blocks = -1;
    hash_map[pointer_id].start_block = -1;
    total_pointers--;
}

void Allocator::free(Pointer &p)
{   

    index_t start = __get_start_block__(p.pointer_id);
    size_t n_blocks = __get_size_blocks__(p.pointer_id);
	for(index_t j = start;  j < start + n_blocks; ++j){
        memory_map[j] = false;
    }

    unregister(p.pointer_id);
    p.set_id(-1);

    
}


Pointer::Pointer(Allocator *allocator, index_t pointer_id): 
    allocator(allocator), pointer_id(pointer_id)
{

}

void* Pointer::get() const
{
    return allocator->__resolve__(pointer_id);
}

index_t Pointer::get_id() const
{
    return this->pointer_id;
}

void Pointer::set_id(index_t pointer_id)
{
    this->pointer_id = pointer_id;
}

// size_t Pointer::get_size() const
// {
//     return this->allocator->__get_size__(this->number);
// }

