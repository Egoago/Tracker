#pragma once
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"
#include <serializer.h>
#include <iostream>
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

	Object() : snapshots(Registry(boost::extents[0][0][0][0][0][0])){}
	Object(std::string name);
	/*void print(std::stream& os) {
		os << objectName << std::endl;
	}*/
	~Object() {}

	void setName(const char* str) { objectName = str; }
	void configTest(const char* str) { config.setEntry("test", str); }

	friend std::ostream& operator<<(std::ostream& out, Bits<class Object&> object)
	{
		std::cout << "save object start" << std::endl;
		out << bits(object.t.objectName) << bits(object.t.config);
		std::cout << "save object end" << std::endl;
		return (out);
	}
	friend std::istream& operator>>(std::istream& in, Bits<class Object&> object)
	{
		in >> bits(object.t.objectName) >> bits(object.t.config);
		return (in);
	}

	void print() { std::cout << config.getEntry("test") << std::endl; }
};