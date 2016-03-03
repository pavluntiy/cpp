#include <stdexcept>
#include <string>

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
    void *pointer;
    Allocator* allocator;
    int number;
    friend class Allocator;
public:
    Pointer(Allocator *allocator = nullptr, int number = -1);
    void* get() const; 
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

    void *resolve(int);

public:
    Allocator(void*, size_t);
    
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag() {}
    std::string dump() { return ""; }

};

