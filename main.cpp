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

    std::size_t byte_reserved = sizeof(T)*R;

    pointer reserved_memory_ptr = nullptr;

    pointer free_memory_ptr = nullptr;


    logging_allocator()
    {
        std::cout << __PRETTY_FUNCTION__ << "[n = " << byte_reserved << "]" << std::endl;
        reserved_memory_ptr = reinterpret_cast<T *>(std::malloc(byte_reserved));
        free_memory_ptr = reserved_memory_ptr;

    }
    ~logging_allocator()
    {
        std::cout << __PRETTY_FUNCTION__ << "[n = " << byte_reserved << "]" << std::endl;
        std::free(reserved_memory_ptr);
    }

    template<typename U>
    struct rebind {
        using other = logging_allocator<U, R>;
    };

    T *allocate(std::size_t n) {
        auto p = free_memory_ptr;
        if (!p)
            throw std::bad_alloc();

        free_memory_ptr += sizeof(value_type) * n;
        return reinterpret_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t n) { ; }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) {
        new(p) U(std::forward<Args>(args)...);
    }

    void destroy(T *p) {
        p->~T();
    }
};


template<typename T, typename U>
struct MapNode
{
    using key_type = T;
    using value_type = U;

    key_type _key;
    value_type _value;
    MapNode *_nextElement;

    MapNode(key_type key_, value_type value_, MapNode *nextElement_ = nullptr) :
    _key(key_),
    _value(value_),
    _nextElement(nextElement_) {;}

    key_type getKey() {
        return _key;
    }

    value_type getValue() {
        return _value;
    }

    MapNode& getNext() {
        return _nextElement;
    }

    void setNext(MapNode& next_) {
        _nextElement = next_;
    }
};

template<typename T, typename U, typename Allocator>
struct myMap
{
    using key_type = T;

    using value_type = U;

    typedef typename Allocator::template rebind<MapNode<T,U>>::other MyMapAllocator;

    MyMapAllocator _allocator;

    MapNode<T,U>* headElement;

    void add(const key_type key_, value_type value_)
    {
        MapNode<T,U>* temp = _allocator.allocate(48);
        _allocator.construct(temp, key_, value_);
        headElement = temp;
    }

    value_type get(const key_type key_)
    {
        headElement->getValue();
    }

};

int main(int, char *[]) {

    /*
    //std map and std allocator
    auto m = std::map<int, int, std::less<int>>{};
    for (size_t i = 0; i < 10; ++i) {
        m[i] = std::tgamma(i+1);
        std::cout << i << " " << m[i] << std::endl;
    }

    //std map and my allocator
    auto n = std::map<int, int, std::less<int>, logging_allocator<std::pair<const int, int>, 10>>{};
    for (size_t i = 0; i < 10; ++i) {
        n[i] = std::tgamma(i+1);
        std::cout << i << " " << n[i] << std::endl;
    }
*/

    auto map = myMap<const int, int, logging_allocator<std::pair<const int, int>, 10>>{};

    map.add(1,20);
    std::cout << map.get(1) << std::endl;



    return 0;
}
