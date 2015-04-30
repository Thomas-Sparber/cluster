/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/sqlquery_insertinto.hpp>
#include <cluster/database/database.hpp>

using namespace std;
using namespace cluster;

SQLQueryBuildValue SQLQuery_insertInto::add(const std::string &str)
{
	switch(step)
	{
	case Step::begin:
		table = str;
		step = Step::after_table_name;
		return SQLQueryBuildValue::parsed;
	case Step::after_table_name:
		if(str == "(")
		{
			step = Step::column;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "values")
		{
			step = Step::values_definition;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "default")return SQLQueryBuildValue::need_more;
		if(str == "default values")
		{
			step = Step::finished;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::column:
		columns.push_back(str);
		step = Step::after_column;
		return SQLQueryBuildValue::parsed;
	case Step::after_column:
		if(str == ",")
		{
			step = Step::column;
			return SQLQueryBuildValue::parsed;
		}
		if(str == ")")
		{
			step = Step::columns_finished;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::columns_finished:
		if(str == "values")
		{
			step = Step::values_definition;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "select")
		{
			step = Step::values_select;
			return SQLQueryBuildValue::parsed;
		}
		if(str == "default")return SQLQueryBuildValue::need_more;
		if(str == "default values")
		{
			step = Step::finished;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::values_select:
		throw SQLException("INSERT INTO ... SELECT not implemented yet");
	case Step::values_definition:
		if(str == "(")
		{
			step = Step::value;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::value:
		if(str == "(")
		{
			step = Step::single_value_select;
			return SQLQueryBuildValue::parsed;
		}
		values.push_back(str);
		step = Step::after_value;
		return SQLQueryBuildValue::parsed;
	case Step::after_value:
		if(str == ",")
		{
			step = Step::value;
			return SQLQueryBuildValue::parsed;
		}
		if(str == ")")
		{
			if(values.size() % columns.size() != 0)throw SQLException("Not enough values supplied");
			step = Step::values_finished;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::single_value_select:
		throw SQLException("SELECT inside value definition not implemented yet");
	case Step::values_finished:
		if(str == ",")
		{
			step = Step::values_definition;
			return SQLQueryBuildValue::parsed;
		}
		throw SQLException(std::string("Unexpected ") + str);
	case Step::finished:
		throw SQLException("Query is already finished");
	default:
		throw SQLException("Unknown error");
	}
}

bool SQLQuery_insertInto::checkDependencies(const Database &db, SQLResult *result) const
{
	//Check if table exists
	const Table *t = db.getTable(table);
	if(!t)
	{
		if(result)result->localFail(std::string("Table ")+table+" does not exist");
		return false;
	}

	//Check if columns exist
	for(const std::string &column : columns)
	{
		if(!t->columnExists(column))
		{
			if(result)result->localFail(std::string("The table ")+table+" has no column named "+column);
			return false;
		}
	}

	//Check if all non-null columns are specified
	for(const Column &c : t->getColumns())
	{
		if(c.notNull && !c.defaultValue.empty() && !c.autoIncrement)
		{
			bool found = false;
			for(const std::string &column : columns)
			{
				if(c.name == column)
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				if(result)result->localFail(std::string("Non-null column ")+c.name+" not specified");
				return false;
			}
		}
	}

	return true;
}

bool SQLQuery_insertInto::execute(Database &db, SQLResult *result, bool isCoordinator, const SQLQuery &q) const
{
	Table *t = db.getTable(table);
	if(!t)
	{
		if(result)result->localFail(std::string("Table ")+table+" does not exist");
		return false;
	}

	try {
		db.insert(table, columns, values, result, isCoordinator, q);
	} catch(const SQLException &e) {
		if(result)result->localFail(string("Error inserting data: ") + e.text);
		return false;
	}
	return true;
}
