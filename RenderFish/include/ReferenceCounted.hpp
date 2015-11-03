#pragma once

#include <cstdint>
#include "RenderFish.hpp"
#include "Parallel.hpp"

class ReferenceCounted
{
public:
//protected:
	AtomicInt32 n_references = 0;
public:
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
		if (ptr) atomic_add(&ptr->n_references, 1);
	}

	Reference(const Reference<T> &r) {
		ptr = r.ptr;
		if (ptr) atomic_add(&ptr->n_references, 1);
	}

	~Reference() {
		if (ptr && atomic_add(&ptr->n_references, -1) == 0)
			delete ptr;
	}

	Reference &operator=(const Reference<T> &r) {
		if (r->ptr) atomic_add(&r.ptr->n_references, 1);
		if (ptr && atomic_add(&ptr->n_references, -1) == 0) delete ptr;
		ptr = r.ptr;
		return *this;
	}

	Reference &operator=(T *p) {
		if (p) atomic_add(&p->n_references, 1);
		if (ptr && atomic_add(&ptr->n_references, -1) == 0) delete ptr;
		ptr = p;
		return *this;
	}

	T *operator->() { return ptr; }
	const T *operator->() const { return ptr; }
	operator bool() const { return ptr != nullptr; }
	const T *get_ptr() const { return ptr; }
};

