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
public:
    Pointer(void* pointer = nullptr);
    void* get() const; 
    void set(void *);
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

    size_t block_size;

    size_t n_blocks;

    bool *memory_map;

    size_t buf_size;
    size_t list_size;
    void *base;
    void *begin;
    void *end;

    size_t allocated_size;


public:
    Allocator(void*, size_t);
    
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag() {}
    std::string dump() { return ""; }
};

