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

	static bool isSpecialCharacter(unsigned char c)
	{
		if(c == '_')return false;
		return !isalnum(c);
	}

	/**
	  * Creates the internal query tree which is
	  * used to execute it
	 **/
	bool createQueryTree(const Database &db, SQLResult *result)
	{
		std::string newQuery(query);
		bool lastWasNotAlnum = false;
		for(unsigned int i = 0; i < newQuery.size(); ++i)
		{
			if(newQuery[i] == ';')
			{
				if(newQuery.size() > i+1)
				{
					if(result)result->localFail("Provided more than one query");
					return false;
				}
				newQuery = newQuery.substr(0, i);
				break;
			}

			if(isSpecialCharacter(newQuery[i]) && !isspace(newQuery[i]))
			{
				lastWasNotAlnum = true;
				if(i > 0 && !isspace(newQuery[i]))
				{
					newQuery = newQuery.substr(0, i) + " " + newQuery.substr(i);
					++i;
				}
			}
			else if(lastWasNotAlnum && !isspace(newQuery[i]))
			{
				lastWasNotAlnum = false;
				newQuery = newQuery.substr(0, i) + " " + newQuery.substr(i);
				++i;
			}
			else lastWasNotAlnum = false;
		}
		std::stringstream ss(newQuery);
		return createQueryTree(ss, queryTree, db, result);
	}

	/**
	  * Executes the query on the given database
	 **/
	bool execute(Database &db, SQLResult *result) const
	{
		if(!queryTree)
		{
			if(result)result->localFail("Empty query tree");
			return false;
		}

		return queryTree->execute(db, result);
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
