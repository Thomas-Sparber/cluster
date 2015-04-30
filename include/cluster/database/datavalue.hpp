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
#include <sstream>
#include <cluster/package.hpp>
#include <cluster/database/sqlexception.hpp>

namespace cluster
{

/**
  * Defines the type a value can be
 **/
enum class ValueType : char
{

	c_invalid,

	/**
	  * A bitfield value. can either be 0 or 1.
	  * The length defines the amount of bits per field
	 **/
	c_bit,	//requires length -> default 1

	/**
	  * A very small number that ranges from -128 to 127
	 **/
	c_tinyint,

	/**
	  * a boolean value
	 **/
	c_bool,

	/**
	  * Ranges from -32768 to 32767
	 **/
	c_smallint,

	/**
	  * Ranges from -8388608 to 8388607
	 **/
	c_mediumint,

	/**
	  * Ranges from -2147483648 to 2147483647
	 **/
	c_int,

	/**
	  * Ranges from -9223372036854775808 to 9223372036854775807
	 **/
	c_bigint,

	/**
	  * Numbers stores as text
	 **/
	c_decimal,

	/**
	  * -3.402823466E+38 to -1.175494351E-38 and 0 and 1.175494351E-38 to -3.402823466E+38
	 **/
	c_float,

	/**
	  * -1.7976931348623157E+308 to -2.2250738585072014E-308 and 0 and 1.7976931348623157E+308 to 2.2250738585072014E-308
	 **/
	c_double,

	/**
	  * Date from '1000-01-01' to '9999-12-31'
	 **/
	c_date,

	/**
	  * Date and time from '1000-01-01 00:00:00' to '9999-12-31 23:59:59'
	 **/
	c_datetime,

	/**
	  * Timestamp from '1970-01-01 00:00:01' to '2038-01-19 03:14:07'
	 **/
	c_timestamp,

	/**
	  * Time from -838:59:59 to 838:59:59
	 **/
	c_time,

	/**
	  * 1901 to 2155 and 0000, two digits from (19)70 to (20)69
	 **/
	c_year,	//length -> either 2 or 4 digits. default 4

	/**
	  * Simple character with fixed size
	 **/
	c_char, //length -> 0 to 255

	/**
	  *  Stores bytes
	 **/
	c_binary, //length

	/**
	  * Characters with variable size
	 **/
	c_text,

}; //end enum ColumnType

/**
  * This function is overloaded from the Package class
  * to retrieve a ValueType from a Package
 **/
template <>
inline bool operator>>(const Package &p, ValueType &t)
{
	return p>>reinterpret_cast<char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a ValueType into a Package
 **/
template <>
inline void operator<<(Package &p, const ValueType &t)
{
	p<<reinterpret_cast<const char&>(t);
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
	DataValue() :
		type(ValueType::c_invalid),
		length(0),
		value(nullptr)
	{}

	DataValue(const DataValue &v) :
		type(v.type),
		length(v.length),
		value(nullptr)
	{
		copyFrom(v);
	}

	~DataValue()
	{
		deleteValue();
	}

	DataValue& operator= (const DataValue &v)
	{
		copyFrom(v);
		return (*this);
	}

	/**
	  * Copies the members from the given DataValue
	 **/
	void copyFrom(const DataValue &v);

	/**
	  * Inits the DataValue with the default value
	  * for the type
	 **/
	void initDefaultValue();

	/**
	  * Returns the string representation of the value
	 **/
	std::string toString() const;

	/**
	  * Sets the given value
	 **/
	void setValue(const std::string &v);

	/**
	  * Increases the value by one
	 **/
	DataValue& operator++ ();

	/**
	  * Compares two data values
	 **/
	bool operator== (const DataValue &v) const
	{
		return compare(v) == 0;
	}

	/**
	  * Compares two data values
	 **/
	bool operator!= (const DataValue &v) const
	{
		return compare(v) != 0;
	}

	/**
	  * Compares two data values
	 **/
	bool operator> (const DataValue &v) const
	{
		return compare(v) > 0;
	}

	/**
	  * Compares two data values
	 **/
	bool operator< (const DataValue &v) const
	{
		return compare(v) < 0;
	}

	/**
	  * Compares two data values
	 **/
	bool operator>= (const DataValue &v) const
	{
		return compare(v) >= 0;
	}

	/**
	  * Compares two data values
	 **/
	bool operator<= (const DataValue &v) const
	{
		return compare(v) <= 0;
	}

	/**
	  * Sets the type of the DataValue
	 **/
	void setType(const std::string &t);

	/**
	  * Returns if the given DataValue is of the samy type
	 **/
	bool equalType(const DataValue &other) const
	{
		return type == other.type && length == other.length;
	}

	/**
	  * Returns the type as string
	 **/
	std::string getTypeAsString() const
	{
		switch(type)
		{
		case ValueType::c_bit:		return "bit"; //TODO length
		case ValueType::c_year:		return "year";
		case ValueType::c_tinyint:	return "tinyint";
		case ValueType::c_bool:		return "bool";
		case ValueType::c_time:		return "time";
		case ValueType::c_smallint:	return "smallint";
		case ValueType::c_mediumint:return "mediumint";
		case ValueType::c_timestamp:return "timestamp";
		case ValueType::c_int:		return "int";
		case ValueType::c_bigint:	return "bigint";
		case ValueType::c_text:		return "text";
		case ValueType::c_decimal:	return "decimal";
		case ValueType::c_float:	return "float";
		case ValueType::c_double:	return "double";
		case ValueType::c_date:		return "date";
		case ValueType::c_datetime:	return "datetime";
		case ValueType::c_char:		return "char";	//TODO length
		case ValueType::c_binary:	return "binary"; //TODO length
		default:					return "invalid";
		}
	}

	/**
	  * Retrieves the value from the package
	 **/
	bool retrieve(const Package &pkg);

	/**
	  * Inserts the value into the package
	 **/
	void insert(Package &pkg) const;

	/**
	  * Deletes the value
	 **/
	void deleteValue();

	/**
	  * Compares the given value with the current value
	 **/
	int compare(const DataValue &other) const;

	/**
	  * Returns the size of the data in byte
	 **/
	long long getDataSize() const;

	/**
	  * Writes the value to the given ostream
	  * in binary form
	 **/
	void writeTo(std::ostream &o) const;

	/**
	  * Loads the value from the given input stream
	  * in binary form
	 **/
	bool loadFrom(std::istream &i);

	/**
	  * Returns whether the value type can be auto incremented
	 **/
	bool canHaveAutoincrement() const
	{
		switch(type)
		{
		case ValueType::c_tinyint:
		case ValueType::c_smallint:
		case ValueType::c_mediumint:
		case ValueType::c_int:
		case ValueType::c_bigint:
			return true;
		default:
			return false;
		}
	}

	/**
	  * Returns whether the value is null
	 **/
	bool isNull() const
	{
		return (value == nullptr);
	}

	/**
	  * The type of data
	 **/
	ValueType type;

	/**
	  * The length of the data
	 **/
	unsigned int length;

	/**
	  * The actual value
	 **/
	void *value;

}; //end struct DataValue

/**
  * This function is overloaded from the Package class
  * to retrieve a DataValue from a Package
 **/
template <>
inline bool operator>>(const Package &p, DataValue &t)
{
	return t.retrieve(p);
}


/**
  * This function is overloaded from the Package class
  * to insert a DataValue into a Package
 **/
template <>
inline void operator<<(Package &p, const DataValue &t)
{
	t.insert(p);
}

} //end namespace

#endif //DATAVALUE_HPP
