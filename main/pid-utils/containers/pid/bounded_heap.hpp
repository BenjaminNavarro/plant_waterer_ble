/**
 * @file bounded_heap.hpp
 * @author Robin Passama
 * @brief include file for bounded heap class
 * @date 2022-06-10
 * 
 * @ingroup containers
 * 
 */
#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <iostream>
#include <array>
#include <functional>
#include <pid/memory_zone.hpp>

namespace pid{

template<typename T, unsigned int Limit>
class BoundedHeap: public MemoryZone<T,Limit>{

using iterator = typename MemoryZone<T,Limit>::iterator;
using const_iterator = typename MemoryZone<T,Limit>::const_iterator;

public:
	BoundedHeap(): MemoryZone<T,Limit>(){}
	BoundedHeap(const BoundedHeap & copied): MemoryZone<T,Limit>(copied){}
	BoundedHeap(BoundedHeap && moved): MemoryZone<T,Limit>(std::move(moved)){}
	BoundedHeap& operator=(const BoundedHeap & copied){
		this->MemoryZone<T,Limit>::operator=(copied);
		return (*this);
	}

	BoundedHeap& operator=(BoundedHeap && moved){
	  this->MemoryZone<T,Limit>::operator=(std::move(moved));
  	return (*this);
	}

	virtual ~BoundedHeap()=default;


  iterator put(){
    return(put(T()));
  }

  iterator put(const T& element){
    if(this->size()==this->capacity()){
      return (this->end());
    }
    auto it = this->add_element(this->begin());//always adding at beginning
    *it = element;//set the value
    return (it);
	}

	iterator put(T&& element){
    if(this->size()==this->capacity()){
      return (this->end());
    }
    auto it = this->add_element(this->begin());
    *it = std::move(element);//set the value
    return (it);
	}

  iterator rem(iterator& iter){
      return (this->remove_element(iter));
  }

	bool rem(const T& element){
    if(this->size()==0){
      return (false);
    }
    for(auto iter = this->begin(); iter != this->end(); ++iter){
      if(element == *iter){//the pointed element must be removed (require an operator== on T)
        rem(iter);
        return (true);
      }
    }
    return (false);
	}

};

}
