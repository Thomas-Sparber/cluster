/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLQUERYELEMENT_HPP
#define SQLQUERYELEMENT_HPP

#include <string>

namespace cluster
{

class Database;
class SQLResult;

/**
  * The type of query
 **/
enum class SQLQueryType : unsigned char
{

	invalid = ' ',

	create_table = 't'

}; //end enum SQLQueryType

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLQueryType from a Package
 **/
template <>
inline bool operator>>(const Package &p, SQLQueryType &t)
{
	return p>>reinterpret_cast<unsigned char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a SQLQueryType into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLQueryType &t)
{
	p<<reinterpret_cast<const unsigned char&>(t);
}

enum class SQLQueryBuildValue
{
	need_more,
	parsed
}; //end enum SQLQueryBuildValue

class SQLException
{
public:
	SQLException(const std::string &str_text) : text(str_text) {};
	std::string text;
}; //end class SQLException

/**
  * This class represents one part of an
  * SQL query
 **/
class SQLQueryElement
{

public:
	/**
	  * Default constructor
	 **/
	SQLQueryElement(SQLQueryType t) : type(t) {}

	/**
	  * Default destructor
	 **/
	virtual ~SQLQueryElement() {}

	/**
	  * Returns the type of query
	 **/
	SQLQueryType getType() const
	{
		return type;
	}

	virtual SQLQueryBuildValue add(const std::string &str) = 0;

	virtual SQLQueryElement* clone() const = 0;

	virtual bool execute(Database &db, SQLResult *result) = 0;

	virtual bool checkDependencies(const Database &db, SQLResult *result) const = 0;

	virtual bool extract(const Package &p) = 0;

	virtual void insert(Package &p) const = 0;

	static SQLQueryType getQueryType(const std::string &s)
	{
		if(s == "create")return SQLQueryType::invalid;

		if(s == "create table")return SQLQueryType::create_table;

		throw SQLException(std::string("Invalid SQL query: " + s));
	}

private:
	/**
	  * The type of the query
	 **/
	const SQLQueryType type;

}; //end class SQLQueryElement

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLQueryElement from a Package
 **/
template <>
inline bool operator>>(const Package &p, SQLQueryElement &q)
{
	return q.extract(p);
}


/**
  * This function is overloaded from the Package class
  * to insert a SQLQueryElement into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLQueryElement &q)
{
	q.insert(p);
}

} //end namespace cluster

#endif //SQLQUERYELEMENT_HPP
