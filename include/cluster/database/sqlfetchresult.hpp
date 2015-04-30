/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLFETCHRESULT_HPP
#define SQLFETCHRESULT_HPP

#include <string>
#include <vector>
#include <cluster/database/datavalue.hpp>

namespace cluster
{

/**
  * This struct is used to store one row
 **/
struct SQLFetchResult
{

	/**
	  * Default constructor
	 **/
	SQLFetchResult() :
		data(),
		key()
	{}

	/**
	  * Default virtual destructor
	 **/
	virtual ~SQLFetchResult() {}

	/**
	  * Constructs a fetch result for the given table
	 **/
	SQLFetchResult(std::size_t dataCount) :
		data(dataCount),
		key()
	{}

	/**
	  * Constructs a fetch result for the given table
	  * using the given data
	 **/
	SQLFetchResult(const std::vector<DataValue> &v_data) :
		data(v_data),
		key()
	{}

	/**
	  * Constructs a fetch result for the given table
	  * using the given data
	 **/
	SQLFetchResult(const std::vector<DataValue> &v_data, const std::vector<DataValue> &v_key) :
		data(v_data),
		key(v_key)
	{}

	/**
	  * The data
	 **/
	std::vector<DataValue> data;

	/**
	  * The key of the data
	 **/
	std::vector<DataValue> key;

}; //end struct SQLFetchResult

/**
  * This struct is used to store one row
 **/
struct SQLTableFetchResult : public SQLFetchResult
{

	/**
	  * Default constructor
	 **/
	SQLTableFetchResult() :
		SQLFetchResult(),
		table()
	{}

	/**
	  * Constructs a fetch result for the given table
	 **/
	SQLTableFetchResult(const std::string &str_table, std::size_t dataCount) :
		SQLFetchResult(dataCount),
		table(str_table)
	{}

	/**
	  * Constructs a fetch result for the given table
	  * using the given data
	 **/
	SQLTableFetchResult(const std::string &str_table, std::vector<DataValue> v_data) :
		SQLFetchResult(v_data),
		table(str_table)
	{}

	/**
	  * Constructs a fetch result for the given table
	  * using the given data
	 **/
	SQLTableFetchResult(const std::string &str_table, std::vector<DataValue> v_data, const std::vector<DataValue> &v_key) :
		SQLFetchResult(v_data, v_key),
		table(str_table)
	{}

	/**
	  * The name of the table
	 **/
	std::string table;

}; //end struct SQLTableFetchResult

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLFetchResult from a Package
 **/
template <>
inline bool operator>>(const Package &p, SQLFetchResult &r)
{
	if(!(p>>r.data))return false;
	if(!(p>>r.key))return false;
	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a SQLFetchResult into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLFetchResult &r)
{
	p<<r.data;
	p<<r.key;
}

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLTableFetchResult from a Package
 **/
template <>
inline bool operator>>(const Package &p, SQLTableFetchResult &r)
{
	if(!(p>>static_cast<SQLFetchResult&>(r)))return false;
	if(!(p>>r.table))return false;
	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a SQLTableFetchResult into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLTableFetchResult &r)
{
	p<<static_cast<const SQLFetchResult&>(r);
	p<<r.table;
}

} //end namespace cluster

#endif //SQLFETCHRESULT_HPP
