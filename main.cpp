#include <iostream>
#include <vector>
#include <map>
#include <cmath>


template<typename T, size_t R>
struct logging_allocator {
    using value_type = T;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    std::size_t _byte_reserved = sizeof(T)*R;

    pointer _reserved_memory_ptr = nullptr;

    pointer _free_memory_ptr = nullptr;


    logging_allocator() {
        //std::cout << __PRETTY_FUNCTION__ << "[n = " << byte_reserved << "]" << std::endl;
        _reserved_memory_ptr = reinterpret_cast<T *>(std::malloc(_byte_reserved));
        _free_memory_ptr = _reserved_memory_ptr;

    }
    ~logging_allocator() {
        //std::cout << __PRETTY_FUNCTION__ << "[n = " << byte_reserved << "]" << std::endl;
        std::free(_reserved_memory_ptr);
    }

    template<typename U>
    struct rebind {
        using other = logging_allocator<U, R>;
    };

    T *allocate(std::size_t n) {
        auto p = _free_memory_ptr;
        if (!p)
            throw std::bad_alloc();

        _free_memory_ptr += n; // <======= (Fix 1) ptr arithmetic

        return reinterpret_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t n) { ; }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) {

        // we have to transfer arguments like a &&ref, because class U could have a move_ctor
        new(p) U(std::forward<Args>(args)...);
    }

    void destroy(T *p) {

        if(p != nullptr)
            p->~T();
    }
};

template<typename T, typename U>
struct MapNode {
    using k_type = T;
    using v_type = U;

    k_type _key;
    v_type _value;
    MapNode *_nextElement;

    MapNode(k_type key_, v_type value_, MapNode *nextElement_ = nullptr) :
    _key(key_),
    _value(value_),
    _nextElement(nextElement_) {;}

    k_type getKey() {
        return _key;
    }

    v_type getValue() {
        return _value;
    }

    MapNode* getNext() {
        return _nextElement;
    }

    void setNext(MapNode* next_) {
        _nextElement = next_;
    }
};

template<typename T, typename U, typename Allocator>
struct myMap {
    using k_type = T;
    using v_type = U;
    typedef typename Allocator::template rebind<MapNode<T,U>>::other MyMapAllocator;

    MyMapAllocator _allocator;
    MapNode<T,U>* _headElement;
    MapNode<T,U>* _tailElement;
    size_t _size = 0;

    ~myMap() {

        auto temp = _headElement;
        for(size_t i = 0; i < _size; i++) {
            auto temp1 = temp->getNext();
            _allocator.destroy(temp);
            temp = temp1;
        }
        _allocator.deallocate(_headElement, _size);
    }


    void add(const k_type&& key_, v_type&& value_) {

        MapNode<T,U>* temp = _allocator.allocate(1);
        // (Fix 2) using std::forward to save type like a &&ref
        _allocator.construct(temp, std::forward<const k_type>(key_), std::forward<v_type>(value_));

        if(_size == 0)
            _headElement = temp;
        else
            _tailElement->setNext(temp);

        _tailElement = temp;
        _size++;
    }

    v_type get(const k_type key_) {

        auto temp = _headElement;
        for(size_t i = 0; i < _size; i++) {

            if(temp->getKey() == key_)
                return temp->getValue();

            temp = temp->getNext();
        }
        return _headElement->getValue();
    }

};

void f1() {

    std::cout << " === std::map and std::allocator ===" << std::endl;
    auto m = std::map<int, int, std::less<int>>{};

    for (size_t i = 0; i < 10; ++i) {
        m[i] = std::tgamma(i+1);
        std::cout << i << " " << m[i] << std::endl;
    }
}

void f2() {

    std::cout << " === std::map and my allocator ===" << std::endl;
    auto n = std::map<int, int, std::less<int>, logging_allocator<std::pair<const int, int>, 10>>{};

    for (size_t i = 0; i < 10; ++i) {
        n[i] = std::tgamma(i+1);
        std::cout << i << " " << n[i] << std::endl;
    }
}

void f3() {

    std::cout << " === my map and std::allocator ===" << std::endl;
    auto r = myMap<int, int, std::allocator<MapNode<const int, int>>>{};

    for (size_t i = 0; i < 10; ++i) {
        // (Fix 3) using std::move
        r.add(std::move(i), std::move(std::tgamma(i+1)));
        std::cout << i << " " << r.get(i) << std::endl;
    }
}

void f4() {

    std::cout << " === my map and my allocator ===" << std::endl;
    auto p = myMap<int, int, logging_allocator<std::pair<const int, int>, 10>>{};

    for (size_t i = 0; i < 10; ++i) {
        // (Fix 3) using std::move
        p.add(std::move(i), std::move(std::tgamma(i+1)));
        std::cout << i << " " << p.get(i) << std::endl;
    }
}

int main(int, char *[]) {

    try {

        f1();
        f2();
        f3();
        f4();
    }
    catch(...)
    {
        std::cout << "Unknown error" << std::endl;
    }

    return 0;
}
