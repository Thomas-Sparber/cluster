/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef COLUMN_HPP
#define COLUMN_HPP

#include <string>

namespace cluster
{

enum class ColumnType : unsigned char
{

	c_invalid = ' ',

	c_text = 't',

	c_integer = 'i',

	c_double = 'd'

}; //end enum ColumnType

/**
  * This function is overloaded from the Package class
  * to retrieve a ColumnType from a Package
 **/
template <>
inline bool operator>>(const Package &p, ColumnType &t)
{
	return p>>reinterpret_cast<unsigned char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a ColumnType into a Package
 **/
template <>
inline void operator<<(Package &p, const ColumnType &t)
{
	p<<reinterpret_cast<const unsigned char&>(t);
}

struct Column
{

	/**
	  * Constructs a column using the given name
	 **/
	Column(const std::string &str_name) :
		name(str_name),
		type(ColumnType::c_invalid),
		notNull(false),
		primaryKey(false),
		unique(false),
		autoIncrement(false),
		defaultValue()
	{}

	int compare(void *a, void *b) const
	{
		switch(type)
		{
		case ColumnType::c_text:
			return static_cast<std::string*>(a)->compare(*static_cast<std::string*>(b));
		case ColumnType::c_integer:
			return (*static_cast<int*>(a)) < (*static_cast<int*>(b)) ? -1 :
				(*static_cast<int*>(a)) > (*static_cast<int*>(b)) ? 1 :
				0;
		case ColumnType::c_double:
			return (*static_cast<double*>(a)) < (*static_cast<double*>(b)) ? -1 :
				(*static_cast<double*>(a)) > (*static_cast<double*>(b)) ? 1 :
				0;
		default: return 0; //Error
		}
	}

	void enableAutoIncrement()
	{
		switch(type)
		{
		case ColumnType::c_integer:
			autoIncrement = true;
			break;
		default:
			throw SQLException("This column can not hat AUTOINCREMENT");
		}
	}

	bool setType(const std::string &t)
	{
		if(t == "text")
		{
			type = ColumnType::c_text;
			return true;
		}
		if(t == "integer")
		{
			type = ColumnType::c_integer;
			return true;
		}
		if(t == "double")
		{
			type = ColumnType::c_double;
			return true;
		}
		return false;
	}

	void setDefault(std::string value)
	{
		defaultValue = value;
	}

	std::string getTypeAsString() const
	{
		switch(type)
		{
		case ColumnType::c_text: return "text";
		case ColumnType::c_integer: return "integer";
		case ColumnType::c_double: return "double";
		default: return "invalid";
		}
	}

	std::string toString() const
	{
		return name + " " + getTypeAsString() + (unique ? " UNIQUE" : "") + (notNull ? " NOT NULL" : "") + (primaryKey ? " PRIMARY KEY" : "");
	}

	/**
	  * The name of the column
	 **/
	std::string name;

	/**
	  * The type of the column
	 **/
	ColumnType type;

	/**
	  * Indicates whether the column can be null or not
	 **/
	bool notNull;

	/**
	  * Indicates whether the column is a primary key
	 **/
	bool primaryKey;

	/**
	  * Indicates whether the column needs to be unique
	 **/
	bool unique;

	/**
	  * Indicates whether the column is an auto increment column
	 **/
	bool autoIncrement;

	/**
	  * The default value of the column. Can be empty
	 **/
	std::string defaultValue;

}; //end struct Column

/**
  * This function is overloaded from the Package class
  * to retrieve a Column from a Package
 **/
template <>
inline bool operator>>(const Package &p, Column &c)
{
	if(!(p>>c.name))return false;
	if(!(p>>c.type))return false;
	if(!(p>>c.notNull))return false;
	if(!(p>>c.primaryKey))return false;
	if(!(p>>c.unique))return false;
	if(!(p>>c.autoIncrement))return false;
	if(!(p>>c.defaultValue))return false;
	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a Column into a Package
 **/
template <>
inline void operator<<(Package &p, const Column &c)
{
	p<<c.name;
	p<<c.type;
	p<<c.notNull;
	p<<c.primaryKey;
	p<<c.unique;
	p<<c.autoIncrement;
	p<<c.defaultValue;
}

} //end namespace

#endif //COLUMN_HPP
