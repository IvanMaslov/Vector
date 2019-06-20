//
// Created by maslov on 19.06.19.
//

#ifndef PRO_VEC_H
#define PRO_VEC_H

#include <algorithm>
#include <cassert>
#include <cstring>

template<class T>
class vec {

    struct Big {
        size_t links;
        size_t size;
        size_t capacity;
        T data[];
    };

    union Data {
        T small;
        Big *big;
        Data() : big(nullptr) {};
        ~Data() {};
    } _data;

    size_t isSmall;

    Big *allocBig(size_t capacity = 1) {
        Big *res = static_cast<Big*>(operator new(sizeof(Big) + sizeof(T) * capacity));
        res->links = 1;
        res->size = 0;
        res->capacity = capacity;
        return res;
    }

    Big *cloneBig(Big *arg, size_t prefered_size = 0) {
        assert(arg != nullptr);
        Big *cop = allocBig(std::max(arg->capacity, prefered_size));
        cop->size = arg->size;
        for (size_t i = 0; i < arg->size; ++i)
            new(cop->data + i) T(arg->data[i]);
        return cop;
    }


    void make_big() {
        if (isSmall == 2)
            return;
        Big *t = allocBig();
        if (isSmall == 1) {
            new(t->data) T(_data.small);
            t->size = 1;
        }
        _data.small.~T();
        _data.big = t;
        isSmall = 2;
    }

    void unfollow(Big* arg){
        if(0 == --(arg->links)){
            for(size_t i = 0; i < arg->size; ++i)
                arg->data[i].~T();
            operator delete(static_cast<void*>(arg));
        }
        arg = nullptr;
    }

    void cntr(){
        if(isSmall == 2){

        }
    }

public:


    typedef T value_type;

    typedef T* iterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> reverse_const_iterator;
    typedef reverse_const_iterator const_reverse_iterator;



    vec() noexcept;

    vec(const vec &);

    template<class InputIterator>
    vec(InputIterator, InputIterator);

    vec &operator=(const vec &);

    ~vec() noexcept;

    template<class InputIterator>
    void assign(InputIterator, InputIterator);

    T &operator[](size_t);

    const T &operator[](size_t) const;

    T &front() { return operator[](0); };

    const T &front() const { return operator[](0); };

    T &back() { return operator[](size() - 1); };

    const T &back() const { return operator[](size() - 1); };

    void push_back(const T &);

    void pop_back() { resize(size() - 1); };

    T* data() const noexcept;

    iterator begin() { return data() + 0; };
    const_iterator begin() const { return data() + 0; };

    iterator end() { return data() + size(); };
    const_iterator end() const { return data() + size(); };

    reverse_iterator rbegin() { return std::reverse_iterator(end()); };
    reverse_const_iterator rbegin() const { return std::reverse_iterator(end()); };

    reverse_iterator rend() { return std::reverse_iterator(begin()); };
    reverse_const_iterator rend() const { return std::reverse_iterator(begin()); };

    bool empty() const noexcept { return size() == 0; };

    size_t size() const noexcept;

    void reserve(size_t);

    size_t capacity() noexcept;

    void shrink_to_fit();

    void resize(size_t);

    void clear() noexcept;


    // iterator insert(const_iterator pos, T const& val) {return nullptr;}; //TODO: init
    // iterator erase(const_iterator pos) {return nullptr;}; //TODO: init
    // iterator erase(const_iterator first, const_iterator last) {return nullptr;}; //TODO: init

    iterator insert(const_iterator pos, T const& val){
        vec<T> cop;
        iterator r = begin();
        while(r != pos){
            cop.push_back(*r);
            ++r;
        }
        size_t offset = cop.size();
        cop.push_back(val);
        while(r != end()){
            cop.push_back(*r);
            ++r;
        }
        swap(*this, cop);
        return begin() + offset;
    }


    iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

    iterator erase(const_iterator first, const_iterator last) {
        vec<T> cop;
        iterator t = begin();
        while(t != first) cop.push_back(*(t++));
        while(t != last) t++;
        size_t result_offset = cop.size();
        while(t != end()) cop.push_back(*(t++));
        swap(*this, cop);
        return begin() + result_offset;
    }
    template<class E>
    friend void swap(vec<E> &, vec<E> &);

};

template<class T>
int cmp(const vec<T> &, const vec<T> &);

template<class T>
bool operator==(const vec<T> &, const vec<T> &);

template<class T>
bool operator!=(const vec<T> &, const vec<T> &);

template<class T>
bool operator<(const vec<T> &, const vec<T> &);

template<class T>
bool operator>(const vec<T> &, const vec<T> &);

template<class T>
bool operator<=(const vec<T> &, const vec<T> &);

template<class T>
bool operator>=(const vec<T> &, const vec<T> &);


template<class T>
inline vec<T>::vec() noexcept : isSmall(0) {
    _data.big = nullptr;
}

template<class T>
inline vec<T>::vec(const vec &arg) : isSmall(arg.isSmall) {
    if (isSmall == 0) {
        return;
    }
    if (isSmall == 1) {
        new(&_data.small) T(arg._data.small);
        return;
    }
    if (isSmall == 2) {
        //TODO: COW
        _data.big = cloneBig(arg._data.big);
    }
}


template<class T>
inline vec<T> &vec<T>::operator=(const vec &arg) {
    if (this == &arg)
        return *this;
    //TODO: COW
    vec<T> arg_copy(arg);
    swap(arg_copy, *this);
    return *this;
}

template<class T>
inline vec<T>::~vec() noexcept {
    if (isSmall == 2) {
        unfollow(_data.big);
    }
    if(isSmall == 1){
        _data.small.~T();
    }
}

template<class T>
inline T &vec<T>::operator[](size_t index) {
    return const_cast<T&>(const_cast<const vec<T>*>(this)->operator[](index));
}

template<class T>
inline const T &vec<T>::operator[](size_t index) const {
    if (isSmall == 1)return _data.small;
    return _data.big->data[index];
}

template<class T>
inline void vec<T>::push_back(const T &arg) {

    if (isSmall == 0) {
        isSmall = 1;
        try {
            new(&_data.small) T(arg);
        } catch (...) {
            isSmall = 0;
            throw;
        }
        return;
    }
    vec<T> c;
    if(isSmall == 1) {
        c._data.big = allocBig(2);
        new(c._data.big->data) T(_data.small);
        c._data.big->size = 1; }
    else c._data.big = cloneBig(_data.big, std::max(_data.big->capacity, _data.big->size + 1));
    c.isSmall = 2;
    new (c._data.big->data + size()) T(arg);
    c._data.big->size++;
    swap(c, *this);
}


template<class T>
inline T* vec<T>::data() const noexcept {
    if(isSmall == 2) return _data.big->data;
    return const_cast<T*>(&_data.small);
}


template<class T>
inline size_t vec<T>::size() const noexcept {
    if (isSmall < 2)return isSmall;
    return _data.big->size;
}

template<class T>
inline void vec<T>::reserve(size_t len) {
    vec<T> c;
    c._data.big = allocBig(len);
    c.isSmall = 2;
    c._data.big->size = size();
    for (size_t i = 0; i != size(); ++i)
        new (c._data.big->data + i) T(operator[](i));
    swap(c, *this);
}

template<class T>
inline size_t vec<T>::capacity() noexcept {
    if (isSmall == 0)return 0;
    if (isSmall == 1)return 1;
    return _data.big->capacity;
}

template<class T>
inline void vec<T>::shrink_to_fit() {
    vec<T> c;
    c.reserve(size());
    for (size_t i = 0; i < size(); ++i)
        c.push_back(operator[](i));
    swap(c, *this);
}

template<class T>
inline void vec<T>::resize(size_t len) {
    if (isSmall == 0) {
        isSmall = 2;
        _data.big = allocBig(len);
        return;
    }
    if (isSmall == 1) {
        make_big();
    }
    if(size() >= len){
        for(size_t i = len; i < size(); ++i)
            data()[i].~T();
        _data.big->size = len;
        return;
    }
    vec<T> c;
    c.reserve(len > size() ? len : size());
    for (size_t i = 0; i != len && i != size(); ++i)
        new (c.data() + i) T(operator[](i));
    c._data.big->size = len;
    swap(c, *this);
}

template<class T>
inline void vec<T>::clear() noexcept {
    if (isSmall == 0)return;
    if (isSmall == 1) {
        isSmall = 0;
        return;
    }
    if (isSmall == 2) {
        isSmall = 2;
        _data.big->size = 0;
        return;
    }
}

template<class T>
inline void swap(vec<T> &lhs, vec<T> &rhs) {
    //std::swap(lhs._data, rhs._data);
    // std::swap(lhs._data.big, rhs._data.big);
    // std::swap(lhs._data.small, rhs._data.small);
    if(lhs.isSmall == 2 && rhs.isSmall == 2) std::swap(lhs._data.big, rhs._data.big);
    if(lhs.isSmall != 2 && rhs.isSmall != 2) std::swap(lhs._data.small, rhs._data.small);
    if((lhs.isSmall == 2) ^ (rhs.isSmall == 2)){
        vec<T> & c = ((lhs.isSmall == 2) ? lhs : rhs);
        vec<T> & d = ((rhs.isSmall != 2) ? rhs : lhs);
        void* r = operator new(std::max(sizeof(T), sizeof(void*)));
        memcpy(r, &c._data.big, std::max(sizeof(T), sizeof(void*)));
        new (&c._data.small) T(d._data.small);
        d._data.small.~T();
        memcpy(&d._data.big, r, std::max(sizeof(T), sizeof(void*)));
        operator delete(r);
    }
    std::swap(lhs.isSmall, rhs.isSmall);
}

template<class T>
inline int cmp(const vec<T> &lhs, const vec<T> &rhs) {
    for (size_t i = 0; i < lhs.size(); ++i)
        if (lhs[i] != rhs[i]) {
            return lhs[i] < rhs[i] ? -1 : 1;
        }
    if (lhs.size() != rhs.size())
        return static_cast<const int>(lhs.size()) - static_cast<int>(rhs.size());
    return 0;
}

template<class T>
bool operator==(const vec<T> &lhs, const vec<T> &rhs) { return cmp(lhs, rhs) == 0; }

template<class T>
bool operator!=(const vec<T> &lhs, const vec<T> &rhs) { return cmp(lhs, rhs) != 0; }

template<class T>
bool operator<(const vec<T> &lhs, const vec<T> &rhs) { return cmp(lhs, rhs) < 0; }

template<class T>
bool operator>(const vec<T> &lhs, const vec<T> &rhs) { return cmp(lhs, rhs) > 0; }

template<class T>
bool operator<=(const vec<T> &lhs, const vec<T> &rhs) { return cmp(lhs, rhs) <= 0; }

template<class T>
bool operator>=(const vec<T> &lhs, const vec<T> &rhs) { return cmp(lhs, rhs) >= 0; }

template<class T>
template<class InputIterator>
inline vec<T>::vec(InputIterator l, InputIterator r) {
    isSmall = 0;
    vec<T> cop;
    while (l != r) {
        cop.push_back(*l);
        ++l;
    }
    swap(cop, *this);
}


template<class T>
template<class InputIterator>
inline void vec<T>::assign(InputIterator l, InputIterator r) {
    isSmall = 0;
    vec<T> cop;
    while (l != r) {
        cop.push_back(*l);
        ++l;
    }
    swap(cop, *this);
}


#endif //PRO_VEC_H