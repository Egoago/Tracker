#pragma once
#include <initializer_list>
#include <serializer.h>
//TODO remove log
#include "../Misc/Log.hpp"
#include "../Misc/Base.hpp"

namespace tr {
	template <class CellType>
	class Tensor {
		typedef uint uint;

		uint* _shape = nullptr;
		uint _dims = 0u;
		CellType* storage = nullptr;
		bool allocated = false;
		uint size = 0u;
		void error(const std::string& msg) const {
			Logger::error(msg);
			exit(1);
		}

		void calculateSize();
	public:
		//[Constructing]
		inline Tensor() {}
		inline Tensor(std::initializer_list<uint> shape) { allocate(shape); }
		inline Tensor(const Tensor<CellType>& other) { *this = other; }
		inline Tensor(std::initializer_list<uint> shape, const CellType& value) { allocate(shape, value); }
		void allocate(std::initializer_list<uint> shape);
		void allocate(std::initializer_list<uint> shape, const CellType& value);
		void allocate(const uint* shape, const uint dims);
		void allocate(const uint* shape, const uint dims, const CellType& value);

		Tensor<CellType>& operator=(const Tensor<CellType>& other) {
			Logger::logProcess(__FUNCTION__);
			allocate(other._shape, other._dims);
			std::copy(other.begin(), other.end(), begin());
			Logger::logProcess(__FUNCTION__);
			return *this;
		}

		void deallocate();
		~Tensor() { deallocate(); }

		inline uint getDims() const { return _dims; }
		inline const uint* getShape() const { return _shape; }
		inline uint getSize() const { return size; }

		//[Element accessing]

		//Access element with range check, slower
		CellType& at(std::initializer_list<uint> indices);
		const CellType& at(std::initializer_list<uint> indices) const;
		//Access element without range check, faster
		CellType& operator()(std::initializer_list<uint> indices);
		const CellType& operator()(std::initializer_list<uint> indices) const;
		inline CellType* begin() { return storage; }
		inline const CellType* begin() const { return storage; }
		inline CellType* end() { return storage + size; }
		inline const CellType* end() const { return storage + size; }
		template <const unsigned int iDim, typename IndexType = float>

		//[Serialization]
		friend std::ostream& operator<<(std::ostream& out, Bits<Tensor<CellType>&> o) {
			Logger::logProcess(__FUNCTION__);
			const uint dims = o.t.getDims();
			const uint* shape = o.t.getShape();
			out << bits(dims);
			if (dims > 0u) {
				for (uint i = 0; i < dims; i++)
					out << bits(shape[i]);
				for (CellType* i = o.t.begin(); i < o.t.end(); i++)
					out << bits(*i);
			}
			Logger::logProcess(__FUNCTION__);
			return out;
		}
		//TODO type check
		inline friend std::istream& operator>>(std::istream& ins, Bits<Tensor<CellType>&> o) {
			Logger::logProcess(__FUNCTION__);
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
			Logger::logProcess(__FUNCTION__);
			return ins;
		}
		//TODO remove log
		inline friend std::ostream& operator<<(std::ostream& os, const Tensor<CellType>& tensor) {
			Logger::logProcess(__FUNCTION__);
			if (tensor.allocated) {
				for (const CellType* i = tensor.begin(); i < tensor.end(); i++)
					os << *i << " ";
				os << std::endl;
			}
			else os << "tensor not yet allocated.";
			Logger::logProcess(__FUNCTION__);
			return os;
		}
	};

	template<class CellType>
	inline void Tensor<CellType>::calculateSize() {
		if (!allocated || _dims == 0u) size = 0u;
		else {
			size = 1u;
			if (_shape == nullptr)
				error("shape deleted");;
			for (uint i = 0u; i < _dims; i++)
				size = size * _shape[i];
		}
	}

	template<class CellType>
	inline void Tensor<CellType>::deallocate() {
		if (allocated) {
		Logger::logProcess(__FUNCTION__);
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
		Logger::logProcess(__FUNCTION__);
		}
	}

	template<class CellType>
	inline void Tensor<CellType>::allocate(std::initializer_list<uint> shape) {
		Logger::logProcess(__FUNCTION__);
		if (allocated)
			deallocate();
		_dims = (uint)shape.size();
		if (_dims == 0u) return;
		this->_shape = new uint[_dims];
		int i = 0;
		for (const auto dim : shape) {
			_shape[i] = dim;
			i++;
		}
		allocated = true;
		calculateSize();
		storage = new CellType[size];
		Logger::logProcess(__FUNCTION__);
	}

	template<class CellType>
	inline void Tensor<CellType>::allocate(std::initializer_list<uint> shape, const CellType& value) {
		allocate(shape);
		if (size > 0u) std::fill_n(storage, size, value);
	}

	template<class CellType>
	inline void Tensor<CellType>::allocate(const uint* shape, const uint dims) {
		Logger::logProcess(__FUNCTION__);
		if (allocated)
			deallocate();
		_dims = dims;
		if (_dims == 0u) return;
		_shape = new uint[_dims];
		for (uint i = 0; i < _dims; i++)
			_shape[i] = shape[i];
		allocated = true;
		calculateSize();
		storage = new CellType[size];
		Logger::logProcess(__FUNCTION__);
	}

	template<class CellType>
	inline void Tensor<CellType>::allocate(const uint* shape, const uint dims, const CellType& value) {
		allocate(shape, dims);
		if (size > 0u) std::fill_n(storage, size, value);
	}

	template<class CellType>
	inline CellType& Tensor<CellType>::at(std::initializer_list<uint> indices) {
#ifndef _DEBUG
		Logger::warning("remove at from release");
#endif // !_DEBUG

		if (!allocated)
			error("tensor not yet allocated");
		if (indices.size() != _dims)
			error(std::to_string(indices.size())
				+ " indices given for a tensor with dimensionality of "
				+ std::to_string(_dims));
		uint i = 0u;
		for (const auto index : indices) {
			if (index >= _shape[i])
				error("index"
					+ std::to_string(index)
					+ " at dimension "
					+ std::to_string(i)
					+ " is out of range of "
					+ std::to_string(_shape[i]));
			i++;
		}
		return this->operator()(indices);
	}

	template<class CellType>
	inline const CellType& Tensor<CellType>::at(std::initializer_list<uint> indices) const {
#ifndef _DEBUG
		Logger::warning("remove at from release");
#endif // !_DEBUG

		if (!allocated)
			error("tensor not yet allocated");
		if (indices.size() != _dims)
			error(std::to_string(indices.size())
				+ " indices given for a tensor with dimensionality of "
				+ std::to_string(_dims));
		uint i = 0u;
		for (const auto index : indices) {
			if (index >= _shape[i])
				error("index"
					+ std::to_string(index)
					+ " at dimension "
					+ std::to_string(i)
					+ " is out of range of "
					+ std::to_string(_shape[i]));
			i++;
		}
		return this->operator()(indices);
	}

	template<class CellType>
	inline CellType& Tensor<CellType>::operator()(std::initializer_list<uint> indices) {
		uint indexSum = 0u;
		uint i = 0;
		for (const auto index : indices) {
			indexSum *= _shape[i];
			indexSum += index;
			i++;
		}
		return storage[indexSum];
	}

	template<class CellType>
	inline const CellType& Tensor<CellType>::operator()(std::initializer_list<uint> indices) const {
		uint indexSum = 0u;
		uint i = 0;
		for (const auto index : indices) {
			indexSum *= _shape[i];
			indexSum += index;
			i++;
		}
		return storage[indexSum];
	}
}



