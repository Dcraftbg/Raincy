#pragma once
#define SHF djb2// Selected hash function
#define hashmap_of(T, K) \
	struct {T* val; K key;}

#define hmap_put(k, v) _hmap_put(k, v, SHF)
#define _hmap_put(k, v, hf)


#if 0
uint32_t colors[] = {}

int main(void) {
	
}
#endif