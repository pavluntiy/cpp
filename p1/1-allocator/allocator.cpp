#include "allocator.h"
#include "cstdlib"
#include <iostream>
#include <cstring>



Allocator::Allocator(void *base, size_t buf_size):base(base), buf_size(buf_size)
{
    this->total_pointers = 0;

	this->block_size = 32;
	this->memory_map = static_cast<bool*>(base);
	this->n_blocks = buf_size / (2  * this->block_size + 2 * sizeof(bool) + sizeof(PointerInfo)); 
	this->list_size = this->n_blocks;
    this->hash_map = static_cast<PointerInfo*>(static_cast<void*>(this->memory_map) + 2 * this->n_blocks * sizeof(bool));

	for(size_t i = 0; i < this->list_size; ++i){
		this->memory_map[i] = false;
	}

	this->begin = (this->n_blocks * sizeof(PointerInfo)) + this->hash_map;
	this->end = static_cast<char*>(this->begin) + n_blocks * block_size;

}


size_t Allocator::bytes_to_blocks(size_t bytes)
{
    return (bytes + this->block_size - 1)/this->block_size;
}

index_t Allocator::find_position(size_t required_blocks)
{   

    index_t position = -1;
    bool found = false;
    for(index_t i = 0; i < this->n_blocks; ++i)
    {
        size_t free_blocks_count = 0;
        for(size_t j = i; j < this->n_blocks; ++j){
            if(this->memory_map[j]){
                break;
            }
            else 
            {
                ++free_blocks_count;
            }
        }
        if(free_blocks_count >= required_blocks){
            position = i;
            break;
        }
    }

    return position;
}


index_t Allocator::insert(index_t start_block, size_t n_blocks)
{
    index_t pointer_id = this->total_pointers++;
    this->hash_map[pointer_id].start_block = start_block;
    this->hash_map[pointer_id].n_blocks = n_blocks;

    return pointer_id;
}

void Allocator::remove(index_t pointer_id)
{

    hash_map[pointer_id].n_blocks = -1;
    hash_map[pointer_id].start_block = -1;
    total_pointers--;
}


void Allocator::fill_map(index_t start_block, size_t n_blocks, bool value=true)
{
    for(index_t j = start_block;  j < start_block + n_blocks; ++j){
        memory_map[j] = value;
    }
}



Pointer Allocator::alloc(size_t required_bytes)
{

    

    size_t required_blocks = bytes_to_blocks(required_bytes);
    index_t position = find_position(required_blocks);

    if(position == -1)
    {   
    	throw AllocError(AllocErrorType::NoMemory);
    }

    fill_map(position, required_blocks);

    index_t pointer_id = insert(position, required_blocks);

    return Pointer(this, pointer_id);

}

void* Allocator::resolve(index_t pointer_id)
{   
    if (pointer_id == -1)
    {
        return nullptr;
    }
    return this->begin + get_size_bytes(pointer_id);
}

size_t Allocator::get_size_bytes(index_t pointer_id)
{
    return this->block_size * get_size_blocks(pointer_id);
}

size_t Allocator::get_size_blocks(index_t pointer_id)
{   
    if (pointer_id == -1)
    {
        return 0;
    }
    return this->hash_map[pointer_id].n_blocks;
}

index_t Allocator::get_start_block(index_t pointer_id)
{
    return hash_map[pointer_id].start_block;
}

void Allocator::realloc(Pointer &p, size_t new_size)
{

    size_t orig_size = get_size_blocks(p.get_id());
    index_t start = get_start_block(p.get_id());
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

        memcpy(dist, orig, get_size_bytes(p.pointer_id));

        p = moved;
    }

}


void Allocator::free(Pointer &p)
{   

    index_t start_block = get_start_block(p.pointer_id);
    size_t n_blocks = get_size_blocks(p.pointer_id);
	fill_map(start_block, n_blocks, false);
    remove(p.pointer_id);
    p.set_id(-1); 
    
}


Pointer::Pointer(Allocator *allocator, index_t pointer_id): 
    allocator(allocator), pointer_id(pointer_id)
{

}

void* Pointer::get() const
{
    return allocator->resolve(pointer_id);
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

