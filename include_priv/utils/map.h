#ifndef MAP_H
#define MAP_H

#include "vector.h"
#include "string.h"
#include <cstdlib>

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
bool operator==(const Iterator& other) const {
	return _cur == other._cur;
}
bool operator!=(const Iterator &other) const {
	return _cur != other._cur;
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
// Constructor using a node pointer
ConstIterator(Node<K, M>* _node);

// Copy constructor using another ConstIterator
ConstIterator(const ConstIterator &_citer);

// Constructor using an Iterator
ConstIterator(const Iterator &_iter);

// Getter for the current node
Node<K, M>* get_cur() const;

// Assignment operator
ConstIterator& operator=(const ConstIterator &_citer);

bool operator!=(const ConstIterator& other) const {
	return _cur != other._cur;
}
// Prefix increment operator
ConstIterator& operator++();

// Postfix increment operator
ConstIterator operator++(int);

// Prefix decrement operator
ConstIterator& operator--();

// Postfix decrement operator
ConstIterator operator--(int);

// Dereference operator
const ValueType& operator*() const;

// Arrow operator
const ValueType* operator->() const;

// Destructor
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
size_t count(const K& key) const {
	Node<K, M>* node = find_at_bottom(key);
	return (node != nullptr && node->_value != nullptr && node->_value->key == key) ? 1 : 0;
}

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

template <typename K, typename M>
inline M& lxcd::map<K, M>::operator[](const K& _key){
	lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
	if(_temp_head == NULL) {
		lxcd::map<K, M>::Iterator _iter = insert(lxcd::make_pair(_key, M())).key;
		return _iter.get_cur()->_value->value;
	}
	return _temp_head->_value->value;
}
template <typename K, typename M>
const M& lxcd::map<K, M>::operator[](const K& _key) const {
	const lxcd::Node<K, M>* _temp_head = find_at_bottom(_key);
	if (_temp_head == NULL) {
		// If key is not found, return a default-constructed value.
		static const M default_value;
		return default_value;
	}
	return _temp_head->_value->value;
}
template <typename K, typename M>
inline lxcd::size_t lxcd::map<K, M>::random_level(lxcd::Node<K, M>** _nodes){
	size_t _level = 0;
	while(_level < _LEVELS && rand() < RAND_MAX*0.5) {
		++_level;
	}
	if(_level > _max) {
		size_t _temp = _max+1;
		while(_temp <= _level) {
			_nodes[_temp] = _head;
			++_temp;
		}
		_max = _level;
	}
	return _level;
}

template <typename K, typename M>
inline lxcd::map<K, M>& lxcd::map<K, M>::operator=(const lxcd::map<K, M>& _map){
	if(this == &_map) {
		return *this;
	}
	lxcd::Node<K, M> *_temp_head = _head;
	lxcd::Node<K, M> *_temp;
	while(_temp_head != NULL) {
		_temp = _temp_head->_forward_links[0];
		delete _temp_head;
		_temp_head = NULL;
		_temp_head = _temp;
	}

	map_ctor();
	lxcd::Node<K, M> *_key = _map.get_head()->_forward_links[0];
	if(_key == _map.get_tail()) return *this;
	while(_key != _map.get_tail()) {
		insert(*(_key->_value));
		_key = _key->_forward_links[0];
	}
	return *this;
}

template <typename K, typename M>
inline lxcd::pair<typename lxcd::map<K, M>::Iterator, bool> lxcd::map<K, M>::insert(const lxcd::pair<const K, M>& _val){
	const K& _key = _val.key;
	lxcd::Node<K, M> *_temp = _head;
	lxcd::Node<K, M> **_updated = new lxcd::Node<K, M>* [_LEVELS+1];
	memset(_updated, '\0', sizeof(Node<K, M>*)*(_LEVELS+1));
	size_t i = _max;
	while(i >= 1) {
		while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->key < _key) {
			_temp = _temp->_forward_links[i];
		}
		_updated[i] = _temp;
		i--;
	}

	while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->key < _key) {
		_temp = _temp->_forward_links[0];
	}
	_updated[0] = _temp;
	lxcd::Node<K, M> *_key_updated = _updated[0];
	_key_updated = _key_updated->_forward_links[0];
	if(_key_updated->_value != NULL && _key_updated->_value->key == _val.key) {
		delete [] _updated;
		return lxcd::make_pair(map<K, M>::Iterator(_key_updated), false);
	}

	size_t _level = random_level(_updated);
	_key_updated = NULL;
	_key_updated = new lxcd::Node<K, M>(_level, _val);
	i = 0;
	while(i <= _level) {
		_key_updated->_forward_links[i] = _updated[i]->_forward_links[i];
		_updated[i]->_forward_links[i] = _key_updated;
		++i;
	}
	_key_updated->_prev = _updated[0];
	if(_key_updated->_forward_links[0] != _tail) {
		_key_updated->_forward_links[0]->_prev = _key_updated;
	}
	else{
		_tail->_prev = _key_updated;
	}
	++_size;
	delete [] _updated;
	return lxcd::make_pair(map<K, M>::Iterator(_key_updated), true);
}

template <typename K, typename M>
template <typename IT_T>
inline void lxcd::map<K, M>::insert(IT_T range_beg, IT_T range_end){
	auto _iter = range_beg;
	while(_iter != range_end) {
		insert(*_iter);
		++_iter;
	}
}

template <typename K, typename M>
inline void lxcd::map<K, M>::erase(const K& _key){
	lxcd::Node<K, M> *_temp = _head;
	lxcd::Node<K, M> **_updated = new lxcd::Node<K, M>* [_LEVELS+1];
	memset(_updated, '\0', sizeof(Node<K, M>*)*(_LEVELS+1));
	size_t i = _max;
	while(i >= 1) {
		while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->key < _key) {
			_temp = _temp->_forward_links[i];
		}
		_updated[i] = _temp;
		i--;
	}

	while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->key < _key) {
		_temp = _temp->_forward_links[0];
	}
	_updated[0] = _temp;
	lxcd::Node<K, M> *_key_updated = _updated[0];
	_key_updated = _key_updated->_forward_links[0];
	if(_key_updated->_value->key == _key) {
		i = 0;
		while(i <= _max && _updated[i]->_forward_links[i] == _key_updated) {
			_updated[i]->_forward_links[i] = _key_updated->_forward_links[i];
			++i;
		}

		if(_key_updated->_forward_links[0] != _tail) {
			_key_updated->_forward_links[0]->_prev = _key_updated->_prev;
		}
		else{
			_tail->_prev = _key_updated->_prev;
			_key_updated->_prev->_forward_links[0] = _tail;
		}

		delete _key_updated;
		while(_max > 0 && _head->_forward_links[_max] == NULL) {
			--_max;
		}
		--_size;
		delete [] _updated;
	}
	else{
		//TODO change this to return : throw lxcd::OutOfRangeException("out of range");
	}
}

template <typename K, typename M>
inline void lxcd::map<K, M>::erase(map<K, M>::Iterator _iter){
	const K& _key = _iter.get_cur()->_value->key;
	lxcd::Node<K, M> *_temp = _head;
	lxcd::Node<K, M> **_updated = new lxcd::Node<K, M>* [_LEVELS+1];
	memset(_updated, '\0', sizeof(Node<K, M>*)*(_LEVELS+1));
	size_t i = _max;
	while(i >= 1) {
		while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->key < _key) {
			_temp = _temp->_forward_links[i];
		}
		_updated[i] = _temp;
		i--;
	}

	while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->key < _key) {
		_temp = _temp->_forward_links[0];
	}
	_updated[0] = _temp;
	lxcd::Node<K, M> *_key_updated = _updated[0];
	_key_updated = _key_updated->_forward_links[0];
	if(_key_updated->_value->key == _key) {
		i = 0;
		while(i <= _max && _updated[i]->_forward_links[i] == _key_updated) {
			_updated[i]->_forward_links[i] = _key_updated->_forward_links[i];
			++i;
		}

		if(_key_updated->_forward_links[0] != _tail) {
			_key_updated->_forward_links[0]->_prev = _key_updated->_prev;
		}
		else{
			_tail->_prev = _key_updated->_prev;
			_key_updated->_prev->_forward_links[0] = _tail;
		}

		delete _key_updated;
		while(_max > 0 && _head->_forward_links[_max] == NULL) {
			--_max;
		}
		--_size;
		delete [] _updated;
	}
	else{
		//TODO change this to return : throw lxcd::OutOfRangeException("out of range");
	}
}

template <typename K, typename M>
inline void lxcd::map<K, M>::clear(){
	lxcd::Node<K, M> *_temp_head = _head;
	lxcd::Node<K, M> *_temp;
	while(_temp != NULL) {
		_temp = _temp_head->_forward_links[0];
		delete _temp_head;
		_temp_head = _temp;
	}
	reset_size();
	reset_head_tail();
	map_ctor();
}

template <typename K, typename M>
inline lxcd::Node<K, M>* lxcd::map<K, M>::find_at_bottom(const K& _key) const {
	lxcd::Node<K, M> *_temp = _head;
	int i = _max;
	while(i >= 1) {
		while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->key < _key) {
			_temp = _temp->_forward_links[i];
		}
		i--;
	}

	while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->key < _key) {
		_temp = _temp->_forward_links[0];
	}
	_temp = _temp->_forward_links[0];
	if(_temp == _tail) return NULL;
	if(_temp != NULL) {
		if(_temp->_value->key == _key)
			return _temp;
	}
	return NULL;
}

template <typename K, typename M>
inline bool lxcd::map<K, M>::operator!=(const lxcd::map<K, M>& _map) const {
	if(*this == _map) return false;
	return true;
}

template <typename K, typename M>
inline bool operator==(const lxcd::map<K, M>& _map1, const lxcd::map<K, M>& _map2){
	if(_map1.size() != _map2.size()) return false;
	auto _iter1 = _map1.begin();
	auto _iter2 = _map2.begin();
	while(_iter1 != _map1.end() && _iter2 != _map2.end()) {
		if(*_iter1 != *_iter2) {
			return false;
		}
		++_iter1;
		++_iter2;
	}
	return true;
}

template <typename K, typename M>
inline bool operator!=(const lxcd::map<K, M>& _map1, const lxcd::map<K, M>& _map2){
	return !(_map1 == _map2);
}

template <typename K, typename M>
inline bool operator<(const lxcd::map<K, M>& _map1, const lxcd::map<K, M>& _map2){
	size_t _size1 = _map1.size();
	size_t _size2 = _map2.size();
	if(_size1 < _size2) return true;
	if(_size2 < _size1) return false;
	//same size
	auto _iter1 = _map1.begin();
	auto _iter2 = _map2.begin();
	while(_iter1 != _map1.end() && _iter2 != _map2.end()) {
		bool _less = (*_iter1).key < (*_iter2).key;
		bool _less2 = (*_iter2).key < (*_iter1).key;
		if(_less) return true;
		if(_less2) return false;
		++_iter1;
		++_iter2;
	}
	//maps are same
	return false;
}

template<typename K, typename M>
inline lxcd::map<K, M>::map() {
	map_ctor();
}

template<typename K, typename M>
inline lxcd::map<K, M>::map(const lxcd::map<K, M>& _map) {
	map_ctor();
	lxcd::Node<K, M> *_temp = _map.get_head()->_forward_links[0];
	while (_temp != _map.get_tail()) {
		ValueType& _val = *(_temp->_value);
		insert(_val);
		_temp = _temp->_forward_links[0];
	}
}

// Constructor definition with initializer_list

template<typename K, typename M>
inline lxcd::map<K, M>::map(InitializerList<lxcd::pair<const K, M> > _l) {
	map_ctor();
	auto _iter = _l.begin();
	while (_iter != _l.end()) {
		insert(*_iter);
		_iter++;
	}
}

template<typename K, typename M>
inline lxcd::map<K, M>::~map(){
	lxcd::Node<K, M> *_temp_head = _head;
	lxcd::Node<K, M> *_temp;
	while(_temp_head != NULL) {
		_temp = _temp_head->_forward_links[0];
		delete _temp_head;
		_temp_head = _temp;
	}
}

template<typename K, typename M>
inline lxcd::Node<K, M> *lxcd::map<K, M>::get_head() const {
	return _head;
}

template<typename K, typename M>
inline lxcd::Node<K, M> *lxcd::map<K, M>::get_tail() const {
	return _tail;
}

template<typename K, typename M>
inline void lxcd::map<K, M>::map_ctor(){
	init_head_tail();
	init_assign_head_tail();
	init_size();
}

template<typename K, typename M>
inline void lxcd::map<K, M>::init_head_tail(){
	_head = new lxcd::Node<K, M>(_LEVELS);
	_tail = new lxcd::Node<K, M>(_LEVELS);
}

template<typename K, typename M>
inline void lxcd::map<K, M>::init_assign_head_tail(){
	_head->_forward_links[0] = _tail;
	_tail->_prev = _head;
	_head->_prev = NULL;
	_tail->_forward_links[0] = NULL;
}

template<typename K, typename M>
inline void lxcd::map<K, M>::init_size(){
	_max = 0;
	_size = 0;
}

template<typename K, typename M>
inline void lxcd::map<K, M>::reset_head_tail(){
	_head = NULL;
	_tail = NULL;
}

template<typename K, typename M>
inline void lxcd::map<K, M>::reset_size(){
	_max = 0;
	_size = 0;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator lxcd::map<K, M>::find(const K &_key) {
	lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
	if(_temp_head == NULL) {
		return map<K, M>::Iterator(_tail);
	}
	return map<K, M>::Iterator(_temp_head);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator lxcd::map<K, M>::find(const K &_key) const {
	lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
	if(_temp_head == NULL) {
		return map<K, M>::ConstIterator(_tail);
	}
	return map<K, M>::ConstIterator(_temp_head);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator lxcd::map<K, M>::begin(){
	return map<K, M>::Iterator(_head->_forward_links[0]);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator lxcd::map<K, M>::end(){
	return map<K, M>::Iterator(_tail);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator lxcd::map<K, M>::begin() const {
	return ConstIterator(_head->_forward_links[0]);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator lxcd::map<K, M>::end() const {
	return ConstIterator(_tail);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator lxcd::map<K, M>::rbegin(){
	return ReverseIterator(_tail->_prev);
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator lxcd::map<K, M>::rend(){
	return ReverseIterator(_head);
}

template<typename K, typename M>
inline lxcd::size_t lxcd::map<K, M>::size() const {
	return _size;
}

template<typename K, typename M>
inline lxcd::size_t lxcd::map<K, M>::count() const {
	return _size;
}

template<typename K, typename M>
inline bool lxcd::map<K, M>::empty() const {
	return (_size == 0) ? true : false;
}

template<typename K, typename M>
inline M &lxcd::map<K, M>::at(const K &_key){
	lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
	if(_temp_head == NULL) {
		//TODO change this to return : throw lxcd::OutOfRangeException("out of range");
	}
	else return _temp_head->_value->value;

}

template<typename K, typename M>
inline const M &lxcd::map<K, M>::at(const K &_key) const {
	lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
	if(_temp_head == NULL) {
		//TODO change this to return : throw lxcd::OutOfRangeException("out of range");
	}
	return _temp_head->_value->value;
}

template<typename K, typename M>
inline bool operator==(const typename lxcd::map<K, M>::Iterator &_iter1, const typename lxcd::map<K, M>::Iterator &_iter2){
	return (_iter1.get_cur() == _iter2.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator!=(const typename lxcd::map<K, M>::Iterator &_iter1, const typename lxcd::map<K, M>::Iterator &_iter2){
	return (_iter1.get_cur() != _iter2.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator==(const typename lxcd::map<K, M>::ConstIterator &_citer1, const typename lxcd::map<K, M>::ConstIterator &_citer2){
	return (_citer1.get_cur() == _citer2.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator!=(const typename lxcd::map<K, M>::ConstIterator &_citer1, const typename lxcd::map<K, M>::ConstIterator &_citer2){
	return (_citer1.get_cur() != _citer2.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator==(const typename lxcd::map<K, M>::ReverseIterator &_riter1, const typename lxcd::map<K, M>::ReverseIterator &_riter2){
	return (_riter1.get_cur() == _riter2.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator!=(const typename lxcd::map<K, M>::ReverseIterator &_riter1, const typename lxcd::map<K, M>::ReverseIterator &_riter2){
	return (_riter1.get_cur() != _riter2.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator==(const typename lxcd::map<K, M>::Iterator &_iter, const typename lxcd::map<K, M>::ConstIterator &_citer){
	return (_iter.get_cur() == _citer.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator==(const typename lxcd::map<K, M>::ConstIterator &_citer, const typename lxcd::map<K, M>::Iterator &_iter){
	return (_citer.get_cur() == _iter.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator!=(const typename lxcd::map<K, M>::Iterator &_iter, const typename lxcd::map<K, M>::ConstIterator &_citer){
	return (_iter.get_cur() != _citer.get_cur()) ? true : false;
}

template<typename K, typename M>
inline bool operator!=(const typename lxcd::map<K, M>::ConstIterator& _citer, const typename lxcd::map<K, M>::Iterator& _iter){
	return (_citer.get_cur() != _iter.get_cur()) ? true : false;
}

template<typename K, typename M>
inline lxcd::map<K, M>::ReverseIterator::ReverseIterator(const lxcd::map<K, M>::ReverseIterator &_riter) : _cur(_riter.get_cur()){
}

template<typename K, typename M>
inline lxcd::map<K, M>::ReverseIterator::ReverseIterator(lxcd::Node<K, M> *_node) : _cur(_node){
}

template<typename K, typename M>
inline lxcd::Node<K, M> *lxcd::map<K, M>::ReverseIterator::get_cur() const {
	return _cur;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator &lxcd::map<K, M>::ReverseIterator::operator=(const lxcd::map<K, M>::ReverseIterator &_riter){
	_cur = _riter.get_cur();
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator &lxcd::map<K, M>::ReverseIterator::operator++(){
	if(_cur == NULL) return *this;
	_cur = _cur->_prev;
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator lxcd::map<K, M>::ReverseIterator::operator++(int){
	map<K, M>::ReverseIterator _this = *this;
	if(_cur == NULL) return _this;
	_cur = _cur->_prev;
	return _this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator &lxcd::map<K, M>::ReverseIterator::operator--(){
	if(_cur == NULL) return *this;
	_cur = _cur->_forward_links[0];
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ReverseIterator lxcd::map<K, M>::ReverseIterator::operator--(int){
	map<K, M>::ReverseIterator _this = *this;
	if(_cur == NULL) return _this;
	_cur = _cur->_prev;
	return _this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ValueType &lxcd::map<K, M>::ReverseIterator::operator*() const {
	return *_cur->_value;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ValueType *lxcd::map<K, M>::ReverseIterator::operator->() const {
	return _cur->_value;
}

template<typename K, typename M>
inline lxcd::map<K, M>::ReverseIterator::~ReverseIterator(){
	_cur = nullptr;
}

template<typename K, typename M>
inline lxcd::map<K, M>::ConstIterator::ConstIterator(const lxcd::map<K, M>::ConstIterator &_citer) : _cur(_citer.get_cur()){
}

template<typename K, typename M>
inline lxcd::map<K, M>::ConstIterator::ConstIterator(const lxcd::map<K, M>::Iterator &_iter) : _cur(_iter.get_cur()){
}

template<typename K, typename M>
inline lxcd::map<K, M>::ConstIterator::ConstIterator(lxcd::Node<K, M> *_node) : _cur(_node){
}

template<typename K, typename M>
inline lxcd::Node<K, M> *lxcd::map<K, M>::ConstIterator::get_cur() const {
	return _cur;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator &lxcd::map<K, M>::ConstIterator::operator=(const lxcd::map<K, M>::ConstIterator &_citer){
	_cur = _citer.get_cur();
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator &lxcd::map<K, M>::ConstIterator::operator++(){
	if(_cur == NULL) return *this;
	_cur = _cur->_forward_links[0];
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator lxcd::map<K, M>::ConstIterator::operator++(int){
	map<K, M>::ConstIterator _this = *this;
	if(_cur == NULL) return _this;
	_cur = _cur->_forward_links[0];
	return _this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator &lxcd::map<K, M>::ConstIterator::operator--(){
	if(_cur == NULL) return *this;
	_cur = _cur->_prev;
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ConstIterator lxcd::map<K, M>::ConstIterator::operator--(int){
	map<K, M>::ConstIterator _this = *this;
	if(_cur == NULL) return _this;
	_cur = _cur->_prev;
	return _this;
}

template<typename K, typename M>
inline const typename lxcd::map<K, M>::ValueType &lxcd::map<K, M>::ConstIterator::operator*() const {
	return *_cur->_value;
}

template<typename K, typename M>
inline const typename lxcd::map<K, M>::ValueType *lxcd::map<K, M>::ConstIterator::operator->() const {
	return _cur->_value;
}

template<typename K, typename M>
inline lxcd::map<K, M>::ConstIterator::~ConstIterator(){
	_cur = nullptr;
}

template<typename K, typename M>
inline lxcd::map<K, M>::Iterator::Iterator(const lxcd::map<K, M>::Iterator &_iter) : _cur(_iter.get_cur()){
}

template<typename K, typename M>
inline lxcd::map<K, M>::Iterator::Iterator(lxcd::Node<K, M> *_node) : _cur(_node){
}

template<typename K, typename M>
inline lxcd::Node<K, M> *lxcd::map<K, M>::Iterator::get_cur() const {
	return _cur;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator &lxcd::map<K, M>::Iterator::operator=(const lxcd::map<K, M>::Iterator &_iter){
	_cur = _iter.get_cur();
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator &lxcd::map<K, M>::Iterator::operator++(){
	if(_cur == NULL) return *this;
	_cur = _cur->_forward_links[0];
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator lxcd::map<K, M>::Iterator::operator++(int){
	map<K, M>::Iterator _this = *this;
	if(_cur == NULL) return _this;
	_cur = _cur->_forward_links[0];
	return _this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator &lxcd::map<K, M>::Iterator::operator--(){
	if(_cur == NULL) return *this;
	_cur = _cur->_prev;
	return *this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::Iterator lxcd::map<K, M>::Iterator::operator--(int){
	map<K, M>::Iterator _this = *this;
	if(_cur == NULL) return _this;
	_cur = _cur->_prev;
	return _this;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ValueType &lxcd::map<K, M>::Iterator::operator*() const {
	return *_cur->_value;
}

template<typename K, typename M>
inline typename lxcd::map<K, M>::ValueType *lxcd::map<K, M>::Iterator::operator->() const {
	return _cur->_value;
}

template<typename K, typename M>
inline lxcd::map<K, M>::Iterator::~Iterator(){
	_cur = nullptr;
}

template<typename K, typename M>
inline lxcd::Node<K, M>::Node(size_t _level){
	int _temp_level = _level+1;
	_forward_links = new Node*[_temp_level];
	size_t _total_size = sizeof(Node*)*(_temp_level);
	memset(_forward_links, '\0', _total_size);
	_value = NULL;
	_prev = NULL;
}

template<typename K, typename M>
inline lxcd::Node<K, M>::Node(size_t _level, const lxcd::Node<K, M>::ValueType& _val){
	int _temp_level = _level+1;
	_forward_links = new Node*[_temp_level];
	size_t _total_size = sizeof(Node*)*(_temp_level);
	memset(_forward_links, '\0', _total_size);
	_value = new lxcd::pair<const K, M>(_val);
	_prev = NULL;
}

template<typename K, typename M>
inline lxcd::Node<K, M>::~Node() {
	delete [] _forward_links;
	delete _value;
}

}


#endif
