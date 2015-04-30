/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef INDEXITERATOR_HPP
#define INDEXITERATOR_HPP

#include <map>
#include <cluster/database/index.hpp>
#include <cluster/database/column.hpp>

namespace cluster
{

class Table;

/**
  * The indexiterator is used for fetching data
  * from a table. It holds pointers to the data
  * to be feteched.
 **/
struct IndexIterator
{

	/**
	  * Default constructor which creates an
	  * invalid IndexIterator
	 **/
	IndexIterator() :
		table(),
		index(),
		iterator(std::vector<DataValue>()),
		columns(),
		finished(true)
	{}

	/**
	  * Creates an IndexIterator that points
	  * to a specifix IndexElement of a specific Index
	  * in a specific table. Also the columns and a flag
	  * whether the query is already finished are inserted
	 **/
	IndexIterator(const std::string &t_table, unsigned int ui_index, IndexColumn it_iterator, const std::vector<Column> &v_columns, bool b_finished) :
		table(t_table),
		index(ui_index),
		iterator(it_iterator),
		columns(v_columns),
		finished(b_finished)
	{}

	/**
	  * The table where the index can be found
	 **/
	std::string table;

	/**
	  * The index where the indexElement can be found
	 **/
	std::size_t index;

	/**
	  * The iterator of the current indexElement
	 **/
	IndexColumn iterator;

	/**
	  * The columns which are fetched using this IndexIterator
	 **/
	std::vector<Column> columns;

	/**
	  * A flag whether all data has been fetched
	 **/
	bool finished;

}; //end struct IndexIterator

/**
  * This function is overloaded from the Package class
  * to retrieve a IndexIterator from a Package
 **/
template <>
inline bool operator>>(const Package &p, IndexIterator &t)
{
	if(!(p>>t.table))return false;
	if(!(p>>t.index))return false;
	if(!(p>>t.iterator))return false;
	if(!(p>>t.columns))return false;
	if(!(p>>t.finished))return false;
	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a IndexIterator into a Package
 **/
template <>
inline void operator<<(Package &p, const IndexIterator &t)
{
	p<<t.table;
	p<<t.index;
	p<<t.iterator;
	p<<t.columns;
	p<<t.finished;
}

} //end namespace cluster

#endif //INDEXITERATOR_HPP
