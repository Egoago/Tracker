#pragma once
#include <initializer_list>
#include <string>
#include "../Misc/Log.h"

namespace tr {
	template <typename CellType>
	class Tensor
	{
		typedef unsigned int uint;

		uint* dimensions = nullptr;
		uint dimCount = 0;
		CellType* storage = nullptr;
		bool allocated = false;
		void error(const std::string& msg) {
			Logger::error(msg);
			exit(1);
		}
	public:
		Tensor() {}
		Tensor(std::initializer_list<uint> dimensions) { allocate(dimensions); }
		~Tensor() {
			if (allocated) {
				delete[] storage;
				delete dimensions;
			}
		}
		void allocate(std::initializer_list<uint> dimensions) {
			if (allocated) {
				delete[] storage;
				delete this->dimensions;
			}
			dimCount = (uint)dimensions.size();
			if (dimCount == 0) return;
				this->dimensions = new uint[dimCount];
			for (const auto i : dimensions)
				this->dimensions[i] = i;
			allocated = true;
			storage = new CellType[getSize()];
		}

		inline uint getDim() const { return dimCount; }
		uint getSize() const {
			if (!allocated) return 0;
			uint size = 1;
			for (uint i = 0; i < dimCount; i++)
				size *= dimensions[i];
			return size;
		}
		//Reinstantiates the Tensor with the current dimensions
		void clear() {
			allocate(dimensions);
		}

		//Access element with range check, slower
		CellType& at(std::initializer_list<uint> indices) {
			if (!allocated)
				error("tensor not yet allocated");
			if (indices.size() != dimCount)
				error(to_string(indices.size())
					+ " indices given for a tensor with dimensionality of "
					+ to_string(dimCount));
			uint i = 0;
			for (const auto index : indices) {
				i++;
				if (index >= dimensions[i])
					error("index"
						+ to_string(index)
						+ " at dimension "
						+ to_string(i-1)
						+ " is out of range of "
						+ to_string(dimensions[i]));
			}
			return this->operator()(indices);
		}

		//Access element without range check, faster
		CellType& operator()(std::initializer_list<uint> indices) {
			uint indexSum = 0;
			uint i = 0;
			for (const auto index : indices) {
				indexSum *= dimensions[i];
				indexSum += index;
				i++;
			}
			return storage[indexSum];
		}

		std::string toString() {
			std::string str;
			for (uint i = 0; i < getSize(); i++)
				str += to_string(at({i})) + " ";
			return str;
		}

		inline CellType* begin() { return storage; }
		inline CellType* end() { return storage + getSize(); }
	};
}
