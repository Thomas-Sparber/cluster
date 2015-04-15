/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <cluster/clusterobjectserialized.hpp>
#include <cluster/database/sqlquery.hpp>
#include <cluster/database/sqlresult.hpp>
#include <cluster/database/table.hpp>
#include <vector>
#include <mutex>
#include <string>

namespace cluster
{

class SQLQueryElement;

/**
  * 
 **/
class Database : public ClusterObjectSerialized
{

public:
	/**
	  * Default constructor which creates the
	  * Database and adds it to the given network
	 **/
	Database(ClusterObject *network, unsigned int maxPackagesToRemember=100);

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
	SQLResult executeQuery(const std::string &query)
	{
		return executeQuery(SQLQuery(query));
	}

	/**
	  * Executes the given query
	 **/
	SQLResult execute(const SQLQuery &query)
	{
		SQLResult res;
		doAndSend(query, &res);
		return res;
	}

	/**
	  * Executes the given query
	 **/
	SQLResult executeQuery(const SQLQuery &query)
	{
		SQLResult res;
		doAndSend(query, &res);
		return res;
	}

	bool tableExists(const std::string &name) const
	{
		return (getTable(name) != nullptr);
	}

	/**
	  * Creates the table with the given name
	 **/
	Table* createTable(const std::string &name)
	{
		Table *t = new Table(name);
		tables.push_back(t);
		return t;
	}

	/**
	  * Returns the table with the given name
	 **/
	Table* getTable(const std::string &name)
	{
		for(Table *t : tables)
		{
			if(t->getName() == name)return t;
		}
		return nullptr;
	}

	/**
	  * Returns the table with the given name
	 **/
	const Table* getTable(const std::string &name) const
	{
		for(Table *t : tables)
		{
			if(t->getName() == name)return t;
		}
		return nullptr;
	}

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const override
	{
		return "Database";
	}

protected:
	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a package was missed
	  * and needs to be performed.
	 **/
	virtual bool perform(const Package &message) override;

	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a member needs to
	  * rebuild and needs the data. The given package is filled
	  * with the data so that the rebuild function can rebuild the
	  * entire structure using this package.
	 **/
	virtual void getRebuildPackage(Package &out) override;

	/**
	  * Ovedrrides the function from ClusterObjectSerialized.
	  * This function is called whenever the structure needs
	  * to be rebuilt entirely.
	 **/
	virtual void rebuild(const Package &out) override;

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
	  * Performs the given operation by first sending it to the
	  * network, waiting for approvement and then performing
	  * it locally.
	 **/
	void doAndSend(SQLQuery query, SQLResult *result);

	/**
	  * Performs the given operation locally. This function
	  * is called by doAndSend and by received.
	 **/
	void perform(const SQLQuery &query, SQLResult *result);

private:
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
