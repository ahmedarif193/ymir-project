#include "utils/map.h"

template <typename K, typename M>
M& lxcd::map<K, M>::operator[](const K& _key){
    lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
    if(_temp_head == NULL){
        lxcd::map<K, M>::Iterator _iter = insert(make_pair(_key, M())).first;
        return _iter.get_cur()->_value->second;
    }
    return _temp_head->_value->second;
}

template <typename K, typename M>
size_t lxcd::map<K, M>::random_level(Node<K, M>** _nodes){
    size_t _level = 0;
    while(_level < _LEVELS && rand() < RAND_MAX*0.5){
        ++_level;
    }
    if(_level > _max){
        size_t _temp = _max+1;
        while(_temp <= _level){
            _nodes[_temp] = _head;
            ++_temp;
        }
        _max = _level;
    }
    return _level;
}

template <typename K, typename M>
lxcd::map<K, M>& lxcd::map<K, M>::operator=(const lxcd::map<K, M>& _map){
    if(this == &_map){
        return *this;
    }
    lxcd::Node<K, M> *_temp_head = _head;
    lxcd::Node<K, M> *_temp;
    while(_temp_head != NULL){
        _temp = _temp_head->_forward_links[0];
        delete _temp_head;
        _temp_head = NULL;
        _temp_head = _temp;
    }

    map_ctor();
    lxcd::Node<K, M> *_first = _map.get_head()->_forward_links[0];
    if(_first == _map.get_tail()) return *this;
    while(_first != _map.get_tail()){
        insert(*(_first->_value));
        _first = _first->_forward_links[0];
    }
    return *this;
}

template <typename K, typename M>
lxcd::pair<typename lxcd::map<K, M>::Iterator, bool> lxcd::map<K, M>::insert(const lxcd::pair<const K, M>& _val){
    const K& _key = _val.first;
    lxcd::Node<K, M> *_temp = _head;
    lxcd::Node<K, M> **_updated = new lxcd::Node<K, M>* [_LEVELS+1];
    memset(_updated, '\0', sizeof(Node<K, M>*)*(_LEVELS+1));
    size_t i = _max;
    while(i >= 1){
        while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->first < _key){
            _temp = _temp->_forward_links[i];
        }
        _updated[i] = _temp;
        i--;
    }

    while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->first < _key){
        _temp = _temp->_forward_links[0];
    }
    _updated[0] = _temp;
    lxcd::Node<K, M> *_first_updated = _updated[0];
    _first_updated = _first_updated->_forward_links[0];
    if(_first_updated->_value != NULL && _first_updated->_value->first == _val.first){
        delete [] _updated;
        return make_pair(map<K, M>::Iterator(_first_updated), false);
    }

    size_t _level = random_level(_updated);
    _first_updated = NULL;
    _first_updated = new lxcd::Node<K, M>(_level, _val);
    i = 0;
    while(i <= _level){
        _first_updated->_forward_links[i] = _updated[i]->_forward_links[i];
        _updated[i]->_forward_links[i] = _first_updated;
        ++i;
    }
    _first_updated->_prev = _updated[0];
    if(_first_updated->_forward_links[0] != _tail){
        _first_updated->_forward_links[0]->_prev = _first_updated;
    }
    else{
        _tail->_prev = _first_updated;
    }
    ++_size;
    delete [] _updated;
    return make_pair(map<K, M>::Iterator(_first_updated), true);
}

template <typename K, typename M>
template <typename IT_T>
void lxcd::map<K, M>::insert(IT_T range_beg, IT_T range_end){
    auto _iter = range_beg;
    while(_iter != range_end){
        insert(*_iter);
        ++_iter;
    }
}

template <typename K, typename M>
void lxcd::map<K, M>::erase(const K& _key){
    lxcd::Node<K, M> *_temp = _head;
    lxcd::Node<K, M> **_updated = new lxcd::Node<K, M>* [_LEVELS+1];
    memset(_updated, '\0', sizeof(Node<K, M>*)*(_LEVELS+1));
    size_t i = _max;
    while(i >= 1){
        while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->first < _key){
            _temp = _temp->_forward_links[i];
        }
        _updated[i] = _temp;
        i--;
    }

    while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->first < _key){
        _temp = _temp->_forward_links[0];
    }
    _updated[0] = _temp;
    lxcd::Node<K, M> *_first_updated = _updated[0];
    _first_updated = _first_updated->_forward_links[0];
    if(_first_updated->_value->first == _key){
        i = 0;
        while(i <= _max && _updated[i]->_forward_links[i] == _first_updated){
            _updated[i]->_forward_links[i] = _first_updated->_forward_links[i];
            ++i;
        }

        if(_first_updated->_forward_links[0] != _tail){
            _first_updated->_forward_links[0]->_prev = _first_updated->_prev;
        }
        else{
            _tail->_prev = _first_updated->_prev;
            _first_updated->_prev->_forward_links[0] = _tail;
        }

        delete _first_updated;
        while(_max > 0 && _head->_forward_links[_max] == NULL){
            --_max;
        }
        --_size;
        delete [] _updated;
    }
    else{
        throw std::out_of_range("out of range");
    }
}

template <typename K, typename M>
void lxcd::map<K, M>::erase(map<K, M>::Iterator _iter){
    const K& _key = _iter.get_cur()->_value->first;
    lxcd::Node<K, M> *_temp = _head;
    lxcd::Node<K, M> **_updated = new lxcd::Node<K, M>* [_LEVELS+1];
    memset(_updated, '\0', sizeof(Node<K, M>*)*(_LEVELS+1));
    size_t i = _max;
    while(i >= 1){
        while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->first < _key){
            _temp = _temp->_forward_links[i];
        }
        _updated[i] = _temp;
        i--;
    }

    while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->first < _key){
        _temp = _temp->_forward_links[0];
    }
    _updated[0] = _temp;
    lxcd::Node<K, M> *_first_updated = _updated[0];
    _first_updated = _first_updated->_forward_links[0];
    if(_first_updated->_value->first == _key){
        i = 0;
        while(i <= _max && _updated[i]->_forward_links[i] == _first_updated){
            _updated[i]->_forward_links[i] = _first_updated->_forward_links[i];
            ++i;
        }

        if(_first_updated->_forward_links[0] != _tail){
            _first_updated->_forward_links[0]->_prev = _first_updated->_prev;
        }
        else{
            _tail->_prev = _first_updated->_prev;
            _first_updated->_prev->_forward_links[0] = _tail;
        }

        delete _first_updated;
        while(_max > 0 && _head->_forward_links[_max] == NULL){
            --_max;
        }
        --_size;
        delete [] _updated;
    }
    else{
        throw std::out_of_range("out of range");
    }
}

template <typename K, typename M>
void lxcd::map<K, M>::clear(){
    lxcd::Node<K, M> *_temp_head = _head;
    lxcd::Node<K, M> *_temp;
    while(_temp != NULL){
        _temp = _temp_head->_forward_links[0];
        delete _temp_head;
        _temp_head = _temp;
    }
    reset_size();
    reset_head_tail();
    map_ctor();
}

template <typename K, typename M>
lxcd::Node<K, M>* lxcd::map<K, M>::find_at_bottom(const K& _key) const{
    lxcd::Node<K, M> *_temp = _head;
    int i = _max;
    while(i >= 1){
        while(_temp->_forward_links[i] != NULL && _temp->_forward_links[i]->_value->first < _key){
            _temp = _temp->_forward_links[i];
        }
        i--;
    }

    while(_temp->_forward_links[0] != _tail && _temp->_forward_links[0]->_value->first < _key){
        _temp = _temp->_forward_links[0];
    }
    _temp = _temp->_forward_links[0];
    if(_temp == _tail) return NULL;
    if(_temp != NULL){
        if(_temp->_value->first == _key)
            return _temp;
    }
    return NULL;
}

template <typename K, typename M>
bool lxcd::map<K, M>::operator!=(const lxcd::map<K, M>& _map) const{
    if(*this == _map) return false;
    return true;
}

template <typename K, typename M>
bool operator==(const lxcd::map<K, M>& _map1, const lxcd::map<K, M>& _map2){
    if(_map1.size() != _map2.size()) return false;
    auto _iter1 = _map1.begin();
    auto _iter2 = _map2.begin();
    while(_iter1 != _map1.end() && _iter2 != _map2.end()){
        if(*_iter1 != *_iter2){
            return false;
        }
        ++_iter1;
        ++_iter2;
    }
    return true;
}

template <typename K, typename M>
bool operator!=(const lxcd::map<K, M>& _map1, const lxcd::map<K, M>& _map2){
    return !(_map1 == _map2);
}

template <typename K, typename M>
bool operator<(const lxcd::map<K, M>& _map1, const lxcd::map<K, M>& _map2){
    size_t _size1 = _map1.size();
    size_t _size2 = _map2.size();
    if(_size1 < _size2) return true;
    if(_size2 < _size1) return false;
    //same size
    auto _iter1 = _map1.begin();
    auto _iter2 = _map2.begin();
    while(_iter1 != _map1.end() && _iter2 != _map2.end()){
        bool _less = (*_iter1).first < (*_iter2).first;
        bool _less2 = (*_iter2).first < (*_iter1).first;
        if(_less) return true;
        if(_less2) return false;
        ++_iter1;
        ++_iter2;
    }
    //maps are same
    return false;
}

template<typename K, typename M>
lxcd::map<K, M>::map() {
    map_ctor();
}

template<typename K, typename M>
lxcd::map<K, M>::map(const lxcd::map<K, M>& _map) {
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
lxcd::map<K, M>::map(std::initializer_list<lxcd::pair<const K, M>> _l) {
    map_ctor();
    auto _iter = _l.begin();
    while (_iter != _l.end()) {
        insert(*_iter);
        _iter++;
    }
}

template<typename K, typename M>
lxcd::map<K, M>::~map(){
    lxcd::Node<K, M> *_temp_head = _head;
    lxcd::Node<K, M> *_temp;
    while(_temp_head != NULL){
        _temp = _temp_head->_forward_links[0];
        delete _temp_head;
        _temp_head = _temp;
    }
}

template<typename K, typename M>
lxcd::Node<K, M> *lxcd::map::get_head() const{
    return _head;
}

template<typename K, typename M>
lxcd::Node<K, M> *lxcd::map::get_tail() const{
    return _tail;
}

template<typename K, typename M>
void lxcd::map::map_ctor(){
    init_head_tail();
    init_assign_head_tail();
    init_size();
}

template<typename K, typename M>
void lxcd::map::init_head_tail(){
    _head = new lxcd::Node<K, M>(_LEVELS);
    _tail = new lxcd::Node<K, M>(_LEVELS);
}

template<typename K, typename M>
void lxcd::map::init_assign_head_tail(){
    _head->_forward_links[0] = _tail;
    _tail->_prev = _head;
    _head->_prev = NULL;
    _tail->_forward_links[0] = NULL;
}

template<typename K, typename M>
void lxcd::map::init_size(){
    _max = 0;
    _size = 0;
}

template<typename K, typename M>
void lxcd::map::reset_head_tail(){
    _head = NULL;
    _tail = NULL;
}

template<typename K, typename M>
void lxcd::map::reset_size(){
    _max = 0;
    _size = 0;
}

template<typename K, typename M>
lxcd::map::Iterator lxcd::map::find(const K &_key){
    lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
    if(_temp_head == NULL){
        return map<K, M>::Iterator(_tail);
    }
    return map<K, M>::Iterator(_temp_head);
}

template<typename K, typename M>
lxcd::map::ConstIterator lxcd::map::find(const K &_key) const{
    lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
    if(_temp_head == NULL){
        return map<K, M>::ConstIterator(_tail);
    }
    return map<K, M>::ConstIterator(_temp_head);
}

template<typename K, typename M>
lxcd::map::Iterator lxcd::map::begin(){return map<K, M>::Iterator(_head->_forward_links[0]);}

template<typename K, typename M>
lxcd::map::Iterator lxcd::map::end(){return map<K, M>::Iterator(_tail);}

template<typename K, typename M>
lxcd::map::ConstIterator lxcd::map::begin() const{return ConstIterator(_head->_forward_links[0]);}

template<typename K, typename M>
lxcd::map::ConstIterator lxcd::map::end() const{return ConstIterator(_tail);}

template<typename K, typename M>
lxcd::map::ReverseIterator lxcd::map::rbegin(){return ReverseIterator(_tail->_prev);}

template<typename K, typename M>
lxcd::map::ReverseIterator lxcd::map::rend(){return ReverseIterator(_head);}

template<typename K, typename M>
std::size_t lxcd::map::size() const{return _size;}

template<typename K, typename M>
std::size_t lxcd::map::count() const{return _size;}

template<typename K, typename M>
bool lxcd::map::empty() const{return (_size == 0)? true : false;}

template<typename K, typename M>
M &lxcd::map::at(const K &_key){
    lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
    if(_temp_head == NULL){
        throw std::out_of_range("out of range");
    }
    else return _temp_head->_value->second;

}

template<typename K, typename M>
const M &lxcd::map::at(const K &_key) const{
    lxcd::Node<K, M> *_temp_head = find_at_bottom(_key);
    if(_temp_head == NULL){
        throw std::out_of_range("out of range");
    }
    else return _temp_head->_value->second;
}

template<typename K, typename M>
bool lxcd::map::operator==(const lxcd::map::Iterator &_iter1, const lxcd::map::Iterator &_iter2){
    return (_iter1.get_cur() == _iter2.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator!=(const lxcd::map::Iterator &_iter1, const lxcd::map::Iterator &_iter2){
    return (_iter1.get_cur() != _iter2.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator==(const lxcd::map::ConstIterator &_citer1, const lxcd::map::ConstIterator &_citer2){
    return (_citer1.get_cur() == _citer2.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator!=(const lxcd::map::ConstIterator &_citer1, const lxcd::map::ConstIterator &_citer2){
    return (_citer1.get_cur() != _citer2.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator==(const lxcd::map::ReverseIterator &_riter1, const lxcd::map::ReverseIterator &_riter2){
    return (_riter1.get_cur() == _riter2.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator!=(const lxcd::map::ReverseIterator &_riter1, const lxcd::map::ReverseIterator &_riter2){
    return (_riter1.get_cur() != _riter2.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator==(const lxcd::map::Iterator &_iter, const lxcd::map::ConstIterator &_citer){
    return (_iter.get_cur() == _citer.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator==(const lxcd::map::ConstIterator &_citer, const lxcd::map::Iterator &_iter){
    return (_citer.get_cur() == _iter.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator!=(const lxcd::map::Iterator &_iter, const lxcd::map::ConstIterator &_citer){
    return (_iter.get_cur() != _citer.get_cur())? true : false;
}

template<typename K, typename M>
bool lxcd::map::operator!=(const lxcd::map::ConstIterator &_citer, const lxcd::map::Iterator &_iter){
    return (_citer.get_cur() != _iter.get_cur())? true : false;
}

template<typename K, typename M>
lxcd::map::ReverseIterator::ReverseIterator(const lxcd::map::ReverseIterator &_riter) : _cur(_riter.get_cur()){}

template<typename K, typename M>
lxcd::map::ReverseIterator::ReverseIterator(lxcd::Node<K, M> *_node) : _cur(_node){}

template<typename K, typename M>
lxcd::Node<K, M> *lxcd::map::ReverseIterator::get_cur() const{return _cur;}

template<typename K, typename M>
lxcd::map::ReverseIterator &lxcd::map::ReverseIterator::operator=(const lxcd::map::ReverseIterator &_riter){
    _cur = _riter.get_cur();
    return *this;
}

template<typename K, typename M>
lxcd::map::ReverseIterator &lxcd::map::ReverseIterator::operator++(){
    if(_cur == NULL) return *this;
    _cur = _cur->_prev;
    return *this;
}

template<typename K, typename M>
lxcd::map::ReverseIterator lxcd::map::ReverseIterator::operator++(int){
    map<K, M>::ReverseIterator _this = *this;
    if(_cur == NULL) return _this;
    _cur = _cur->_prev;
    return _this;
}

template<typename K, typename M>
lxcd::map::ReverseIterator &lxcd::map::ReverseIterator::operator--(){
    if(_cur == NULL) return *this;
    _cur = _cur->_forward_links[0];
    return *this;
}

template<typename K, typename M>
lxcd::map::ReverseIterator lxcd::map::ReverseIterator::operator--(int){
    map<K, M>::ReverseIterator _this = *this;
    if(_cur == NULL) return _this;
    _cur = _cur->_prev;
    return _this;
}

template<typename K, typename M>
lxcd::map::ValueType &lxcd::map::ReverseIterator::operator*() const{
    return *_cur->_value;
}

template<typename K, typename M>
lxcd::map::ValueType *lxcd::map::ReverseIterator::operator->() const{
    return _cur->_value;
}

template<typename K, typename M>
lxcd::map::ReverseIterator::~ReverseIterator(){
    _cur = nullptr;
}

template<typename K, typename M>
lxcd::map::ConstIterator::ConstIterator(const lxcd::map::ConstIterator &_citer) : _cur(_citer.get_cur()){}

template<typename K, typename M>
lxcd::map::ConstIterator::ConstIterator(const lxcd::map::Iterator &_iter) : _cur(_iter.get_cur()){}

template<typename K, typename M>
lxcd::map::ConstIterator::ConstIterator(lxcd::Node<K, M> *_node) : _cur(_node){}

template<typename K, typename M>
lxcd::Node<K, M> *lxcd::map::ConstIterator::get_cur() const{return _cur;}

template<typename K, typename M>
lxcd::map::ConstIterator &lxcd::map::ConstIterator::operator=(const lxcd::map::ConstIterator &_citer){
    _cur = _citer.get_cur();
    return *this;
}

template<typename K, typename M>
lxcd::map::ConstIterator &lxcd::map::ConstIterator::operator++(){
    if(_cur == NULL) return *this;
    _cur = _cur->_forward_links[0];
    return *this;
}

template<typename K, typename M>
lxcd::map::ConstIterator lxcd::map::ConstIterator::operator++(int){
    map<K, M>::ConstIterator _this = *this;
    if(_cur == NULL) return _this;
    _cur = _cur->_forward_links[0];
    return _this;
}

template<typename K, typename M>
lxcd::map::ConstIterator &lxcd::map::ConstIterator::operator--(){
    if(_cur == NULL) return *this;
    _cur = _cur->_prev;
    return *this;
}

template<typename K, typename M>
lxcd::map::ConstIterator lxcd::map::ConstIterator::operator--(int){
    map<K, M>::ConstIterator _this = *this;
    if(_cur == NULL) return _this;
    _cur = _cur->_prev;
    return _this;
}

template<typename K, typename M>
const lxcd::map::ValueType &lxcd::map::ConstIterator::operator*() const{
    return *_cur->_value;
}

template<typename K, typename M>
const lxcd::map::ValueType *lxcd::map::ConstIterator::operator->() const{
    return _cur->_value;
}

template<typename K, typename M>
lxcd::map::ConstIterator::~ConstIterator(){
    _cur = nullptr;
}

template<typename K, typename M>
lxcd::map::Iterator::Iterator(const lxcd::map::Iterator &_iter) : _cur(_iter.get_cur()){}

template<typename K, typename M>
lxcd::map::Iterator::Iterator(lxcd::Node<K, M> *_node) : _cur(_node){}

template<typename K, typename M>
lxcd::Node<K, M> *lxcd::map::Iterator::get_cur() const{return _cur;}

template<typename K, typename M>
lxcd::map::Iterator &lxcd::map::Iterator::operator=(const lxcd::map::Iterator &_iter){
    _cur = _iter.get_cur();
    return *this;
}

template<typename K, typename M>
lxcd::map::Iterator &lxcd::map::Iterator::operator++(){
    if(_cur == NULL) return *this;
    _cur = _cur->_forward_links[0];
    return *this;
}

template<typename K, typename M>
lxcd::map::Iterator lxcd::map::Iterator::operator++(int){
    map<K, M>::Iterator _this = *this;
    if(_cur == NULL) return _this;
    _cur = _cur->_forward_links[0];
    return _this;
}

template<typename K, typename M>
lxcd::map::Iterator &lxcd::map::Iterator::operator--(){
    if(_cur == NULL) return *this;
    _cur = _cur->_prev;
    return *this;
}

template<typename K, typename M>
lxcd::map::Iterator lxcd::map::Iterator::operator--(int){
    map<K, M>::Iterator _this = *this;
    if(_cur == NULL) return _this;
    _cur = _cur->_prev;
    return _this;
}

template<typename K, typename M>
lxcd::map::ValueType &lxcd::map::Iterator::operator*() const{
    return *_cur->_value;
}

template<typename K, typename M>
lxcd::map::ValueType *lxcd::map::Iterator::operator->() const{
    return _cur->_value;
}

template<typename K, typename M>
lxcd::map::Iterator::~Iterator(){
    _cur = nullptr;
}

template<typename K, typename M>
lxcd::Node::Node(std::size_t _level){
    int _temp_level = _level+1;
    _forward_links = new Node*[_temp_level];
    size_t _total_size = sizeof(Node*)*(_temp_level);
    memset(_forward_links, '\0', _total_size);
    _value = NULL;
    _prev = NULL;
}

template<typename K, typename M>
lxcd::Node::Node(std::size_t _level, const lxcd::Node::ValueType &_val){
    int _temp_level = _level+1;
    _forward_links = new Node*[_temp_level];
    size_t _total_size = sizeof(Node*)*(_temp_level);
    memset(_forward_links, '\0', _total_size);
    _value = new lxcd::pair<const K, M>(_val);
    _prev = NULL;
}

template<typename K, typename M>
lxcd::Node::~Node(){
    delete [] _forward_links;
    delete _value;
}
