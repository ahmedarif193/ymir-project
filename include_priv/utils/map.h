#ifndef MAP_H
#define MAP_H

#include "vector.h"
#include "string.h"

#define _LEVELS 100
namespace lxcd {


template<typename T1, typename T2>
class pair {
public:
    pair() : key(T1()), value(T2()) {
    }
    pair(const T1& t1, const T2& t2) : key(t1), value(t2) {
    }
    template<typename U1, typename U2>
    pair(const pair<U1, U2>& other) : key(other.key), value(other.value) {
    }
    T1 key;
    T2 value;
};

template<typename T1, typename T2>
pair<T1, T2> make_pair(const T1& t1, const T2& t2) {
    return pair<T1, T2>(t1, t2);
}
template <typename K, typename M>
class map;

//contains the {key, value} pair
template <typename K, typename M>
class Node {
public:
    friend class map<K, M>;
    typedef pair<const K, M> ValueType;

    Node(size_t _level);

    Node(size_t _level, const ValueType& _val);

    //delete forward links and value
    ~Node();

private:
    Node** _forward_links;
    Node* _prev;
    ValueType* _value;
};

//maintains a skiplists of nodes
template <typename K, typename M>
class map {
public:
    //member type
    typedef pair<const K, M> ValueType;

    /*
        defining Iterator class
        helps iterate through elements of the map
 */
    class Iterator {
    public:
        Iterator() = default;
        Iterator(const Iterator& _iter);
        Iterator(Node<K, M>* _node);
        Node<K, M>* get_cur() const;

        //defining operators for Iterator
        Iterator& operator=(const Iterator& _iter);
        bool operator==(const Iterator &_iter) const {
            return (_cur == _iter._cur);
        }
        bool operator!=(const Iterator &_iter) const {
            return !(*this == _iter);
        }
        Iterator& operator++();
        Iterator operator++(int);
        Iterator& operator--();
        Iterator operator--(int);
        ValueType& operator*() const;
        ValueType* operator->() const;

        ~Iterator();

    private:
        Node<K, M>* _cur;

    };

    /*
        defining ConstIterator class
        works exactly like the Iterator but values are const and cannot be changed
 */
    class ConstIterator {
    public:
        ConstIterator(const ConstIterator& _citer);
        ConstIterator(const Iterator& _iter);
        ConstIterator(Node<K, M>* _node);
        Node<K, M>* get_cur() const;

        //defining operators for ConstIterator
        ConstIterator& operator=(const ConstIterator& _citer);
        ConstIterator& operator++();
        ConstIterator operator++(int);
        ConstIterator& operator--();
        ConstIterator operator--(int);
        const ValueType& operator*() const;
        const ValueType* operator->() const;

        ~ConstIterator();

    private:
        Node<K, M>* _cur;
    };

    /*
        defining ReverseIterator
        iterator through the elements of the map in reverse order
 */
    class ReverseIterator {
    public:
        ReverseIterator(const ReverseIterator& _riter);
        ReverseIterator(Node<K, M>* _node);
        Node<K, M>* get_cur() const;

        //defining operators for ReverseIterator
        ReverseIterator& operator=(const ReverseIterator& _riter);
        ReverseIterator& operator++();
        ReverseIterator operator++(int);
        ReverseIterator& operator--();
        ReverseIterator operator--(int);
        ValueType& operator*() const;
        ValueType* operator->() const;

        ~ReverseIterator();

    private:
        Node<K, M>* _cur;
    };

    map();

    map(const map& _map);

    map(InitializerList<pair<const K, M> > _l);

    //delete every node in the map
    ~map();

    Node<K, M>* get_head() const;
    Node<K, M>* get_tail() const;

    //constructs the map
    void map_ctor();
    //init parameters
    void init_head_tail();
    void init_assign_head_tail();
    void init_size();

    //reset parameters
    void reset_head_tail();
    void reset_size();

    //return Iterator pointing to the node which has key given by _key
    Iterator find(const K& _key);

    //return ConstIterator pointing to the node which has key given by _key
    ConstIterator find(const K& _key) const;

    /*
        begin() -- return iterator pointing to the key element in the map
        end() -- return iterator pointing to one past the last element in the map (logically)
        rbegin() -- works like begin() for ReverseIterator
        rend() -- works like end() for ReverseIterator
 */
    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;
    ReverseIterator rbegin();
    ReverseIterator rend();

    size_t size() const;
    size_t count() const;
    bool empty() const;

    //return mapped type for the given key
    M& at(const K& _key);
    //return const reference to mapped type for given key
    const M& at(const K& _key) const;

    //comparison operators on iterator
    template<typename K1, typename M1>
    friend bool operator==(const typename map<K1, M1>::Iterator& _iter1, const typename map<K1, M1>::Iterator& _iter2);
    template<typename K1, typename M1>
    friend bool operator!=(const typename map<K1, M1>::Iterator& _iter1, const typename map<K1, M1>::Iterator& _iter2);
    template<typename K1, typename M1>
    friend bool operator==(const typename map<K1, M1>::ConstIterator& _citer1, const typename map<K1, M1>::ConstIterator& _citer2);
    template<typename K1, typename M1>
    friend bool operator!=(const typename map<K1, M1>::ConstIterator& _citer1, const typename map<K1, M1>::ConstIterator& _citer2);
    template<typename K1, typename M1>
    friend bool operator==(const typename map<K1, M1>::ConstIterator& _citer, const typename map<K1, M1>::Iterator& _iter);
    template<typename K1, typename M1>
    friend bool operator!=(const typename map<K1, M1>::Iterator& _iter, const typename map<K1, M1>::ConstIterator& _citer);
    template<typename K1, typename M1>
    friend bool operator==(const typename map<K1, M1>::Iterator& _iter, const typename map<K1, M1>::ConstIterator& _citer);
    template<typename K1, typename M1>
    friend bool operator==(const typename map<K1, M1>::ConstIterator& _citer, const typename map<K1, M1>::Iterator& _iter);
    template<typename K1, typename M1>
    friend bool operator!=(const typename map<K1, M1>::Iterator& _iter, const typename map<K1, M1>::ConstIterator& _citer);
    template<typename K1, typename M1>
    friend bool operator!=(const typename map<K1, M1>::ConstIterator& _citer, const typename map<K1, M1>::Iterator& _iter);

    //clear all nodes in the map
    void clear();
    //operator[] for accessing mapped type for key
    M& operator[](const K&);
    const M& operator[](const K&) const;
    //level randomizer for insert
    size_t random_level(Node<K, M>**);
    //getting node for key using bottom link
    Node<K, M>* find_at_bottom(const K& _key) const;
    //insert ValueType into map and return iterator positioned at that node
    pair<Iterator, bool> insert(const ValueType&);
    //range based insert
    template <typename IT_T>
    void insert(IT_T range_beg, IT_T range_end);
    //remove node pointed to by iterator
    void erase(Iterator);
    //remove node which has given key
    void erase(const K&);
    //assignment of maps
    map& operator=(const map&);
    //map comparisons
    bool operator!=(const map&) const;
    template <typename Key, typename mapped>
    friend bool operator==(const map<Key, mapped>&, const map<Key, mapped>&);
    template <typename Key, typename mapped>
    friend bool operator<(const map<Key, mapped>&, const map<Key, mapped>&);
    template <typename Key, typename mapped>
    friend bool operator!=(const map<Key, mapped>&, const map<Key, mapped>&);

private:
    Node<K, M>* _head;
    Node<K, M>* _tail;
    size_t _max;
    size_t _size;
};

}

#include "impl_map.hpp"

#endif
