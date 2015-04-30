/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLQUERY_HPP
#define SQLQUERY_HPP

#include <string>
#include <vector>
#include <cluster/package.hpp>
#include <sstream>
#include <cluster/database/sqlqueryelement.hpp>
#include <cluster/database/sqlresult.hpp>

namespace cluster
{

class Table;
class Database;

class SQLQuery
{

public:
	/**
	  * Default constructor
	 **/
	SQLQuery();

	/**
	  * Copy constructor
	 **/
	SQLQuery(const SQLQuery &q);

	/**
	  * Constructs a query using the given string
	 **/
	SQLQuery(const std::string &query);

	/**
	  * Default destructor
	 **/
	virtual ~SQLQuery();

	/**
	  * Asignment operator
	 **/
	SQLQuery& operator= (const SQLQuery &q);

	/**
	  * Checks if the given query equls the current one
	 **/
	bool operator== (const SQLQuery &q) const;

	/**
	  * Checks if the given query is not equl to the current one
	 **/
	bool operator!= (const SQLQuery &q) const
	{
		return !((*this) == q);
	}

	/**
	  * Returns the query in string representation
	 **/
	const std::string& getQuery() const
	{
		return query;
	}

	static bool isSpecialCharacter(char c)
	{
		if(c == '_')return false;
		if(c == '\'')return false;
		return !isalnum(c);
	}

	/**
	  * Creates the internal query tree which is
	  * used to execute it
	 **/
	bool createQueryTree(const Database &db, SQLResult *result);

	/**
	  * Executes the query on the given database
	 **/
	bool execute(Database &db, SQLResult *result, bool isCoordinator) const
	{
		if(!queryTree)
		{
			if(result)result->localFail("Empty query tree");
			return false;
		}

		return queryTree->execute(db, result, isCoordinator, (*this));
	}

	friend bool operator>> <>(const Package &p, SQLQuery &q);
	friend void operator<< <>(Package &p, const SQLQuery &q);

private:
	/**
	  * Creates the internal query tree which is
	  * used to execute it
	 **/
	static bool createQueryTree(std::stringstream &ss, SQLQueryElement *&tree, const Database &db, SQLResult *result);

private:
	/**
	  * The string representation of the query
	 **/
	std::string query;

	/**
	  * The sql query tree
	 **/
	SQLQueryElement *queryTree;

}; //end class SQLQuery

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLQuery from a Package
 **/
template <>
bool operator>>(const Package &p, SQLQuery &q);


/**
  * This function is overloaded from the Package class
  * to insert a SQLQuery into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLQuery &q)
{
	p<<q.query;
	if(q.queryTree)
	{
		p<<q.queryTree->getType();
		p<<(*q.queryTree);
	}
	else
	{
		p<<SQLQueryType::invalid;
	}
}

} //end namespace cluster

#endif //SQLQUERY_HPP
