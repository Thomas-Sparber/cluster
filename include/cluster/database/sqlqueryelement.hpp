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
#include <cluster/database/sqlexception.hpp>
#include <cluster/package.hpp>

namespace cluster
{

class Database;
class SQLResult;
class SQLQuery;

/**
  * The type of query
 **/
enum class SQLQueryType : char
{

	invalid = ' ',

	create_table = 't',

	insert_into = 'i',

	select = 's'

}; //end enum SQLQueryType

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLQueryType from a Package
 **/
template <>
inline bool operator>>(const Package &p, SQLQueryType &t)
{
	return p>>reinterpret_cast<char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a SQLQueryType into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLQueryType &t)
{
	p<<reinterpret_cast<const char&>(t);
}

enum class SQLQueryBuildValue
{
	need_more,
	parsed
}; //end enum SQLQueryBuildValue

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

	/**
	  * Constructs a query word by word.
	  * Returns the build value more data is
	  * needed to parse it. This function throws
	  * an SQLException if there is a syntax error.
	 **/
	virtual SQLQueryBuildValue add(const std::string &str) = 0;

	/**
	  * Executes the query on the given database
	 **/
	virtual bool execute(Database &db, SQLResult *result, bool isCoordinator, const SQLQuery &q) const = 0;

	/**
	  * Checks the dependencies for the query on the given database
	 **/
	virtual bool checkDependencies(const Database &db, SQLResult *result) const = 0;

	/**
	  * Loads the query from the given Package
	 **/
	virtual bool extract(const Package &p) = 0;

	/**
	  * Stores the query in the given Package
	 **/
	virtual void insert(Package &p) const = 0;

	/**
	  * Creates a copy of the query
	 **/
	virtual SQLQueryElement* clone() const = 0;

	static SQLQueryType getQueryType(const std::string &s)
	{
		if(s == "create")return SQLQueryType::invalid;
		if(s == "insert")return SQLQueryType::invalid;

		if(s == "create table")return SQLQueryType::create_table;
		if(s == "insert into")return SQLQueryType::insert_into;
		if(s == "select")return SQLQueryType::select;

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
