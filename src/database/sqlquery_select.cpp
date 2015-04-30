/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/sqlquery_select.hpp>
#include <cluster/database/indexiterator.hpp>
#include <cluster/database/sqlresult.hpp>
#include <cluster/database/database.hpp>

using namespace std;
using namespace cluster;

SQLQueryBuildValue SQLQuery_select::add(const std::string &str)
{
	switch(step)
	{
	case Step::begin:
		table = str;
		step = Step::after_table_name;
		return SQLQueryBuildValue::parsed;
	default:
		throw SQLException("Unknown error");
	}
}

bool SQLQuery_select::checkDependencies(const Database &db, SQLResult *result) const
{
	const Table *t = db.getTable(table);
	if(!t)
	{
		if(result)result->localFail(std::string("Table ")+table+" does not exist");
		return false;
	}
	return true;
}

bool SQLQuery_select::execute(Database &db, SQLResult *result, bool isCoordinator, const SQLQuery &q) const
{
	const Table *t = db.getTable(table);
	if(!t)
	{
		if(result)result->localFail(std::string("Table ")+table+" does not exist");
		return false;
	}
	db.select(table, t->getColumns(), result, isCoordinator, q);
	return true;
}
