/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef DATAVALUE_HPP
#define DATAVALUE_HPP

#include <string>

namespace cluster
{

enum class ValueType : unsigned char
{

	c_invalid,

	c_text,

	c_integer,

	c_double

}; //end enum ColumnType

/**
  * This function is overloaded from the Package class
  * to retrieve a ValueType from a Package
 **/
template <>
inline bool operator>>(const Package &p, ValueType &t)
{
	return p>>reinterpret_cast<unsigned char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a ValueType into a Package
 **/
template <>
inline void operator<<(Package &p, const ValueType &t)
{
	p<<reinterpret_cast<const unsigned char&>(t);
}

/**
  * A DataValue represents a column value
  * of a table
 **/
struct DataValue
{

	/**
	  * Constructs an invalid data value
	 **/
	DataValue()
		type(ValueType::c_invalid),
		value(nullptr)
	{}

	/**
	  * The type of data
	 **/
	ValueType type;

	/**
	  * The actual value
	 **/
	void *value;

}; //end struct DataValue

} //end namespace

#endif //DATAVALUE_HPP
