/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/sqlquery.hpp>
#include <cluster/database/sqlresult.hpp>
#include <cluster/database/table.hpp>
#include <cluster/database/database.hpp>
#include <cluster/database/sqlquery_createtable.hpp>
#include <cluster/database/sqlquery_insertinto.hpp>
#include <cluster/database/sqlquery_select.hpp>
#include <iostream>
#include <ctype.h>

using namespace std;
using namespace cluster;

SQLQuery::SQLQuery() :
	query(),
	queryTree(nullptr)
{}

SQLQuery::SQLQuery(const std::string &str_query) :
	query(str_query),
	queryTree(nullptr)
{}

SQLQuery::SQLQuery(const SQLQuery &q) :
	query(q.query),
	queryTree(q.queryTree ? q.queryTree->clone() : nullptr)
{}

SQLQuery::~SQLQuery()
{
	if(queryTree)delete queryTree;
}

SQLQuery& SQLQuery::operator= (const SQLQuery &q)
{
	this->query = q.query;

	delete this->queryTree;
	this->queryTree = q.queryTree ? q.queryTree->clone() : nullptr;
	return (*this);
}

bool SQLQuery::operator== (const SQLQuery &q) const
{
	return query == q.query;
}

bool SQLQuery::createQueryTree(const Database &db, SQLResult *result)
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

bool SQLQuery::createQueryTree(stringstream &ss, SQLQueryElement *&tree, const Database &db, SQLResult *result)
{
	string s;
	string temp;
	SQLQueryType type = SQLQueryType::invalid;

	//Check SQL query type
	while(ss>>s)
	{
		try {
			if(!temp.empty())temp += " ";
			transform(s.begin(), s.end(), s.begin(), ::tolower);
			temp += s;
			if((type = SQLQueryElement::getQueryType(temp)) != SQLQueryType::invalid)
			{
				temp.clear();
				break;
			}
		} catch(const SQLException &ex) {
			string str;
			getline(ss, str);
			if(result)result->localFail(string("SQL Syntax error near >>") + s + str + "<<: " + ex.text);
			ss.str(string());
			return false;
		}
	}

	//Check if query type is valid
	switch(type)
	{
	case SQLQueryType::create_table:
		tree = new SQLQuery_createTable();
		break;
	case SQLQueryType::insert_into:
		tree = new SQLQuery_insertInto();
		break;
	case SQLQueryType::select:
		tree = new SQLQuery_select();
		break;
	default:
		if(result && result->wasSuccess())
		{
			result->localFail("Invalid SQL query");
		}
		return false;
	}

	//Append all of the parameters
	while(ss>>s)
	{
		try {
			if(!temp.empty())temp += " ";
			transform(s.begin(), s.end(), s.begin(), ::tolower);
			temp += s;
			switch(tree->add(temp))
			{
			case SQLQueryBuildValue::need_more:
				break;
			case SQLQueryBuildValue::parsed:
				temp.clear();
			}
		} catch(const SQLException &ex) {
			if(result)result->localFail(string("SQL Syntax error near ") + ss.str() + ": " + ex.text);
			ss.str(string());
			return false;
		}
	}

	//Checking for dependecies like existing tables
	return tree->checkDependencies(db, result);
}

template <>
bool cluster::operator>>(const Package &p, SQLQuery &q)
{
	q.query = "";
	if(q.queryTree)delete q.queryTree;
	q.queryTree = nullptr;

	SQLQueryType type = SQLQueryType::invalid;
	if(!(p>>q.query))return false;
	if(!(p>>type))return false;
	switch(type)
	{
	case SQLQueryType::create_table:
		q.queryTree = new SQLQuery_createTable();
		if(!(p>>(*q.queryTree)))return false;
		break;
	case SQLQueryType::insert_into:
		q.queryTree = new SQLQuery_insertInto();
		if(!(p>>(*q.queryTree)))return false;
		break;
	case SQLQueryType::select:
		q.queryTree = new SQLQuery_select();
		if(!(p>>(*q.queryTree)))return false;
		break;
	default:
		break;
	}
	return true;
}
