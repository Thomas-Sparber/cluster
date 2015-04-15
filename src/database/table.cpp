/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/table.hpp>

using namespace std;
using namespace cluster;

Table::Table(const string &str_name) :
	name(str_name),
	columns(),
	indices(),
	localFile(str_name+".table", ios_base::in | ios_base::out | ios_base::binary)
{}

Table::~Table()
{
	localFile.close();
}

bool Table::addColumn(const Column &column)
{
	if(!column.notNull && !empty())return false;

	if(!empty())
	{
		//TODO add column to existing data
	}
	columns.push_back(column);
	if(column.primaryKey)
	{
		bool found = false;
		for(Index &i : indices)
		{
			if(i.isForPrimary())
			{
				if(!i.addColumn(column))return false;
				found = true;
				break;
			}
		}
		if(!found)indices.push_back(Index(column, true));
	}
	else if(column.unique)indices.push_back(Index(column, false));
	return true;
}
