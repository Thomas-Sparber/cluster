/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/sqlquery_createtable.hpp>
#include <cluster/database/database.hpp>

using namespace std;
using namespace cluster;

SQLQueryBuildValue SQLQuery_createTable::add(const std::string &str)
{
	switch(step)
	{
	case Step::begin:
		if(str == "if")return SQLQueryBuildValue::need_more;
		if(str == "if not")return SQLQueryBuildValue::need_more;
		if(str == "if not exists")
		{
			ifNotExists = true;
			return SQLQueryBuildValue::parsed;
		}

		if(str.empty())throw SQLException("A table name can not be empty");
		for(const char c : str)if(!isalnum(c))throw SQLException("Table name contains invalid characters");

		tableName = str;
		step = Step::after_table_name;
		return SQLQueryBuildValue::parsed;
	case Step::after_table_name:
		if(str == "(")
		{
			step = Step::column_declaration;
			return SQLQueryBuildValue::parsed;
		}
		else if(str == "as")
		{
			step = Step::as_select_declaration;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::column_declaration:
		columns.push_back(str);
		step = Step::current_column_declaration;
		return SQLQueryBuildValue::parsed;
	case Step::current_column_declaration:
		if(columns.empty())throw SQLException("No column definitions");
		if(str == "not")return SQLQueryBuildValue::need_more;
		if(str == "primary")return SQLQueryBuildValue::need_more;
		if(str == "default")return SQLQueryBuildValue::need_more;
		if(str == ",")
		{
			step = Step::column_declaration;
			return SQLQueryBuildValue::parsed;
		}
		if(str == ")")
		{
			step = Step::after_column_declaration;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "not null")
		{
			columns.back().notNull = true;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "primary key")
		{
			columns.back().primaryKey = true;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "unique")
		{
			columns.back().unique = true;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "autoincrement")
		{
			columns.back().enableAutoIncrement();
			return SQLQueryBuildValue::parsed;
		}
		if(str.length() > 7 && str.substr(0,7) == "default")
		{
			columns.back().setDefault(str.substr(7));
			return SQLQueryBuildValue::parsed;
		}
		columns.back().setType(str);
		return SQLQueryBuildValue::parsed;
	case Step::as_select_declaration:
		throw SQLException(std::string("CREATE TABLE AS SELECT not implemented yet"));
	case Step::after_column_declaration:
		return SQLQueryBuildValue::parsed;
	default:
		throw SQLException("Unknown error");
	}
}

bool SQLQuery_createTable::checkDependencies(const Database &db, SQLResult *result) const
{
	//Check if table already exists
	if(!ifNotExists)
	{
		if(db.tableExists(tableName))
		{
			result->localFail(std::string("Table ") + tableName + " already exists");
			return false;
		}
	}

	//Check if primary key is defined
	bool primaryKeyDefined = false;
	for(const Column &column : columns)
	{
		if(column.primaryKey)
		{
			primaryKeyDefined = true;
			break;
		}
	}
	if(!primaryKeyDefined)
	{
		if(result)result->localFail("No primary key defined");
		return false;
	}
	return true;
}

bool SQLQuery_createTable::execute(Database &db, SQLResult *result, bool isCoordinator, const SQLQuery &q) const
{
	try {
		Table *t = db.createTable(tableName, result, isCoordinator, q);
		for(const Column &column : columns)t->addColumn(column);
	} catch(const SQLException &e) {
		if(result)result->localFail(std::string("Can't create table ")+tableName+": "+e.text);
		return false;
	}

	/*std::cout<<"CREATE TABLE "<<tableName<<" (";
	bool first = true;
	for(const Column &column : columns)
	{
		if(first)first = false;
		else std::cout<<", ";
		std::cout<<column.toString();
	}
	std::cout<<");"<<std::endl;*/
	return true;
}
