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
	SQLQuery_createTable() :
		SQLQueryElement(SQLQueryType::create_table),
		ifNotExists(false),
		tableName(),
		columns(),
		step(Step::begin)
	{}

	virtual ~SQLQuery_createTable() {}

	virtual SQLQueryBuildValue add(const std::string &str) override
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
			if(!columns.back().setType(str))throw SQLException(std::string("Unknown column type ") + str);
			return SQLQueryBuildValue::parsed;
		case Step::as_select_declaration:
			throw SQLException(std::string("CREATE TABLE AS SELECT not implemented yet"));
		case Step::after_column_declaration:
			return SQLQueryBuildValue::parsed;
		default:
			throw SQLException("Unknown error");
		}
	}

	virtual bool checkDependencies(const Database &db, SQLResult *result) const override
	{
		if(!ifNotExists)
		{
			if(db.tableExists(tableName))
			{
				result->localFail(std::string("Table ") + tableName + " already exists");
				return false;
			}
		}
		return true;
	}

	virtual bool execute(Database &db, SQLResult *result) override
	{
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

		Table *t = db.createTable(tableName);
		for(const Column &column : columns)t->addColumn(column);

		std::cout<<"CREATE TABLE "<<tableName<<" (";
		bool first = true;
		for(const Column &column : columns)
		{
			if(first)first = false;
			else std::cout<<",";
			std::cout<<column.toString();
		}
		std::cout<<");"<<std::endl;
		return true;
	}

	virtual bool extract(const Package &p) override
	{
		if(!(p>>ifNotExists))return false;
		if(!(p>>tableName))return false;
		unsigned int columnsCount;
		if(!(p>>columnsCount))return false;
		for(unsigned int i = 0; i < columnsCount; ++i)
		{
			Column column("");
			if(!(p>>column))return false;
			columns.push_back(column);
		}
		return true;
	}

	virtual void insert(Package &p) const override
	{
		p<<ifNotExists;
		p<<tableName;
		const unsigned int columnsCount = columns.size();
		p<<columnsCount;
		for(const Column &column : columns)p<<column;
	}

	virtual SQLQueryElement* clone() const override
	{
		return new SQLQuery_createTable(*this);
	}

	std::string getTableName() const
	{
		return tableName;
	}

private:
	bool ifNotExists;
	std::string tableName;
	std::vector<Column> columns;

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
