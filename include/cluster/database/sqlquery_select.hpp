/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLQUERY_SELECT_HPP
#define SQLQUERY_SELECT_HPP

#include <cluster/database/sqlqueryelement.hpp>
#include <string>

namespace cluster
{

/**
  * This represents a query that selects
  * some data from the database
 **/
class SQLQuery_select : public SQLQueryElement
{

public:
	/**
	  * Default constructor
	 **/
	SQLQuery_select() :
		SQLQueryElement(SQLQueryType::select),
		table(),
		columns(),
		step(Step::begin)
	{}

	/**
	  * Default destructor
	 **/
	virtual ~SQLQuery_select() {}

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
		return true;
	}

	/**
	  * Stores the query in the given Package
	 **/
	virtual void insert(Package &p) const override
	{
		p<<table;
		p<<columns;
	}

	/**
	  * Creates a copy of the query
	 **/
	virtual SQLQueryElement* clone() const override
	{
		return new SQLQuery_select(*this);
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

}; //end class SQLQuery_select

} //end namespace cluster

#endif //SQLQUERY_SELECT_HPP
