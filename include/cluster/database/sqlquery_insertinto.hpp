/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLQUERY_INSERTINTO_HPP
#define SQLQUERY_INSERTINTO_HPP

#include <cluster/database/sqlqueryelement.hpp>
#include <string>
#include <iostream>

namespace cluster
{

class SQLQuery_insertInto : public SQLQueryElement
{

public:
	/**
	  * Default constructor
	 **/
	SQLQuery_insertInto() :
		SQLQueryElement(SQLQueryType::insert_into),
		table(),
		columns(),
		values(),
		step(Step::begin)
	{}

	/**
	  * Default destructor
	 **/
	virtual ~SQLQuery_insertInto() {}

	/**
	  * Constructs a query word by word.
	  * Returns the build value more data is
	  * needed to parse it. This function throws
	  * an SQLException if there is a syntax error.
	 **/
	virtual SQLQueryBuildValue add(const std::string &str) override;

	/**
	  * Checks the dependencies for the query on the given database
	 **/
	virtual bool checkDependencies(const Database &db, SQLResult *result) const override;

	/**
	  * Executes the query on the given database
	 **/
	virtual bool execute(Database &db, SQLResult *result, bool isCoordinator, const SQLQuery &q) const override;

	/**
	  * Loads the query from the given Package
	 **/
	virtual bool extract(const Package &p) override
	{
		if(!(p>>table))return false;
		if(!(p>>columns))return false;
		if(!(p>>values))return false;
		return true;
	}

	/**
	  * Stores the query in the given Package
	 **/
	virtual void insert(Package &p) const override
	{
		p<<table;
		p<<columns;
		p<<values;
	}

	/**
	  * Creates a copy of the query
	 **/
	virtual SQLQueryElement* clone() const override
	{
		return new SQLQuery_insertInto(*this);
	}

private:
	/**
	  * The table name where to insert the data
	 **/
	std::string table;

	/**
	  * The columns for which to insert data
	 **/
	std::vector<std::string> columns;

	/**
	  * The values to insert
	 **/
	std::vector<std::string> values;

	/**
	  * This inner class is used to record
	  * the current step of creating the query
	 **/
	enum class Step
	{
		begin,
		after_table_name,
		column,
		after_column,
		columns_finished,
		values_select,
		values_definition,
		value,
		after_value,
		single_value_select,
		values_finished,
		finished
	} step; //end enum step

}; //end class SQLQuery_insertInto

} //end namespace cluster

#endif //SQLQUERY_INSERTINTO_HPP
