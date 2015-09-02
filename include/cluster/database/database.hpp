/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <cluster/clusterobjectdistributed.hpp>
#include <cluster/database/sqlquery.hpp>
#include <cluster/database/sqlresult.hpp>
#include <cluster/database/table.hpp>
#include <vector>
#include <list>
#include <mutex>
#include <string>

namespace cluster
{

class SQLQueryElement;
class SQLQuery_insertInto;
class SQLFetchResult;

/**
  * 
 **/
class Database : public ClusterObjectDistributed
{

public:
	/**
	  * Default constructor which creates the
	  * Database and adds it to the given network
	 **/
	Database(ClusterObject *network, const std::string &name, unsigned int dataRedundancy, unsigned int takeOverSize, unsigned int maxPackagesToRemember=100);

	/**
	  * Default destructor
	 **/
	virtual ~Database();

	/**
	  * Executes the given query
	 **/
	SQLResult execute(const std::string &query)
	{
		return execute(SQLQuery(query));
	}

	/**
	  * Executes the given query
	 **/
	SQLResult execute(SQLQuery query);

	bool tableExists(const std::string &tableName) const
	{
		return (getTable(tableName) != nullptr);
	}

	/**
	  * Creates the table with the given name
	 **/
	Table* createTable(const std::string &tableName, SQLResult *result, bool isCoordinator, const SQLQuery &q);

	/**
	  * Returns the table with the given name
	 **/
	Table* getTable(const std::string &tableName)
	{
		for(Table *t : tables)
		{
			if(t->getName() == tableName)return t;
		}
		return nullptr;
	}

	/**
	  * Returns the table with the given name
	 **/
	const Table* getTable(const std::string &tableName) const
	{
		for(Table *t : tables)
		{
			if(t->getName() == tableName)return t;
		}
		return nullptr;
	}

	/**
	  * Creates the IndexIterator to select all rows from the
	  * given table
	 **/
	void select(const std::string &table, const std::vector<Column> &cols, SQLResult *result, bool isCoordinator, const SQLQuery &q);

	/**
	  * Selects the next row from the iterator
	 **/
	void selectNext(std::size_t onlineClient, IndexIterator &it, SQLFetchResult &out);

	/**
	  * Insertd the given data into the given table
	 **/
	void insert(const std::string &table, const std::vector<std::string> &cols, const std::vector<std::string> &values, SQLResult *result, bool isCoordinator, const SQLQuery &q);

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const override
	{
		return "Database";
	}

protected:
	/**
	  * Overrides the function from ClusterObjectDistributed.
	  * This function is called whenever a command
	  * package needs to be performed.
	 **/
	virtual bool performCommand(const Package &message, Package &answer, Package &toSend) override;

	/**
	  * Overrides the function from ClusterObjectDistributed.
	  * This function is called whenever a data
	  * package needs to be performed.
	 **/
	virtual bool performInsert(const Package &message, std::string &idOut, std::string &error) override;

	/**
	  * Overrides the function from ClusterObjectDistributed.
	  * This function is called whenever some data need to
	  * be fetched
	 **/
	virtual bool performFetch(const std::string &id, Package &answer) override;

	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a member needs to
	  * rebuild and needs the data. The given package is filled
	  * with the data so that the rebuild function can rebuild the
	  * entire structure using this package.
	 **/
	virtual void getRebuildPackage(Package &out) const override;

	/**
	  * Ovedrrides the function from ClusterObjectSerialized.
	  * This function is called whenever the structure needs
	  * to be rebuilt entirely.
	 **/
	virtual void rebuild(const Package &out, const Address &address) override;

private:
	/**
	  * Copying a database is illegal because
	  * of the tables which have open files
	 **/
	Database(const Database &d);

	/**
	  * Copying a database is illegal because
	  * of the tables which have open files
	 **/
	Database& operator= (const Database &d);

	/**
	  * Sends the given query to the network
	  * and saves the response in result. The mutex
	  * is used for synchronized access to the result
	 **/
	void sendToNetwork(const SQLQuery &query, std::mutex *m, SQLResult *result);

	/**
	  * Sends the given query to the given address
	  * and saves the response in result. The mutex
	  * is used for synchronized access to the result
	 **/
	void sendToNetwork(const Address &address, const SQLQuery &query, std::mutex *m, SQLResult *result);

	/**
	  * Sends the given query to the given address
	  * and saves the response in result. The mutex
	  * is used for synchronized access to the result
	 **/
	void sendToNetwork(std::size_t clientIndex, const SQLQuery &query, std::mutex *m, SQLResult *result);

	/**
	  * Sends the given IndexIterator to the given address
	  * and saves the response in result. The mutex
	  * is used for synchronized access to the result
	 **/
	void sendToNetwork(const Address &address, IndexIterator &it, std::mutex *m, std::list<SQLFetchResult> &result);

	/**
	  * Sends the given IndexIterator to the given address
	  * and saves the response in result. The mutex
	  * is used for synchronized access to the result
	 **/
	void sendToNetwork(std::size_t clientIndex, IndexIterator &it, std::mutex *m, std::list<SQLFetchResult> &result);

private:
	/**
	  * The name of the database
	 **/
	std::string name;

	/**
	  * This vector stores the table data
	 **/
	std::vector<Table*> tables;

	/**
	  * This mutex is used to synchronize access
	  * to the table data
	 **/
	std::mutex databaseMutex;

}; // end class Database

} //end namespace cluster

#endif //DATABASE_HPP
