#include <stdexcept>
#include <string>

using index_t = int;

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message = ""):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {
private:
    // void *pointer;
    Allocator* allocator;
    index_t pointer_id;

    index_t get_id() const;
    void set_id(index_t);

    friend class Allocator;
public:
    Pointer(Allocator *allocator = nullptr, index_t pointer_id = -1);
    void* get() const; 
    // size_t get_size() const;
};

class Allocator {
protected:
    struct PointerInfo
    {
        int start_block;
        int n_blocks;
    };

    size_t block_size;

    size_t total_blocks;

    bool *memory_map;
    PointerInfo *hash_map;

    size_t buf_size;
    size_t list_size;
    void *base;
    void *begin;
    void *end;

    int defragged;

    int total_pointers;

    size_t allocated_size;

    friend class Pointer;

    void* get_address(index_t);
    void* resolve(index_t);

    size_t get_size_blocks(index_t);
    index_t get_start_block(index_t);
    size_t get_size_bytes(index_t);

    size_t bytes_to_blocks(size_t);
    size_t count_free_blocks(index_t);
    index_t find_position(size_t);

    void fill_map(index_t, size_t, bool);

    index_t insert(index_t, size_t);
    index_t remove(index_t);

    void move(index_t, index_t, size_t, size_t);

    void shrink(index_t, size_t);
    bool realloc_move(Pointer &p, size_t);
    bool realloc_inplace(Pointer &p, size_t);

    void set_start_block(index_t, index_t);
    void set_n_blocks(index_t, size_t);





public:
    Allocator(void*, size_t);
    
    Pointer alloc(size_t);
    void realloc(Pointer& , size_t);
    void free(Pointer&);

    void defrag();
    std::string dump();

};

