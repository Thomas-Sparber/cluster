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
struct IndexColumn
{

public:
	/**
	  * Constructs an IndexColumn for the given
	  * columns using the given data
	 **/
	IndexColumn(const std::vector<DataValue> &v_data) :
		data(v_data)
	{}

	/**
	  * Copy construtor
	 **/
	IndexColumn(const IndexColumn &c) :
		data(c.data)
	{}

	/**
	  * Assignment operator
	 **/
	IndexColumn& operator= (const IndexColumn &c)
	{
		data = c.data;
		return (*this);
	}

	/**
	  * Returns whether the given column is smaller
	  * than the current column
	 **/
	bool operator< (const IndexColumn &c) const
	{
		for(std::size_t i = 0; i < data.size() && i < c.data.size(); ++i)
		{
			const int compare = data[i].compare(c.data[i]);
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
		for(std::size_t i = 0; i < data.size() && i < c.data.size(); ++i)
		{
			const int compare = data[i].compare(c.data[i]);
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
		for(std::size_t i = 0; i < data.size() && i < c.data.size(); ++i)
		{
			const int compare = data[i].compare(c.data[i]);
			if(compare != 0)return false;
		}
		return true;
	}

	/**
	  * The index data
	 **/
	std::vector<DataValue> data;

}; //end struct IndexColumn

/**
  * This function is overloaded from the Package class
  * to retrieve a IndexColumn from a Package
 **/
template <>
inline bool operator>>(const Package &p, IndexColumn &c)
{
	c.data.clear();
	if(!(p>>c.data))return false;
	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a IndexColumn into a Package
 **/
template <>
inline void operator<<(Package &p, const IndexColumn &c)
{
	p<<c.data;
}

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
	  * Constructs an empty index
	 **/
	Index() :
		forPrimary(),
		columns(),
		container()
	{}

	/**
	  * Constructs an index for the given columns
	 **/
	Index(const std::vector<uint64_t> &v_columns, bool b_forPrimary) :
		forPrimary(b_forPrimary),
		columns(v_columns),
		container()
	{}

	/**
	  * Constructs an index for the given column
	 **/
	Index(uint64_t column, bool b_forPrimary) :
		forPrimary(b_forPrimary),
		columns(),
		container()
	{
		columns.push_back(column);
	}

	/**
	  * Adds a column to the index
	 **/
	void addColumn(uint64_t c)
	{
		if(!container.empty())throw SQLException("Can't add a column to a non empty index");

		columns.push_back(c);
	}

	bool isForPrimary() const
	{
		return forPrimary;
	}

	/**
	  * Inserts the given data into the given index
	 **/
	bool insert(const std::vector<DataValue> &c, long long ll_begin, long long ll_end)
	{
		return insert(IndexColumn(c), IndexElement(ll_begin, ll_end));
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
	std::map<IndexColumn,IndexElement>::const_iterator get(const std::vector<DataValue> &c) const
	{
		return get(IndexColumn(c));
	}

	/**
	  * Gets the given data from the container
	 **/
	std::map<IndexColumn,IndexElement>::const_iterator get(const IndexColumn &c) const
	{
		return container.find(c);
	}

	/**
	  * Gets the start of the index
	 **/
	std::map<IndexColumn,IndexElement>::const_iterator begin() const
	{
		return container.cbegin();
	}

	/**
	  * Gets the end of the index
	 **/
	std::map<IndexColumn,IndexElement>::const_iterator end() const
	{
		return container.cend();
	}

	/**
	  * Finds the indexColumn
	 **/
	std::map<IndexColumn,IndexElement>::const_iterator find(const IndexColumn &c) const
	{
		return container.find(c);
	}

	/**
	  * Returns the indices which are used for the index
	 **/
	const std::vector<std::uint64_t>& getColumns() const
	{
		return columns;
	}

	bool empty() const
	{
		return container.empty();
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
	std::vector<uint64_t> columns;

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
	if(!(p>>i.forPrimary))return false;
	if(!(p>>i.columns))return false;
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
	p<<i.columns;
}

} //end namespace cluster

#endif //INDEX_HPP
