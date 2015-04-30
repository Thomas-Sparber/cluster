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
#include <cluster/database/datavalue.hpp>

namespace cluster
{

struct Column
{

	/**
	  * Constructs invalid empty column
	 **/
	Column() :
		name(),
		type(),
		notNull(false),
		primaryKey(false),
		unique(false),
		autoIncrement(false),
		autoIncrementValue(),
		defaultValue()
	{}

	/**
	  * Constructs a column using the given name
	 **/
	Column(const std::string &str_name) :
		name(str_name),
		type(),
		notNull(false),
		primaryKey(false),
		unique(false),
		autoIncrement(false),
		autoIncrementValue(),
		defaultValue()
	{}

	void enableAutoIncrement()
	{
		if(type.canHaveAutoincrement())autoIncrement = true;
		else throw SQLException("This column can not have AUTOINCREMENT");
	}

	void setType(const std::string &t)
	{
		type.setType(t);
		autoIncrementValue = type;
	}

	const DataValue& nextAutoIncrementValue()
	{
		if(autoIncrementValue.isNull())autoIncrementValue.initDefaultValue();
		return ++autoIncrementValue;
	}

	void setDefault(const std::string &value)
	{
		defaultValue = value;
	}

	std::string toString() const
	{
		return name + " " + type.getTypeAsString() + (unique ? " UNIQUE" : "") + (notNull ? " NOT NULL" : "") + (primaryKey ? " PRIMARY KEY" : "");
	}

	/**
	  * The name of the column
	 **/
	std::string name;

	/**
	  * The type of the column
	 **/
	DataValue type;

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
	  * The auto increment value
	 **/
	DataValue autoIncrementValue;

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
	if(c.autoIncrement)if(!(p>>c.autoIncrementValue))return false;
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
	if(c.autoIncrement)p<<c.autoIncrementValue;
	p<<c.defaultValue;
}

} //end namespace

#endif //COLUMN_HPP
