#pragma once
#include <stdint.h>
#include <string.h>
#include <type_traits>
#include <string>



template<typename TKey, typename TValue>
class hashtable_base;


class hashtable_node_value_const_char_pointer {
public:
	const char* _value;
	hashtable_node_value_const_char_pointer() :
		_value(nullptr)
	{}
	~hashtable_node_value_const_char_pointer();
	operator const char* &();
	void operator=(const char* str);
	uint32_t length();
};

template<typename TValue>
class hashtable_node_value {
public:
	using TypeValue = TValue;

	TValue _value;
	hashtable_node_value() 
	{}
	hashtable_node_value(TValue value) :
		_value(value)
	{}
};

template<>
class hashtable_node_value<const char*>:public hashtable_node_value_const_char_pointer {
public:
	using TypeValue = hashtable_node_value_const_char_pointer;

	uint32_t _len;
	char* _allocate2;
	hashtable_node_value() :
		_len(0),
		_allocate2(nullptr)
	{}
	hashtable_node_value(const char* value) :
		_len(0),
		_allocate2(nullptr)
	{
		operator=(value);
	}
	~hashtable_node_value() {
		if (_allocate2 != nullptr) {
			free(_allocate2);
			_allocate2 = nullptr;
			_value = nullptr;
			_len = 0;
		}
	}
	operator const char* &() {
		return _value;
	}
	void operator=(const char* str) {
		if (str==nullptr || _value == str) {
			return;
		}
		if (_allocate2 != nullptr) {
			free(_allocate2);
			_allocate2 = nullptr;
			_value = nullptr;
			_len = 0;
		}
		_len = strlen(str);
		_allocate2 = (char*)malloc(_len+1);
		memcpy(_allocate2, str, _len);
		_allocate2[_len] = '\0';
		_value = _allocate2;
	}
	uint32_t length() {
		return _len;
	}
	
};

hashtable_node_value_const_char_pointer::~hashtable_node_value_const_char_pointer() {
}
hashtable_node_value_const_char_pointer::operator const char* &() {
	return (*(hashtable_node_value<const char*>*)this).operator const char *&();
}
void hashtable_node_value_const_char_pointer::operator=(const char* str) {
	(*(hashtable_node_value<const char*>*)this).operator=(str);
}
uint32_t hashtable_node_value_const_char_pointer::length() {
	return (*(hashtable_node_value<const char*>*)this)._len;
}

template<>
class hashtable_node_value<void> {
public:
	using TypeValue =void;
};

template<typename TKey>
class hashtable_node_key {
public:
	using h_int = uint32_t;

	const TKey _key;
	const h_int _hash;
	hashtable_node_key(TKey key, uint32_t l, h_int hash) :
		_key(key),
		_hash(hash)
	{}
	static constexpr uint32_t  fixkeysize(uint32_t size) {
		return 0;
	}
	template<typename TKey = std::string>
	bool comp(const std::string& str, uint32_t count) {
		return (_key.length() == str.length() && str.compare(_key.c_str()) == 0);
	}
	template<typename TKey>
	bool comp(const TKey& key, uint32_t count) {
		return memcmp(&_key, &key, sizeof(TKey)) == 0;
	}
};



template<>
class hashtable_node_key<const char*> {
public:
	using h_int = uint32_t;

	const char* const _key;
	const uint32_t _keylen;
	const h_int _hash;

	hashtable_node_key(const char* key, uint32_t l, h_int hash) :
		_key(key),
		_keylen(l),
		_hash(hash)
	{
		// 不能在构析函数基础类引用并使用继承类变量,初始化顺序不同。
		// _key 使用继承类 _allocate,按内存顺序放置于最后。
		/*
		if (key != nullptr) {
			memcpy((void*)_key, key, l);
			((char*)_key)[l] = '\0';
			*(uint32_t*)(&_keylen) = l;
		}
		*/
	}
	static constexpr uint32_t  fixkeysize(uint32_t size) {
		return size;
	}
	bool comp(const char* str, uint32_t count) {
		return _key == str || (_keylen == count && strcmp(_key, str) == 0);
	}
};


template<class Table, class TNode,bool>
class hashtable_node_header  {
public:
	TNode* _next;
	hashtable_node_header(Table* parent = nullptr, TNode* next=nullptr): _next(next){
	}
};

template<class Table, class TNode>
class hashtable_node_header<Table, TNode, true> {
public:

	Table* _parent;
	TNode* _next;
	hashtable_node_header(Table* parent = nullptr, TNode* next = nullptr):_parent(parent),_next(next){
	}
};

template<class Table, class TNode, bool UseParent, bool>
class hashtable_node_base :public hashtable_node_header<Table, TNode, UseParent> {
public:
	using NodeHeader = hashtable_node_header<Table, TNode, UseParent>;
	hashtable_node_base(Table* parent = nullptr, TNode* next = nullptr) :NodeHeader(parent, next) {
	}
};
template<class Table, class TNode, bool UseParent>
class hashtable_node_base<Table, TNode, UseParent,true>:public hashtable_node_header<Table, TNode, UseParent> {
public:
	using NodeHeader = hashtable_node_header<Table, TNode, UseParent>;

	char _allocate[1];
	hashtable_node_base(Table* parent = nullptr, TNode* next = nullptr):NodeHeader(parent, next){
		_allocate[0] = '\0';
	}
};

template<typename TKey, typename TValue>
struct hashtable_node_allocate {
	static constexpr bool enable = std::is_same_v<TKey, const char*> || std::is_same_v<TValue, const char*>;
};

template<
	typename TKey, 
	typename TValue>
class hashtable_node:
	public hashtable_node_key<TKey>,
	public hashtable_node_value<TValue>,
	public hashtable_node_base<hashtable_base<TKey, TValue>, hashtable_node<TKey, TValue>,false,
		hashtable_node_allocate<TKey,TValue>::enable> {
public:
	using Table = hashtable_base<TKey, TValue>;
	using TNode = hashtable_node<TKey, TValue>;
	using NodeKey = hashtable_node_key<TKey>;
	using NodeValue = hashtable_node_value<TValue>;
	using NodeBase = hashtable_node_base<hashtable_base<TKey, TValue>, hashtable_node<TKey, TValue>, false,
		hashtable_node_allocate<TKey, TValue>::enable>;
	using TypeValue = typename NodeValue::TypeValue;
	using h_int = uint32_t;

	template<class TKey>
	hashtable_node(Table* parent, TKey key, uint32_t l, h_int hash) :
		NodeKey(key, l, hash), NodeBase(parent) {
	}
	template<class TKey= const char*>
	hashtable_node(Table* parent, const char* key, uint32_t l, h_int hash) :
		NodeKey(NodeBase::_allocate, l, hash), NodeBase(parent) {
		if (key != nullptr) {
			memcpy((void*)NodeKey::_key, key, l);
			((char*)NodeKey::_key)[l] = '\0';
		}
	}

	template<class TKey>
	hashtable_node(Table* parent, TKey key, uint32_t l, h_int hash, TValue value) :
		NodeKey(key, l, hash), NodeValue(value), NodeBase(parent) {
	}
	template<class TKey = const char*>
	hashtable_node(Table* parent, const char* key, uint32_t l, h_int hash, TValue value) :
		NodeKey(NodeBase::_allocate, l, hash), NodeValue(value), NodeBase(parent) {
		if (key != nullptr) {
			memcpy((void*)NodeKey::_key, key, l);
			((char*)NodeKey::_key)[l] = '\0';
		}
	}
	// 已知BUG,printf key=const char* 结构长度错误。
	operator TypeValue&() {
		return *(TypeValue*)(NodeValue*)this;
	}
	TypeValue& get() {
		return *(TypeValue*)(NodeValue*)this;
	}
};


template<typename TKey>
class hashtable_node<TKey, void> :
	public hashtable_node_key<TKey>,
	public hashtable_node_base<hashtable_base<TKey, void>, hashtable_node<TKey, void>,false,
	hashtable_node_allocate<TKey, void>::enable > {
public:
	using Table = hashtable_base<TKey, void>;
	using TNode = hashtable_node<TKey, void>;
	using NodeKey = hashtable_node_key<TKey>;
	using NodeValue = void;
	using NodeBase = hashtable_node_base<hashtable_base<TKey, void>, hashtable_node<TKey, void>, false,
		hashtable_node_allocate<TKey, void>::enable>;
	using TypeValue = NodeKey;
	using h_int = uint32_t;

	template<class TKey>
	hashtable_node(Table* parent, TKey key, uint32_t l, h_int hash) :
		NodeKey(key, l, hash), NodeBase(parent) {
	}
	template<class TKey = const char*>
	hashtable_node(Table* parent, const char* key, uint32_t l, h_int hash) :
		NodeKey(NodeBase::_allocate, l, hash), NodeBase(parent) {
		if (key != nullptr) {
			memcpy((void*)NodeKey::_key, key, l);
			((char*)NodeKey::_key)[l] = '\0';
		}
	}
	// 已知BUG,printf key=const char* 结构长度错误。
	operator NodeKey&() {
		return *(NodeKey*)this;
	}
	NodeKey& get() {
		return (NodeKey*)this;
	}
};

template<typename TKey, typename TValue>
class hashtable_base {
public:
	using TNode = hashtable_node<TKey, TValue>;
	using h_int = uint32_t;

	TNode** _data;
	uint32_t _count;
	uint32_t _mask;
	uint32_t _size;

	hashtable_base():
		_data(nullptr),
		_count(0),
		_mask(0),
		_size(0)
	{
		resize(8);
	}

	// typelength
	template<typename TKey = const char*>
	static uint32_t typelength(const char* s) {
		if (!s)
			return 0;
		return strlen(s);
	}
	template<typename TKey = std::string>
	static uint32_t typelength(std::string& s) {
		return s.length();
	}
	template<typename TKey>
	static uint32_t typelength(TKey k) {
		return sizeof(TKey);
	}

	// hash
	static h_int strhash(const char* k,uint32_t l, h_int seed) {
		h_int h = seed;
		if (!k) {
			return h;
		}
		while (l>0) {
			h ^= (h << 5 | h >> 2) + k[--l];
		}
		return h;
	}

	template<typename TKey = const char*>
	static h_int hash(const char* k, uint32_t l, h_int seed) {
		return strhash(k,l,seed);
	}
	template<typename TKey = std::string>
	static h_int hash(const std::string& k, uint32_t l, h_int seed) {
		return strhash(k.c_str(), k.length(), seed);
	}
	template<bool is_class = std::is_class<TKey>>
	static h_int hash(const TKey& k, uint32_t l, h_int seed) {
		return strhash((const char*)&k, sizeof(k), seed);
	}
	template<typename TKey>
	static h_int hash(TKey k, uint32_t l, h_int seed) {
		return strhash((const char*)&k,sizeof(k), seed);
	}

	// comp
	template<typename TKey = const char*>
	static bool comp(const char* a, const char* b) {
		return a==b || strcmp(a,b)==0;
	}
	template<typename TKey = std::string>
	static bool comp(const std::string& a, const std::string& b) {
		return a.compare(b.c_str()) == 0;
	}
	template<typename TKey>
	static bool comp(const TKey& a,const TKey& b, uint32_t count) {
		return memcmp(&a,&b,sizeof(TKey))==0;
	}

	// get
	TNode* gethnode(h_int h) {
		uint32_t idx = h & _mask;
		TNode* node = _data[idx];
		for (; node != nullptr; node = node->_next) {
			if (h==node->_hash) {
				return node;
			}
		}
		return nullptr;
	}
	TNode* getnode(TKey key) {
		uint32_t l = typelength(key);
		h_int h = hash(key,l,0);
		uint32_t idx = h & _mask;
		TNode* node = _data[idx];
		for (; node != nullptr; node = node->_next) {
			if (comp(key, node->_key)) {
				return node;
			}
		}
		return nullptr;
	}
	bool empty(TKey key) {
		return getnode(key)==nullptr;
	}
	template<bool is_class = std::is_class_v<TKey>>
	TNode* get(TKey key) {
		TNode* node = getnode(key);
		return node;
	}
	template<typename TKey>
	TNode* get(TKey key) {
		TNode* node = getnode(key);
		return node;
	}
	// find
	TNode* findfromnode(TNode* node, TKey key) {
		uint32_t l = typelength(key);
		for (; node != nullptr; node = node->_next) {
			if (node->comp(key, l)) {
				return node;
			}
		}
		return nullptr;
	}

	// newkey
	TNode* newkey(TKey key) {
		if (_count >> 3 >= _size) {
			resize(_size << 1);
		}
		uint32_t keylen = typelength(key);
		h_int h = hash(key, keylen, 0);
		uint32_t idx = h & _mask;
		TNode* node = _data[idx];
		for (; node != nullptr; node = node->_next) {
			if (node->comp(key, keylen)) {
				return node;
			}
		}
		uint32_t size = TNode::fixkeysize(keylen);
		node = new(malloc(sizeof(TNode) + size)) TNode(this, key, keylen,h);
		node->_next = _data[idx];
		_data[idx] = node;
		++_count;
		return node;
	}

	// add
	
	template<typename TValue>
	TNode* add(TKey key, TValue value) {
		if (_count >> 3 >= _size) {
			resize(_size * 2);
		}
		uint32_t keylen = typelength(key);
		h_int h = hash(key, keylen, 0);
		uint32_t idx = h & _mask;
		TNode* node = _data[idx];
		for (; node != nullptr;node=node->_next) {
			if (node->comp(key, keylen)) {
				break;
			}
		}
		if (node == nullptr) {
			uint32_t size = TNode::fixkeysize(keylen);
			node = new(malloc(sizeof(TNode) + size)) TNode(this,key, keylen,h, value);
			node->_next = _data[idx];
			_data[idx] = node;
			++_count;
		}
		else {
			node->_value = value;
		}
		return node;
	}
	
	template<typename TValue=void>
	TNode* add(TKey key) {
		return newkey(key);
	}

	// remove
	bool remove(TKey key) {
		uint32_t keylen = typelength(key);
		h_int h = hash(key, keylen, 0);
		uint32_t idx = h & _mask;
		TNode* lnode = nullptr;
		TNode* node = _data[idx];
		for (; node != nullptr; node = node->_next) {
			if (node->comp(key, keylen)) {
				break;
			}
			lnode = node;
		}
		if (node != nullptr) {
			if (lnode != nullptr) {
				lnode->_next = node->_next;
			}
			if (node == _data[idx]) {
				_data[idx] = nullptr;
			}
			delete node;
			--_count;
			if (_size > 8 && _count < (_size >> 3)) {
				resize(_size >> 1);
			}
			return true;
		}
		return false;
	}
	typename TNode::TypeValue& operator[](TKey key) {
		return newkey(key)->get();
	}

	void clear() {
		if (_data != nullptr) {
			for (uint32_t i = 0; i < _size; i++) {
				TNode* rnode = nullptr;
				TNode* node = _data[i];
				uint32_t idx = 0;
				for (; node != nullptr; node = rnode) {
					rnode = node->_next;
					delete node;
				}
				_data[i] = nullptr;
			}
			free(_data);
		}
		_data = nullptr;
		_size = 0;
		_mask = 0;
		_count = 0;
	}
	void resize(uint32_t size) {
		if (size == 0)
			size = 8;
		uint32_t mask = size - 1;
		TNode** data=(TNode**)calloc(size,sizeof(TNode*));
		if (_data != nullptr) {
			for (uint32_t i = 0; i < _size; i++) {
				TNode* node = _data[i];
				uint32_t idx = 0;
				for (; node != nullptr; node = node->_next) {
					idx = node->_hash & mask;
					node->_next = data[idx];
					data[idx] = node;
				}
			}
			free(_data);
		}
		_data = data;
		_size = size;
		_mask = mask;
	}
};

template<typename TKey, typename TValue>
using hashtable = hashtable_base<TKey, TValue>;

