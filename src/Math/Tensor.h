#pragma once
#include <initializer_list>
#include <string>
//TODO remove log
#include "../Misc/Log.h"

namespace tr {
	template <typename CellType>
	class Tensor
	{
		typedef unsigned int uint;

		uint* _shape = nullptr;
		uint dims = 0u;
		CellType* storage = nullptr;
		bool allocated = false;
		uint size = 0u;
		void error(const std::string& msg) {
			Logger::error(msg);
			exit(1);
		}

		void calculateSize();
		void deallocate();
	public:
		Tensor() {}
		Tensor(std::initializer_list<uint> shape) { allocate(shape); }
		Tensor(std::initializer_list<uint> shape, const CellType& value) {
			allocate(shape);
			std::fill_n(storage, size, value);
		}

		~Tensor() { deallocate(); }

		void allocate(std::initializer_list<uint> shape);

		inline uint getDim() const { return dims; }
		inline uint getSize() const { return size; }

		//Access element with range check, slower
		CellType& at(std::initializer_list<uint> indices);

		//Access element without range check, faster
		CellType& operator()(std::initializer_list<uint> indices);

		std::string toString();

		inline CellType* begin() { return storage; }
		inline CellType* end() { return storage + size; }
};

	template<typename CellType>
	inline void Tensor<CellType>::calculateSize()
	{
		Logger::logProcess("calculating size");
		if (!allocated || dims == 0u) size = 0u;
		else {
			size = 1u;
			if (_shape == nullptr)
				error("shape deleted");
			Logger::log("size: " + to_string(size));
			for (uint i = 0u; i < dims; i++) {
				size = size * _shape[i];
				Logger::log("dimension: " + to_string(_shape[i]));
				Logger::log("size: " + to_string(size));
			}
		}
		Logger::logProcess("calculating size");
	}
	template<typename CellType>
	inline void Tensor<CellType>::deallocate()
	{
		Logger::logProcess("deallocating tensor");
		if (storage != nullptr) {
			delete[] storage;
			storage = nullptr;
		}
		if (_shape != nullptr) {
			delete[] _shape;
			_shape = nullptr;
		}
		allocated = false;
		Logger::logProcess("deallocating tensor");
	}
	template<typename CellType>
	inline void Tensor<CellType>::allocate(std::initializer_list<uint> shape)
	{
		Logger::logProcess("allocating");
		if (allocated)
			deallocate();
		dims = (uint)shape.size();
		Logger::log("dims: " + to_string(dims));
		if (dims == 0u) return;
		this->_shape = new uint[dims];
		int i = 0;
		for (const auto dim : shape) {
			_shape[i] = dim;
			Logger::log("dimension: " + to_string(_shape[i]));
			i++;
		}
		allocated = true;
		calculateSize();
		Logger::log("size: " + to_string(size));
		storage = new CellType[size];
		Logger::logProcess("allocating");
	}
	template<typename CellType>
	inline CellType& Tensor<CellType>::at(std::initializer_list<uint> indices)
	{
		Logger::logProcess("at");
		if (!allocated)
			error("tensor not yet allocated");
		if (indices.size() != dims)
			error(to_string(indices.size())
				+ " indices given for a tensor with dimensionality of "
				+ to_string(dims));
		uint i = 0u;
		for (const auto index : indices) {
			if (index >= _shape[i])
				error("index"
					+ to_string(index)
					+ " at dimension "
					+ to_string(i)
					+ " is out of range of "
					+ to_string(_shape[i]));
			i++;
		}
		Logger::logProcess("at");
		return this->operator()(indices);
	}
	template<typename CellType>
	inline CellType& Tensor<CellType>::operator()(std::initializer_list<uint> indices)
	{
		Logger::logProcess("()");
		uint indexSum = 0u;
		uint i = 0;
		for (const auto index : indices) {
			indexSum *= _shape[i];
			indexSum += index;
			i++;
		}
		Logger::logProcess("()");
		return storage[indexSum];
	}
	template<typename CellType>
	inline std::string Tensor<CellType>::toString()
	{
		Logger::logProcess("toString");
		std::string str;
		for (CellType* i = begin(); i < end(); i++)
			str += to_string(*i) + " ";
		Logger::logProcess("toString");
		return str;
	}
}
