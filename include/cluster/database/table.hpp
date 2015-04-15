/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef TABLE_HPP
#define TABLE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <cluster/package.hpp>
#include <cluster/database/column.hpp>
#include <cluster/database/index.hpp>

namespace cluster
{

class Table
{

public:
	/**
	  * Constructs a table using the given name
	 **/
	Table(const std::string &name);

	/**
	  * Default destructor
	 **/
	virtual ~Table();

	/**
	  * Adds the given column to the table
	 **/
	bool addColumn(const Column &column);

	/**
	  * Returns whether the table is empty
	 **/
	bool empty() const
	{
		return true; //TODO
	}

	/**
	  * Returns the name of the table
	 **/
	std::string getName() const
	{
		return name;
	}

	friend bool operator>> <>(const Package &p, Table &q);
	friend void operator<< <>(Package &p, const Table &q);

private:
	/**
	  * The name of the table
	 **/
	const std::string name;

	/**
	  * The columns of the table
	 **/
	std::vector<Column> columns;

	/**
	  * The indices of the table
	 **/
	std::vector<Index> indices;

	/**
	  * The file stream to read from and write to disk
	 **/
	std::fstream localFile;

}; //end class Table

/**
  * This function is overloaded from the Package class
  * to retrieve a Table from a Package
 **/
template <>
inline bool operator>>(const Package &p, Table &t)
{
	t.columns.clear();
	t.indices.clear();

	unsigned int columnsCount;
	if(!(p>>columnsCount))return false;
	for(unsigned int i = 0; i < columnsCount; ++i)
	{
		Column c("");
		if(!(p>>c))return false;
		t.columns.push_back(c);
	}

	unsigned int indexCount;
	if(!(p>>indexCount))return false;
	const std::vector<Column> columns;
	for(unsigned int i = 0; i < indexCount; ++i)
	{
		Index index(columns, false);
		if(!(p>>index))return false;
		t.indices.push_back(index);
	}

	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a Table into a Package
 **/
template <>
inline void operator<<(Package &p, const Table &t)
{
	const unsigned int columnsCount = t.columns.size();
	p<<columnsCount;
	for(const Column &column : t.columns)p<<column;

	const unsigned int indexCount = t.indices.size();
	p<<indexCount;
	for(const Index &index : t.indices)p<<index;
}

} //end namespace cluster

#endif //TABLE_HPP
