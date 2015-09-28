#pragma once

#include <cstdint>

typedef volatile int32_t AtomicInt32;
#ifdef HAS_64_BIT_ATOMICS
//typedef volatile int64_t AtomicInt64;
#endif

inline int32_t AtomicAdd(AtomicInt32 *v, int32_t delta) {
#if defined(RENDERFISH_PLATFORM_IS_WINDOWS)
	int32_t result;
	_ReadWriteBarrier();
	__asm {
		__asm mov edx, v
		__asm mov eax, delta
		__asm lock xadd [edx], eax
		__asm mov result, eax
	}
	_ReadWriteBarrier();
	return result + delta;
#elif defined(RENDERFISH_PLATFORM_IS_WINDOWS)
 	return OSAtomicAdd32Barrier(delta, v);
#else
 	// gcc x86 inline assembly
 	int32_t origValue;
 	__asm__ __volatile__("lock\n"
 		"xaddl %0, %1"
 		: "=r"(origValue), "=m"(*v) : "0"(delta)
 		: "memory");
	return origValue + delta;
#endif
}

class ReferenceCounted
{
public:
	AtomicInt32 n_references = 0;
	ReferenceCounted() {};
	virtual ~ReferenceCounted() {};
};


template <typename T>
class Reference {
private:
	T *ptr;

public:
	Reference(T *p = nullptr) {
		ptr = p;
		if (ptr) AtomicAdd(&ptr->n_references, 1);
	}

	Reference(const Reference<T> &r) {
		ptr = r.ptr;
		if (ptr) AtomicAdd(&ptr->n_references, 1);
	}

	~Reference() {
		if (ptr && AtomicAdd(&ptr->n_references, -1) == 0)
			delete ptr;
	}

	Reference &operator=(const Reference<T> &r) {
		if (r->ptr) AtomicAdd(&r.ptr->n_references, 1);
		if (ptr && AtomicAdd(&ptr->n_references, -1) == 0) delete ptr;
		ptr = r.ptr;
		return *this;
	}

	Reference &operator=(T *p) {
		if (p) AtomicAdd(&p->n_references, 1);
		if (ptr && AtomicAdd(&ptr->n_references, -1) == 0) delete ptr;
		ptr = p;
		return *this;
	}

	T *operator->() { return ptr; }
	const T *operator->() const { return ptr; }
	operator bool() const { return ptr != nullptr; }
	const T *get_ptr() const { return ptr; }
};

