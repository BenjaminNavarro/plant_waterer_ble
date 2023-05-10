/**
 * @file bounded_set.hpp
 * @author Robin Passama
 * @brief include file for bounded set class
 * @date 2022-06-10
 * 
 * @ingroup containers
 * 
 */
#pragma once

#include <tuple>
#include <iostream>
#include <utility>
#include <pid/memory_zone.hpp>
#include <exception>
#include <type_traits>

namespace pid{


template<typename Key, unsigned int Limit>
class BoundedSet: public MemoryZone<Key, Limit>{
  using iterator = typename MemoryZone<Key, Limit>::iterator;
  using const_iterator = typename MemoryZone<Key, Limit>::const_iterator;

private:

  iterator insert_value(const Key & key){
    iterator returned;
    for(auto it = this->begin(); it != this->end(); ++it){
      if(*it==key){//nothing to do already inserted
        return it;
      } 
      else if(key < *it){//if element has a greater value it means new one must be placed before 
        returned = this->add_element(it);
        if(returned != this->end()){
          *returned = key;
        }
        return returned;
      }
    }
    returned = this->add_element(this->end());
    if(returned != this->end()){
      *returned = key;
    }
    return returned;
  }

/////////////////////////////////////
public:
	BoundedSet(): MemoryZone<Key,Limit>(){}

  template <typename AKey=Key, typename ...Keys>
  BoundedSet( typename std::enable_if<std::is_same<AKey, Key>::value, AKey>::type&& key, 
              Keys&&... keys): MemoryZone<Key,Limit>(){
    insert(std::forward<Key>(key), std::forward<Keys>(keys)...);
  }
	BoundedSet(const BoundedSet & copied): MemoryZone<Key,Limit>(copied){}
	BoundedSet(BoundedSet && moved): MemoryZone<Key,Limit>(std::move(moved)){}

	BoundedSet& operator=(const BoundedSet & copied){
    this->MemoryZone<Key,Limit>::operator=(copied);
    return (*this);
	}

	BoundedSet& operator=(BoundedSet && moved){
		this->MemoryZone<Key,Limit>::operator=(std::move(moved));
  	return (*this);
	}

	virtual ~BoundedSet()=default;


  iterator insert(const Key& key){
    return (insert_value(key));
  }

  iterator insert(Key&& key){
    return (insert_value(std::move(key)));
  }

  template<typename Key1=Key, typename Key2=Key, typename ...Keys>
  iterator insert(  typename std::enable_if<std::is_same<Key1, Key>::value, Key1>::type&& key1, 
                    typename std::enable_if<std::is_same<Key2, Key>::value, Key2>::type&& key2, 
                    Keys&&... keys){
    if(insert(key1) != this->end()){
      return insert(std::forward<Key2>(key2), std::forward<Keys>(keys)...);
    }
    return (this->end());
  }


  bool insert_all(const BoundedSet& other){
    for(auto& el: other){
      if(insert(el) == this->end()){
        return false;
      }
    }
    return true;
  }

  iterator erase(const Key& key){
    auto it = find(key);
    if(it==this->end()){
      return (this->end());
    }
    return (this->remove_element(it));
  }

 template<typename Key1, typename Key2, typename ...Keys>
  bool erase(Key1&& key1, Key2&& key2, Keys&&... keys){
    erase(key1);
    return (erase(std::forward<Key2>(key2), std::forward<Keys>(keys)...) != this->end());
  }


  const_iterator find(const Key& key) const{
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      if(*iter == key){
        return (iter);
      }
      else if(key < *iter){//we know we will not find the adequate key as its value should have been found before
        return (this->end());
      }
    }
    return (this->end());
  }

  iterator find(const Key& key) {
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      if(*iter == key){
        return (iter);
      }
      else if(*iter > key){//we know we will not find the adequate key as its value should have been found before
        return (this->end());
      }
    }
    return (this->end());
  }

};



}