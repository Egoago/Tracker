#pragma once
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>

class Object
{
	static ConfigParser config;
	std::string objectName;
	typedef boost::multi_array<Snapshot, 6> Registry;
	typedef Registry::index invdex;
	Registry& snapshots;

	void loadObject(const std::string& objectName);
	void generarteObject(const std::string& fileName);
	void generate6DOFs();

	//friend class boost::serialization::access;
	//// When the class Archive corresponds to an output archive, the
	//// & operator is defined similar to <<.  Likewise, when the class Archive
	//// is a type of input archive the & operator is defined similar to >>.
	//template<class Archive>
	//void serialize(Archive& ar, const unsigned int version)
	//{
	//	ar & objectName;
	//}
public:

	//Object() {}
	Object(std::string name);
	/*void print(std::stream& os) {
		os << objectName << std::endl;
	}*/
	~Object() {}
};