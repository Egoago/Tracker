#pragma once
#include <initializer_list>
#include <serializer.h>
//TODO remove log
#include "../Misc/Log.h"

namespace tr {
	template <class CellType>
	class Tensor
	{
		typedef unsigned int uint;

		uint* _shape = nullptr;
		uint _dims = 0u;
		CellType* storage = nullptr;
		bool allocated = false;
		uint size = 0u;
		void error(const std::string& msg) {
			Logger::error(msg);
			exit(1);
		}

		void calculateSize();
	public:
		//[Constructing]
		Tensor() {}
		Tensor(std::initializer_list<uint> shape) { allocate(shape); }
		Tensor(std::initializer_list<uint> shape, const CellType& value) {
			allocate(shape);
			std::fill_n(storage, size, value);
		}
		void allocate(std::initializer_list<uint> shape);
		void allocate(const uint* shape, const uint dims);

		void deallocate();
		~Tensor() { deallocate(); }

		inline uint getDims() const { return _dims; }
		inline const uint* getShape() const { return _shape; }
		inline uint getSize() const { return size; }

		//[Element accessing]

		//Access element with range check, slower
		CellType& at(std::initializer_list<uint> indices);
		//Access element without range check, faster
		CellType& operator()(std::initializer_list<uint> indices);
		inline CellType* begin() { return storage; }
		inline CellType* end() { return storage + size; }



		std::string toString();

		//[Serialization]
		friend std::ostream& operator<<(std::ostream& out, Bits<class Tensor<CellType>&> o) {
			const uint dims = o.t.getDims();
			const uint* shape = o.t.getShape();
			out << bits(dims);
			if (dims > 0u) {
				for (uint i = 0; i < dims; i++)
					out << bits(shape[i]);
				for (CellType* i = o.t.begin(); i < o.t.end(); i++)
					out << bits(*i);
			}
			return out;
		}
		//TODO type check
		friend std::istream& operator>>(std::istream& ins, Bits<class Tensor<CellType>&> o) {
			o.t.deallocate();
			uint dims = 0u;
			ins >> bits(dims);
			if (dims > 0u) {
				uint* shape = new uint[dims];
				for (uint i = 0; i < dims; i++)
					ins >> bits(shape[i]);
				o.t.allocate(shape, dims);
				for (CellType* cell = o.t.begin(); cell < o.t.end(); cell++)
					ins >> bits(*cell);
			}
			return ins;
		}
	};

	template<class CellType>
	inline void Tensor<CellType>::calculateSize() {
		Logger::logProcess("calculating size");
		if (!allocated || _dims == 0u) size = 0u;
		else {
			size = 1u;
			if (_shape == nullptr)
				error("shape deleted");
			Logger::log("size: " + to_string(size));
			for (uint i = 0u; i < _dims; i++) {
				size = size * _shape[i];
				Logger::log("dimension: " + to_string(_shape[i]));
				Logger::log("size: " + to_string(size));
			}
		}
		Logger::logProcess("calculating size");
	}

	template<class CellType>
	inline void Tensor<CellType>::deallocate() {
		Logger::logProcess("deallocating tensor");
		if (allocated) {
			if (storage != nullptr) {
				delete[] storage;
				storage = nullptr;
			}
			if (_shape != nullptr) {
				delete[] _shape;
				_shape = nullptr;
			}
			allocated = false;
			_dims = 0u;
			size = 0u;
		}
		Logger::logProcess("deallocating tensor");
	}

	template<class CellType>
	inline void Tensor<CellType>::allocate(std::initializer_list<uint> shape) {
		Logger::logProcess("allocating1");
		if (allocated)
			deallocate();
		_dims = (uint)shape.size();
		Logger::log("dims: " + to_string(_dims));
		if (_dims == 0u) return;
		this->_shape = new uint[_dims];
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
		Logger::logProcess("allocating1");
	}

	template<class CellType>
	inline void Tensor<CellType>::allocate(const uint* shape, const uint dims) {
		Logger::logProcess("allocating2");
		if (allocated)
			deallocate();
		_dims = dims;
		if (_dims == 0u) return;
		_shape = new uint[_dims];
		for (uint i = 0; i < _dims; i++)
			_shape[i] = shape[i];
		allocated = true;
		calculateSize();
		Logger::log("size: " + to_string(size));
		storage = new CellType[size];
		Logger::logProcess("allocating2");
	}

	template<class CellType>
	inline CellType& Tensor<CellType>::at(std::initializer_list<uint> indices) {
		Logger::logProcess("at");
		if (!allocated)
			error("tensor not yet allocated");
		if (indices.size() != _dims)
			error(to_string(indices.size())
				+ " indices given for a tensor with dimensionality of "
				+ to_string(_dims));
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

	template<class CellType>
	inline CellType& Tensor<CellType>::operator()(std::initializer_list<uint> indices) {
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

	template<class CellType>
	inline std::string Tensor<CellType>::toString() {
		Logger::logProcess("toString");
		std::string str;
		for (CellType* i = begin(); i < end(); i++)
			str += to_string(*i) + " ";
		Logger::logProcess("toString");
		return str;
	}
}



