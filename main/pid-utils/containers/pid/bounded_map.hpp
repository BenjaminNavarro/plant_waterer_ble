/**
 * @file bounded_map.hpp
 * @author Robin Passama
 * @brief include file for bounded map class
 * @date 2022-06-10
 * 
 * @ingroup containers
 * 
 */
#pragma once

#include <tuple>
#include <array>
#include <iostream>
#include <pid/memory_zone.hpp>

namespace pid{

template<typename Key, typename Value, unsigned int Limit>
class BoundedMap: public MemoryZone<std::pair<Key,Value>,Limit>{

  using map_element_type = std::pair<Key,Value>;
  using iterator = typename MemoryZone<map_element_type, Limit>::iterator;
  using const_iterator = typename MemoryZone<map_element_type, Limit>::const_iterator;

private:


  iterator insert_key(const Key & key){
    iterator returned;
    for(auto it = this->begin(); it != this->end(); ++it){
      if(it->first==key){//nothing to do already inserted
        return it;
      } 
      else if(key < it->first){//if element has a greater value it means new one must be placed before 
        returned = this->add_element(it);
        if(returned != this->end()){
          returned->first = key;
        }
        return returned;
      }
    }
    returned = this->add_element(this->end());
    if(returned != this->end()){
      returned->first = key;
    }
    return returned;
  }

/////////////////////////////////////
public:
  BoundedMap(): MemoryZone<map_element_type,Limit>(){}
	BoundedMap(const BoundedMap & copied): MemoryZone<map_element_type,Limit>(copied){}
	BoundedMap(BoundedMap && moved): MemoryZone<map_element_type,Limit>(std::move(moved)){}

	BoundedMap& operator=(const BoundedMap & copied){
    this->MemoryZone<map_element_type,Limit>::operator=(copied);
    return (*this);
	}

	BoundedMap& operator=(BoundedMap && moved){
		this->MemoryZone<map_element_type,Limit>::operator=(std::move(moved));
  	return (*this);
	}

	virtual ~BoundedMap()=default;


  iterator insert(const Key& key){
    return (insert_key(key));
	}

  iterator erase(const Key& key){
    auto it = find(key);
    if(it==this->end()){
      return (this->end());
    }
    return (this->remove_element(it));
	}

  Value& operator[]( const Key& key ){
    auto it = find(key);
    if(it == this->end()){
      it = insert(key);
      if(it == this->end()){
        throw std::out_of_range("operator[]: capacity is not sufficient");
      }
    }
    // std::cout<<"[] it = "<<std::to_string(it->first)<<std::endl;
    return (it->second);
  }

  Value& operator[]( Key&& key ){
    auto it = find(key);
    if(it == this->end()){
      it = insert(key);
      if(it == this->end()){
        throw std::out_of_range("operator[]: capacity is not sufficient");
      }
    }
    // std::cout<<"[] it = "<<std::to_string(it->first)<<std::endl;
    return (it->second);
  }

  const Value& at( const Key& key ) const{
    auto it = find(key);
    if(it == this->end()){
      throw std::out_of_range("at(): key does not exist in bounded map");
    }
    return (it->second);
  }

  Value& at( const Key& key ){
    auto it = find(key);
    if(it == this->end()){
      throw std::out_of_range("at(): key does not exist in bounded map");
    }
    return (it->second);
  }

  const_iterator find(const Key& key) const{
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      if(iter->first == key){
        return (iter);
      }
      else if(key < iter->first){//we know we will not find the adequate key as its value should have been found before
        return (this->end());
      }
    }
    return (this->end());
  }

  iterator find(const Key& key) {
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      if(iter->first == key){
        return (iter);
      }
      else if(iter->first > key){//we know we will not find the adequate key as its value should have been found before
        return (this->end());
      }
    }
    return (this->end());
  }

};

}
