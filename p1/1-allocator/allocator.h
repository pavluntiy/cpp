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

    // struct BlockList;
    // {
    //     BlockList *next;
    //     void *block;
    //     size_t size;

    //     BlockList(BlockList *next = nullptr, void *block = nullptr, size_t size = 0):
    //         next(next), block(block), size(size)
    //     {}
    // };

    // BlockList *blockList;
    struct MapEntry
    {
        int start_block;
        int n_blocks;
    };

    size_t block_size;

    size_t n_blocks;

    bool *memory_map;
    MapEntry *hash_map;

    size_t buf_size;
    size_t list_size;
    void *base;
    void *begin;
    void *end;

    int total_pointers;

    size_t allocated_size;

    friend class Pointer;

    void* __resolve__(index_t);
    size_t __get_size_blocks__(index_t);
    index_t __get_start_block__(index_t);
    size_t __get_size_bytes__(index_t);

    void unregister(index_t);



public:
    Allocator(void*, size_t);
    
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag() {}
    std::string dump() { return ""; }

};

