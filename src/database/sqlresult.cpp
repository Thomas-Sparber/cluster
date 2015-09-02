/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/sqlresult.hpp>
#include <cluster/database/table.hpp>
#include <cluster/database/database.hpp>
#include <cluster/database/sqlfetchresult.hpp>
#include <iostream>

using namespace std;
using namespace cluster;

SQLResult::SQLResult() :
	success(1),
	errorMessage(),
	results(),
	valuesUsed()
{}

SQLResult::~SQLResult()
{}

bool SQLResult::fetchRow(Database &db, vector<DataValue> &out)
{
	SQLFetchResult result;

	try {
		for(auto &res : results)
		{
			while(!res.second.finished)
			{
				db.selectNext(res.first, res.second, result);
				if(find(valuesUsed.cbegin(), valuesUsed.cend(), result.key) == valuesUsed.cend())
				{
					valuesUsed.push_back(result.key);
					out = result.data;
					return true;
				}
				else
				{
					continue;
				}
			}
		}
	} catch(SQLException &e) {
		localFail(string("Failed to fetch row: ") + e.text);
		cout<<e.text<<endl;
		throw;
	}

	return false;
}
