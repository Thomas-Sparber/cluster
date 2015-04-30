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
#include <list>
#include <fstream>
#include <cluster/package.hpp>
#include <cluster/database/column.hpp>
#include <cluster/database/index.hpp>
#include <cluster/database/indexiterator.hpp>

namespace cluster
{

class Table
{

public:
	/**
	  * Constructs a table using the given name
	 **/
	Table(const std::string &name, const std::string &folder);

	/**
	  * Default destructor
	 **/
	virtual ~Table();

	/**
	  * Adds the given column to the table
	 **/
	void addColumn(const Column &column);

	/**
	  * Checks whether the column with the given name exists
	 **/
	bool columnExists(const std::string &column) const
	{
		for(const Column &c : columns)
		{
			if(c.name == column)return true;
		}
		return false;
	}

	/**
	  * Returns the columns of the table
	 **/
	const std::vector<Column>& getColumns() const
	{
		return columns;
	}

	/**
	  * Returns whether the table is empty
	 **/
	bool empty() const
	{
		for(unsigned int i = 0; i < indices.size(); ++i)
		{
			if(!indices[i].empty())return false;
		}

		return true;
	}

	/**
	  * Inserts the given row into the table
	  * the values need to be of the same size
	  * as the columns
	 **/
	void getInsertValues(const std::vector<std::string> &columns, const std::string *values, std::vector<DataValue> &out);

	/**
	  * Inserts the given row into the table
	 **/
	std::vector<DataValue> insert(const std::vector<DataValue> &values);

	/**
	  * Selects the data from the table
	 **/
	void select(const IndexElement &element, DataValue *out) const;

	/**
	  * Selects the data from the table
	 **/
	void select(const std::vector<Column> &columns, IndexIterator &out) const;

	/**
	  * Selects the next row using the given IndexIterator.
	  * The given vector needs to be of the size as the
	  * amount of columns to be selected
	 **/
	void selectNext(IndexIterator &it, DataValue *out, std::vector<DataValue> &key) const;

	/**
	  * Returns a reference to the primary key
	 **/
	const Index* getPrimaryKey() const
	{
		for(unsigned int i = 0; i < indices.size(); ++i)
		{
			if(indices[i].isForPrimary())return &indices[i];
		}
		return nullptr;
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
	void readRow(long long start, long long end, DataValue *out) const;

private:
	/**
	  * The name of the table
	 **/
	const std::string name;

	/**
	  * The folder where the table file is located
	 **/
	const std::string folder;

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
	mutable std::fstream localFile;

	/**
	  * The local file which holds the indices within the data file
	 **/
	std::fstream indexFile;

	/**
	  * A list of available space within the table file;
	 **/
	std::list<IndexElement> availableSpace;

}; //end class Table

/**
  * This function is overloaded from the Package class
  * to retrieve a Table from a Package
 **/
template <> bool operator>>(const Package &p, Table &t);


/**
  * This function is overloaded from the Package class
  * to insert a Table into a Package
 **/
template <> void operator<<(Package &p, const Table &t);

} //end namespace cluster

#endif //TABLE_HPP
