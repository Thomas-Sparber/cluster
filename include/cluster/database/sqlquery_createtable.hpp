/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLQUERY_CREATETABLE_HPP
#define SQLQUERY_CREATETABLE_HPP

#include <cluster/database/sqlqueryelement.hpp>
#include <cluster/database/column.hpp>
#include <string>
#include <iostream>

namespace cluster
{

class SQLQuery_createTable : public SQLQueryElement
{

public:
	/**
	  * Default constructor
	 **/
	SQLQuery_createTable() :
		SQLQueryElement(SQLQueryType::create_table),
		ifNotExists(false),
		tableName(),
		columns(),
		step(Step::begin)
	{}

	/**
	  * Default destructor
	 **/
	virtual ~SQLQuery_createTable() {}

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
		if(!(p>>ifNotExists))return false;
		if(!(p>>tableName))return false;
		if(!(p>>columns))return false;
		return true;
	}

	/**
	  * Stores the query in the given Package
	 **/
	virtual void insert(Package &p) const override
	{
		p<<ifNotExists;
		p<<tableName;
		p<<columns;
	}

	/**
	  * Creates a copy of the query
	 **/
	virtual SQLQueryElement* clone() const override
	{
		return new SQLQuery_createTable(*this);
	}

	/**
	  * Gets the name of the table to be created
	 **/
	std::string getTableName() const
	{
		return tableName;
	}

private:
	/**
	  * If true, the query does not abort when
	  * the table already exists
	 **/
	bool ifNotExists;

	/**
	  * The name of the table
	 **/
	std::string tableName;

	/**
	  * The columns to be added to the table
	 **/
	std::vector<Column> columns;

	/**
	  * This inner class is used to record
	  * the current step of creating the query
	 **/
	enum class Step
	{
		begin,
		after_table_name,
		column_declaration,
		current_column_declaration,
		as_select_declaration,
		after_column_declaration
	} step; //end enum step

}; //end class SQLQuery_createTable

} //end namespace cluster

#endif //SQLQUERY_CREATETABLE_HPP
