#pragma once

#include "Array1D.h"
#include <vector>

template <class T>
class Array2D {
private:
	std::vector<Array1D<T> > data;

public:
	Array2D() {}
	~Array2D() {}

	int size();
	Array1D<T>& operator[](int index);
};

template<class T> int Array2D<T>::size() {
	return data.size();
}

template<class T>
Array1D<T>& Array2D<T>::operator[](int index) {
	if (index >= data.size()) {
		data.resize(index + 1);
	}

	return data[index];
}
