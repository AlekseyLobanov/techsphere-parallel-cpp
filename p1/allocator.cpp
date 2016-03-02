#include "allocator.h"

std::set<Pointer*> Pointer::_pointers;

Allocator::Allocator(void *base, size_t size) {
    _base = base;
    _size = size;
    _used = std::vector<bool>(size,false);
}

Pointer Allocator::alloc(size_t N) {
    size_t bgn, end;
    bgn = 0;
    end = _size / 2;
    
    bool is_found   = false;
    size_t good_num = _size + 1;
    
    for (size_t i = bgn; i < end; ++i) {
        if ( _used.at(i) )
            continue;
        size_t free_cnt = 1;
        size_t cur_i    = i;
        for (; i < end; ++i) {
            if ( _used.at(i) ) {
                break;
            } else {
                ++free_cnt;
            }
        }
        if ( free_cnt >= N ) {
            good_num = cur_i;
            is_found = true;
            break;
        }
    }
    Pointer res;
    if ( is_found ) {
        res._pos = static_cast<char*>(_base) + good_num;
        _alloced[good_num] = N;
        for (size_t i = good_num; i < good_num + N; ++i) {
            _used.at(i) = true;
        }
    } else {
        throw AllocError(AllocErrorType::NoMemory, "Error with alloc");
    }
    return res;
}

void Allocator::free(Pointer &p) {
    size_t good_num = size_t(p._pos) - size_t(_base);
    auto it = _alloced.find(good_num);
    if ( it == _alloced.end() )
        throw AllocError(AllocErrorType::InvalidFree, "Error with free: unknown key");
    for (size_t i = good_num; i < good_num + it->second; ++i) {
        _used.at(i) = false;
    }
    _alloced.erase(it);
    p._pos = nullptr;
}

void Allocator::defrag() {
    size_t cur_min = 0;
    std::map<size_t,size_t>  new_alloced;
    for (auto it = _alloced.begin(); it != _alloced.end(); ++it) {
        size_t cur_p_start = it->first;
        size_t cur_size = it->second;
        if ( it->first == cur_min ) {
            new_alloced[cur_min] = cur_size;
            cur_min += cur_size;
            continue;
        }
        size_t delta = cur_p_start - cur_min;
        for (auto p_it = Pointer::_pointers.begin(); p_it != Pointer::_pointers.end(); ++p_it) {
            
            char* real_p = static_cast<char*>((*p_it)->_pos);
            if ( size_t(real_p) == (size_t(_base) + cur_p_start) ) {
                for (size_t cur_pos_used = cur_min; cur_pos_used < cur_min + cur_size; ++cur_pos_used) {
                    real_p[cur_pos_used] = real_p[cur_pos_used + delta];
                }
                (*p_it)->_pos = (void*)(static_cast<char*>(_base) + cur_min);
                new_alloced[cur_min] = cur_size;
            }
        }
        cur_min += cur_size;
    }
    for (size_t i = 0; i < cur_min; ++i) {
        _used.at(i) = true;
    }
    for (size_t i = cur_min; i < _used.size(); ++i) {
        _used.at(i) = false;
    }
    swap(new_alloced, _alloced);
}

void Allocator::realloc(Pointer &p, size_t N) {
    auto old_p = p.get();
    if (old_p == nullptr) {
        p = this->alloc(N);
        return;
    }
    size_t old_size = _alloced.find(size_t(old_p) - size_t(_base))->second;
    if ( N <= old_size ) {
        for (size_t i = N; i < old_size; ++i) {
            _used.at(i) = false;
        }
        _alloced[size_t(old_p) - size_t(_base)] = N;
        return;
    }
    // trying to grow inplace:
    bool can_inplace = true;
    for (size_t i = old_size; i < N; ++i){
        if ( _used.at(i) ) {
            can_inplace = false;
            break;
        }
    }
    if ( can_inplace ) {
        for (size_t i = old_size; i < N; ++i){
            _used.at(i) = true;
        }
        _alloced[size_t(old_p) - size_t(_base)] = N;
        return;
    }
    Pointer new_p = alloc(N);
    for (size_t i = size_t(old_p) - size_t(_base); i < old_size; ++i) {
        static_cast<char*>(new_p.get())[i] = static_cast<char*>(old_p)[i];
        _used.at(i) = false;
    }
    this->free(p);
    p = new_p;
}
