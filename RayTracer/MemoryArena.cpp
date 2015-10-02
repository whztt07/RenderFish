#include "MemoryArena.hpp"
#include <stdlib.h>
#include <algorithm> // max

inline void * MemoryArena::alloc(uint32_t size) {
	size = (size + 15) & (-15); // 16-bytes alignment
	if (_cur_block_pos + size > _block_size) {
		_used_blocks.push_back(_current_block);
		if (_available_blocks.size() && size <= _block_size) {
			_current_block = _available_blocks.back();
			_available_blocks.pop_back();
		}
		else {
			_current_block = alloc_aligned<char>(std::max(size, _block_size));
		}
		_cur_block_pos = 0;
	}
	void * ret = _current_block + _cur_block_pos;
	_cur_block_pos += size;
	return ret;
}

inline void MemoryArena::free_all() {
	_cur_block_pos = 0;
	while (!_used_blocks.empty()) {
		_available_blocks.push_back(_used_blocks.back());
		_used_blocks.pop_back();
	}
}
