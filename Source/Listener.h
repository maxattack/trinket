// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#pragma once
#include "Common.h"
#include <EASTL/vector.h>
#include <EASTL/algorithm.h>

// TODO: make a better "iterator" which accounts for events being added/removed mid-iteration

template<typename T>
class ListenerList {
private:
	eastl::vector<T*> listeners;

public:

	ListenerList() noexcept {}
	
	void TryAdd(T* listener) {
		let it = eastl::find(listeners.begin(), listeners.end(), listener);
		if (it == listeners.end())
			listeners.push_back(listener);
	}

	void TryRemove_Swap(T* listener) {
		let it = eastl::find(listeners.begin(), listeners.end(), listener);
		if (it != listeners.end()) {
			*it = listeners.back();
			listeners.pop_back();
		}
	}

	void TryRemove_Shift(T* listener) {
		let it = eastl::find(listeners.begin(), listeners.end(), listener);
		if (it != listeners.end())
			listeners.erase(it);
	}

	auto begin() { return listeners.begin(); }
	auto end() { return listeners.end(); }
};
