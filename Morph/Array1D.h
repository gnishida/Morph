#pragma once

#include <vector>

template <class T>
class Array1D {
private:
	std::vector<T> data;

public:
	Array1D() {}
	~Array1D() {}

	int size();
	T& operator[](int index);
};

template<class T> int Array1D<T>::size() {
	return data.size();
}

template<class T>
T& Array1D<T>::operator[](int index) {
	if (index >= data.size()) {
		data.resize(index + 1);
	}

	return data[index];
}
