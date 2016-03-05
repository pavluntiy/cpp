#include "allocator.h"
#include "cstdlib"
#include <iostream>
#include <sstream>
#include <cstring>



Allocator::Allocator(void *base, size_t buf_size):base(base), buf_size(buf_size)
{   
    defragged = false;

    memset(base, 0, buf_size);
    this->total_pointers = 0;

	this->block_size = 32;
	this->memory_map = (bool*)(base);
	this->total_blocks = buf_size / (2*this->block_size + 2 * sizeof(bool) + sizeof(PointerInfo)); 
    this->hash_map = (PointerInfo*)(void*)((char*)(void*)this->memory_map + 2 * this->total_blocks * sizeof(bool));


    memset(hash_map, -1, total_blocks  * sizeof(PointerInfo));


	this->begin = (void*)(this->total_blocks * sizeof(PointerInfo) + (char*)(this->hash_map));
	this->end = (void*)((char*)this->begin + total_blocks * block_size);

}

void Allocator::set_start_block(index_t pointer_id, index_t start_block)
{
    this->hash_map[pointer_id].start_block = start_block;
}

void Allocator::set_n_blocks(index_t pointer_id, size_t n_blocks)
{
    this->hash_map[pointer_id].n_blocks = n_blocks;
}


size_t Allocator::bytes_to_blocks(size_t bytes)
{
    return (bytes + this->block_size - 1)/this->block_size;
}

size_t Allocator::count_free_blocks(index_t start_block)
{
    size_t free_blocks_count = 0;
    for(size_t i = start_block; i < this->total_blocks + defragged * total_blocks; ++i){
        if(this->memory_map[i]){
            break;
        }
        else 
        {
            ++free_blocks_count;
        }
    }

    return free_blocks_count;
}

index_t Allocator::find_position(size_t required_blocks)
{   

    index_t position = -1;
    for(index_t i = total_blocks * defragged; i < this->total_blocks + total_blocks * defragged; ++i)
    {
        size_t free_blocks_count = count_free_blocks(i);  
        if(free_blocks_count >= required_blocks){
            position = i;
            break;
        }
    }

    return position;
}


index_t Allocator::insert(index_t start_block, size_t required_blocks)
{
    index_t pointer_id = -1;

    for(index_t i = 0; i < this->total_blocks; ++i)
    {
        if(hash_map[i].start_block == -1)
        {
            pointer_id = i;
            break;
        }
    }

    set_start_block(pointer_id, start_block);
    set_n_blocks(pointer_id, required_blocks);

    return pointer_id;
}

index_t Allocator::remove(index_t pointer_id)
{

    set_start_block(pointer_id, -1);
    set_n_blocks(pointer_id, -1);
    total_pointers--;

    return -1;
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

void* Allocator::get_address(index_t start_block)
{
    return (void*)((char*) this->begin + start_block * this->block_size);
}

void* Allocator::resolve(index_t pointer_id)
{   

    if (pointer_id == -1)
    {
        return nullptr;
    }

    return get_address(get_start_block(pointer_id));
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

void Allocator::shrink(index_t pointer_id, size_t required_blocks)
{
        index_t start_block = get_start_block(pointer_id);
        size_t original_blocks = get_size_blocks(pointer_id);
        fill_map(start_block + required_blocks, original_blocks - required_blocks, false);    
        this->hash_map[pointer_id].n_blocks = required_blocks;
}

void Allocator::move(index_t original_start, index_t final_start, size_t original_blocks, size_t final_blocks)
{
        void* origin = get_address(original_start);
        void* destination = get_address(final_start);

        size_t bytes = std::min(original_blocks, final_blocks) * block_size;

        memcpy(destination, origin, bytes);

        fill_map(original_start, original_blocks, false);
        fill_map(final_start, final_blocks, true);
}

bool Allocator::realloc_move(Pointer &p, size_t required_blocks)
{       
        index_t pointer_id = p.get_id();
        index_t new_position = find_position(required_blocks);

        if(new_position == -1)
        {
            throw AllocError(AllocErrorType::NoMemory);
        }

        move(get_start_block(pointer_id), new_position, get_size_blocks(pointer_id), required_blocks);
        set_n_blocks(pointer_id, required_blocks);
        set_start_block(pointer_id, new_position);
  
        return true;
}

bool Allocator::realloc_inplace(Pointer &p, size_t required_blocks)
{   
    index_t pointer_id = p.get_id();
    size_t current_size = get_size_blocks(pointer_id);
    index_t end_block = get_start_block(pointer_id) + current_size;

    if(count_free_blocks(end_block) + current_size >= required_blocks)
    {   
        fill_map(end_block, required_blocks - current_size, true);
        hash_map[pointer_id].n_blocks = required_blocks;
        return true;
    }
    else
    {
        return false;
    }

}

void Allocator::realloc(Pointer &p, size_t new_size)
{
    index_t pointer_id = p.get_id();
    if(pointer_id == -1)
    {
        p = alloc(new_size);
        return;
    }

    size_t original_blocks = get_size_blocks(pointer_id);
    size_t required_blocks = bytes_to_blocks(new_size);

    if(required_blocks <= original_blocks){
        shrink(pointer_id, required_blocks);
    }
    else
    {
        if(!realloc_inplace(p, required_blocks)){
            realloc_move(p, required_blocks);
        }
    }

}


void Allocator::free(Pointer &p)
{   
    index_t start_block = get_start_block(p.get_id());
    size_t n_blocks = get_size_blocks(p.get_id());
	fill_map(start_block, n_blocks, false);
    remove(p.get_id());
    p.set_id(-1); 
}

void Allocator::defrag()
{
    this->defragged = !this->defragged;

    for(index_t pointer_id = 0; pointer_id < this->total_blocks; ++pointer_id)
    {
        if(get_start_block(pointer_id) != -1)
        {
            index_t start_block = get_start_block(pointer_id);

            size_t required_blocks = get_size_blocks(pointer_id);

            index_t new_position = find_position(required_blocks);

            if(new_position == -1)
            {   
                throw AllocError(AllocErrorType::NoMemory);
            }

            move(start_block, new_position, required_blocks, required_blocks);

            set_start_block(pointer_id, new_position);
            set_n_blocks(pointer_id, required_blocks);
        }

    }


}

std::string Allocator::dump()
{
    std::stringstream result;

    // size_t free_blocks = 0;
    // for(index_t i = 0; i < n_blocks; ++i)
    // {
    //     if(memory_map[i] == false && free_blocks > 0)
    //     {
    //         result << "\tUsed memory " << free_blocks << " (from " << i - free_blocks << " to " << i - 1 << ")\n"; 
    //         free_blocks = 0;
    //     }
    //     else
    //     {
    //         free_blocks++;
    //     }
    // }

    // result << "\n=====================\n";

    for(index_t pointer_id = 0; pointer_id < total_blocks; ++pointer_id)
    {
        if(get_start_block(pointer_id) != -1)
        {
            result << "Pointer_id" << pointer_id << ":\n";
            result << "\t\tstart_block: " << get_start_block(pointer_id) << "\n";
            result << "\t\tn_blocks: " << get_size_blocks(pointer_id) << "\n";
            result << "Contains:\n";
            // result << std::hex;
            // unsigned char *start = (unsigned char*)resolve(pointer_id);
            // for(size_t i = 0; i < get_size_bytes(pointer_id); ++i)
            // {
            //     result << "\\x" << (int) start[i] << " ";
            // }    
            // result << "\n:::::::::::::\n";
        }
    }

    return result.str();

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

