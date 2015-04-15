/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef INDEX_HPP
#define INDEX_HPP

#include <cluster/database/column.hpp>
#include <map>

namespace cluster
{

/**
  * The IndexColumn holds the column data for
  * the index. This can also be multiple columns
 **/
class IndexColumn
{

public:
	/**
	  * Constructs an IndexColumn for the given
	  * columns using the given data
	 **/
	IndexColumn(const std::vector<Column> &v_columns, const std::vector<void*> &v_data) :
		columns(&v_columns),
		data(v_data)
	{}

	/**
	  * Copy construtor
	 **/
	IndexColumn(const IndexColumn &c) :
		columns(c.columns),
		data(c.data)
	{}

	/**
	  * Assignment operator
	 **/
	IndexColumn& operator= (const IndexColumn &c)
	{
		columns = c.columns;
		data = c.data;
		return (*this);
	}

	/**
	  * Returns whether the given column is smaller
	  * than the current column
	 **/
	bool operator< (const IndexColumn &c) const
	{
		if(columns != c.columns)return false;
		for(unsigned int i = 0; i < data.size() && i < c.data.size(); ++i)
		{
			const int compare = (*columns)[i].compare(data[i], c.data[i]);
			if(compare < 0)return true;
			if(compare > 0)return false;
		}
		return false;
	}

	/**
	  * Returns whether the given column is smaller
	  * than the current column
	 **/
	bool operator> (const IndexColumn &c) const
	{
		if(columns != c.columns)return false;
		for(unsigned int i = 0; i < data.size() && i < c.data.size(); ++i)
		{
			const int compare = (*columns)[i].compare(data[i], c.data[i]);
			if(compare > 0)return true;
			if(compare < 0)return false;
		}
		return false;
	}

	/**
	  * Returns whether the given column is smaller
	  * than the current column
	 **/
	bool operator== (const IndexColumn &c) const
	{
		if(columns != c.columns)return false;
		for(unsigned int i = 0; i < data.size() && i < c.data.size(); ++i)
		{
			const int compare = (*columns)[i].compare(data[i], c.data[i]);
			if(compare != 0)return false;
		}
		return true;
	}

private:
	/**
	  * The type of data
	 **/
	const std::vector<Column> *columns;

	/**
	  * The data
	 **/
	std::vector<void*> data;

}; //end struct IndexColumn

/**
  * The indexElement stores information about
  * where to find the data for the index
 **/
struct IndexElement
{

	/**
	  * Constructs an IndexElement using a start and end position
	 **/
	IndexElement(long long ll_begin, long long ll_end) :
		begin(ll_begin),
		end(ll_end)
	{}

	/**
	  * The start position of the data
	 **/
	long long begin;

	/**
	  * The end position of the data
	 **/
	long long end;

}; //end struct IndexElement

/**
  * The class index is used for quick search
  * of elements
 **/
class Index
{

public:
	/**
	  * Constructs an index for the given columns
	 **/
	Index(const std::vector<Column> &v_columns, bool b_forPrimary) :
		forPrimary(b_forPrimary),
		columns(v_columns),
		container()
	{}

	/**
	  * Constructs an index for the given column
	 **/
	Index(const Column &column, bool b_forPrimary) :
		forPrimary(b_forPrimary),
		columns(),
		container()
	{
		columns.push_back(column);
	}

	/**
	  * Adds a column to the index
	 **/
	bool addColumn(const Column &c)
	{
		if(!container.empty())return false;
		columns.push_back(c);
		return true;
	}

	bool isForPrimary() const
	{
		return forPrimary;
	}

	/**
	  * Inserts the given data into the given index
	 **/
	bool insert(const std::vector<void*> &c, long long begin, long long end)
	{
		return insert(IndexColumn(columns, c), IndexElement(begin, end));
	}

	/**
	  * Inserts the given data into the index
	 **/
	bool insert(const IndexColumn &c, const IndexElement &e)
	{
		if(container.count(c) > 0)return false;
		container.insert(std::pair<IndexColumn, IndexElement>(c, e));
		return true;
	}

	/**
	  * Gets the given data from the container
	 **/
	IndexElement get(const std::vector<void*> &c)
	{
		return get(IndexColumn(columns, c));
	}

	/**
	  * Gets the given data from the container
	 **/
	IndexElement get(const IndexColumn &c)
	{
		auto it = container.find(c);
		if(it == container.cend())return IndexElement(0, 0);
		return it->second;
	}

	friend bool operator>> <>(const Package &p, Index &q);
	friend void operator<< <>(Package &p, const Index &q);

private:
	/**
	  * If the index is for the primary key
	 **/
	bool forPrimary;

	/**
	  * The columns the index is holding
	 **/
	std::vector<Column> columns;

	/**
	  * The map that holds the index elements
	 **/
	std::map<IndexColumn, IndexElement> container;

}; //end class Index

/**
  * This function is overloaded from the Package class
  * to retrieve a Index from a Package
 **/
template <>
inline bool operator>>(const Package &p, Index &i)
{
	i.columns.clear();

	if(!(p>>i.forPrimary))return false;

	unsigned int columnsCount;
	if(!(p>>columnsCount))return false;
	for(unsigned int j = 0; j < columnsCount; ++j)
	{
		Column c("");
		if(!(p>>c))return false;
		i.columns.push_back(c);
	}

	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a Index into a Package
 **/
template <>
inline void operator<<(Package &p, const Index &i)
{
	p<<i.forPrimary;

	const unsigned int columnsCount = i.columns.size();
	p<<columnsCount;
	for(const Column &column : i.columns)p<<column;
}

} //end namespace cluster

#endif //INDEX_HPP
