/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLRESULT_HPP
#define SQLRESULT_HPP

#include <cluster/package.hpp>
#include <cluster/prototypes/address.hpp>
#include <cluster/database/indexiterator.hpp>
#include <list>

namespace cluster
{

class Database;

class SQLResult
{

public:
	/**
	  * Default Constructor
	 **/
	SQLResult();

	/**
	  * Default destructor
	 **/
	virtual ~SQLResult();

	/**
	  * Checks if the sql result is empty
	 **/
	bool empty() const
	{
		return !hasResult();
	}

	/**
	  * Adds the result of another cluster member
	  * to the result
	 **/
	void add(const SQLResult &res, const Address &address, std::size_t onlineClient)
	{
		if(!res.success)
		{
			success += success / abs(success);
			const bool wasEmpty = errorMessage.empty();
			errorMessage += (wasEmpty ? "" : ", ") + address.address + ": " + res.errorMessage;
		}

		for(auto r : res.results)
		{
//			if(r.first == 0)
//			{
				this->results.push_back(std::pair<unsigned int,IndexIterator>(onlineClient, r.second));
//			}
//			else
//			{
//				this->results.push_back(r);
//			}
		}
	}

	/**
	  * This function is called whenever a query
	  * fails.
	 **/
	void localFail(const std::string &message)
	{
		success *= -1;
		const bool wasEmpty = errorMessage.empty();
		errorMessage = message + (wasEmpty ? "" : ", ") + errorMessage;
	}

	/**
	  * Indicates whether the query was executed
	  * successfully on the local server
	 **/
	bool localSuccess() const
	{
		return success > 0;
	}

	/**
	  * Returns whether the query was executed
	  * successfully on the remote servers
	 **/
	bool remoteSuccess() const
	{
		return abs(success) == 1;
	}

	/**
	  * Returns whether the query was executed successfully
	 **/
	bool wasSuccess() const
	{
		return success == 1;
	}

	/**
	  * Returns the amount of failed servers
	 **/
	unsigned int getAmountFailed() const
	{
		return abs(success) - 1;
	}

	/**
	  * Returns the error message of the result
	 **/
	std::string getErrorMessage() const
	{
		return errorMessage;
	}

	/**
	  * Sets the IndexIterator as result
	 **/
	void addResult(const IndexIterator &it)
	{
		this->results.push_back(std::pair<unsigned int,IndexIterator>(0, it));
	}

	/**
	  * Returns whether there is a result to fetch
	 **/
	bool hasResult() const
	{
		for(const auto &res : results)
		{
			if(!res.second.finished)return true;
		}
		return false;
	}

	/**
	  * Returns the amount of cols per row
	 **/
	std::size_t colsCount() const
	{
		if(results.empty())return 0;
		return results.front().second.columns.size();
	}

	/**
	  * Fetches the next row from the result
	 **/
	bool fetchRow(Database &db, std::vector<DataValue> &out);

	friend bool operator>> <>(const Package &p, SQLResult &q);
	friend void operator<< <>(Package &p, const SQLResult &q);

private:
	/**
	  * Whether the query was executed successfully
	 **/
	int success;

	/**
	  * Contains a error message if the query was
	  * not executed successfully
	 **/
	std::string errorMessage;

	/**
	  * The result of the query
	 **/
	std::list<std::pair<unsigned int,IndexIterator> > results;

	/**
	  * This list stores the values which are already retrieved
	 **/
	std::list<std::vector<DataValue> > valuesUsed;

}; //end class SQLResult

/**
  * This function is overloaded from the Package class
  * to retrieve a SQLResult from a Package
 **/
template <>
inline bool operator>>(const Package &p, SQLResult &r)
{
	if(!(p>>r.success))return false;
	if(!(p>>r.errorMessage))return false;
	if(!(p>>r.results))return false;
	return true;
}


/**
  * This function is overloaded from the Package class
  * to insert a SQLResult into a Package
 **/
template <>
inline void operator<<(Package &p, const SQLResult &r)
{
	p<<r.success;
	p<<r.errorMessage;
	p<<r.results;
}

} //end namespace cluster

#endif //SQLRESULT_HPP
