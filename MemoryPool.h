/*
 * MemoryPool.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase MemoryPool de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_MEMORYPOOL_H
#define MBED_MEMORYPOOL_H

#include "mbed_api.h"
#include "Mutex.h"


/** Define and manage fixed-size memory pools of objects of a given type.
  @tparam  T         data type of a single object (element).
  @tparam  queue_sz  maximum number of objects (elements) in the memory pool.

 @note
 Memory considerations: The memory pool data store and control structures will be created on current thread's stack,
 both for the mbed OS and underlying RTOS objects (static or dynamic RTOS memory pools are not being used).
*/
template<typename T, uint32_t pool_sz>
class MemoryPool  {
public:
    /** Create and Initialize a memory pool. */
    MemoryPool() {
    	_mtx.lock();
    	_pool_mem = new PoolCtrl_t[pool_sz];
    	if(_pool_mem){
    		for(int i=0;i<pool_sz;i++){
    			_pool_mem[i].mem = new T;
    			if(_pool_mem[i].mem){
    				_pool_mem[i].used = false;
    			}
    		}
    	}
    	_mtx.unlock();
    }

    /** Destroy a memory pool */
    ~MemoryPool() {
    	_mtx.lock();
    	for(int i=0;i<pool_sz;i++){
			delete(_pool_mem[i].mem);
		}
    	delete(_pool_mem);
    	_mtx.unlock();
    }

    /** Allocate a memory block of type T from a memory pool.
      @return  address of the allocated memory block or NULL in case of no memory available.
    */
    T* alloc(void) {
    	T* item;
    	_mtx.lock();
    	for(int i=0;i<pool_sz;i++){
    		if(!_pool_mem[i].used){
    			_pool_mem[i].used = true;
    			item = _pool_mem[i].mem;
    			_mtx.unlock();
    			return item;
    		}
		}
    	_mtx.unlock();
    	return (T*)0;
    }

    /** Allocate a memory block of type T from a memory pool and set memory block to zero.
      @return  address of the allocated memory block or NULL in case of no memory available.
    */
    T* calloc(void) {
    	T* item;
    	_mtx.lock();
    	for(int i=0;i<pool_sz;i++){
			if(!_pool_mem[i].used){
				_pool_mem[i].used = true;
				memset(_pool_mem[i].mem, 0, sizeof(T));
				item = _pool_mem[i].mem;
				_mtx.unlock();
				return item;
			}
		}
    	_mtx.unlock();
    	return (T*)0;
    }

    /** Free a memory block.
      @param   block  address of the allocated memory block to be freed.
      @return         osOK on successful deallocation, osErrorParameter if given memory block id
                      is NULL or invalid, or osErrorResource if given memory block is in an
                      invalid memory pool state.

    */
    osStatus free(T *block) {
    	_mtx.lock();
    	for(int i=0;i<pool_sz;i++){
			if(block == _pool_mem[i].mem){
				_pool_mem[i].used = false;
				_mtx.unlock();
				return osOK;
			}
		}
    	_mtx.unlock();
        return osError;
    }

private:
    /** Estructura de control del pool */
    struct PoolCtrl_t{
    	T* mem;					/// Bloque de memoria
    	bool used;				/// Flag de control de uso del bloque
    };

    Mutex 			_mtx;		/// Mutex para operaciones atómicas
    PoolCtrl_t* 	_pool_mem;	/// Pool de memoria

};


#endif

/** @}*/
