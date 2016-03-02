#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <tuple>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>


enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {
public:
    Pointer() : _pos(nullptr) {
        Pointer::_pointers.insert(this);
    }
    Pointer(Pointer const& p): _pos(p._pos) {
        Pointer::_pointers.insert(this);
    }

    ~Pointer() {
        Pointer::_pointers.erase(this);
    }
    void *get() const { return _pos; } 
    friend Allocator;
private:
    void *_pos;
    static std::set<Pointer*> _pointers;
};


class Allocator {
public:
    Allocator(void *base, size_t size);
    
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag();
    std::string dump() { return ""; }    
private:
    void                    *_base;
    size_t                   _size;
    std::vector<bool>        _used;
    std::map<size_t,size_t>  _alloced;
};

#endif // ALLOCATOR_H
