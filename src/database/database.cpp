/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/database.hpp>
#include <cluster/database/sqlfetchresult.hpp>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef __linux__
#include <direct.h>
#endif //__linux__

using namespace std;
using namespace cluster;

const string current_version("1.0.0");

namespace cluster
{

/**
  * This enum defines the actions of the
  * Database
 **/
enum class DatabaseOperation : char
{
	/**
	  * Defines that the package is a SQLQuery
	 **/
	query = 'q',

	/**
	  * Defines that the package is used to select data
	 **/
	select_next = 'n'

}; //end enum DatabaseOperation

/**
  * This function is overloaded from the Package class
  * to retrieve a DatabaseOperation from a Package
 **/
template <>
inline bool operator>>(const Package &p, DatabaseOperation &t)
{
	return p>>reinterpret_cast<char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a DatabaseOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const DatabaseOperation &t)
{
	p<<reinterpret_cast<const char&>(t);
}

} //end namespace cluster

/******************************************************/

void Database::sendToNetwork(const SQLQuery &query, mutex *m, SQLResult *result)
{
	AnswerPackage answer;
	while(!send(DatabaseOperation::query, query, &answer))
		usleep(1000);

	if(result)
	{
		SQLResult res;
		for(const auto &package : answer)
		{
			const Address *address = package.first;
			const Package &pkg = package.second;

			if(m)m->lock();
			if(pkg>>res)result->add(res, *address, getOnlineClientId(*address));
			if(m)m->unlock();
		}
	}
}

void Database::sendToNetwork(const Address &address, const SQLQuery &query, mutex *m, SQLResult *result)
{
	Package answer;
	while(!ask(address, DatabaseOperation::query, query, &answer))
		usleep(1000);

	if(result)
	{
		SQLResult res;

		if(m)m->lock();
		if(answer>>res)result->add(res, address, getOnlineClientId(address));
		if(m)m->unlock();
	}
}

void Database::sendToNetwork(std::size_t clientIndex, const SQLQuery &query, mutex *m, SQLResult *result)
{
	const Address *clientAddress = getOnlineClientAddress(clientIndex);
	if(!clientAddress)return;

	sendToNetwork(*clientAddress, query, m, result);
}

void Database::sendToNetwork(const Address &address, IndexIterator &it, mutex *m, list<SQLFetchResult> &result)
{
	Package answer;
	while(!ask(address, DatabaseOperation::select_next, it, &answer))
		usleep(1000);

	SQLFetchResult res;

	if(!(answer>>it))return;

	if(m)m->lock();
	while(answer>>res)result.push_back(res);
	if(m)m->unlock();
}

void Database::sendToNetwork(std::size_t clientIndex, IndexIterator &it, mutex *m, list<SQLFetchResult> &result)
{
	const Address *clientAddress = getOnlineClientAddress(clientIndex);
	if(!clientAddress)return;

	sendToNetwork(*clientAddress, it, m, result);
}

/******************************************************/

Database::Database(ClusterObject *network, const string &str_name, unsigned int ui_dataRedundancy, unsigned int ui_takeOverSize, unsigned int ui_maxPackagesToRemember) :
	ClusterObjectDistributed(network, ui_dataRedundancy, ui_takeOverSize, true, ui_maxPackagesToRemember),
	name(str_name),
	tables(),
	databaseMutex()
{
	const string fileName = string("databases/") + name + ".db";
	ifstream in(fileName);
#ifdef __linux__
	mkdir((string("databases/") + name).c_str(), 0750);
#else
	_mkdir((string("databases/") + name).c_str());
#endif //__linux__

	string temp;
	if(in>>temp)
	{
		if(temp != current_version)throw SQLException(string("Unsupported Database version: ") + temp + ". Current version is " + current_version);

		//Read tables
		while(in>>temp)
		{
			Table *t = new Table(temp, string("databases/") + name);
			tables.push_back(t);

			if(const Index *i = t->getPrimaryKey())
			{
				list<string> ids;
				for(auto it = i->begin(); it != i->end(); ++it)
				{
					string id = temp;
					for(const DataValue &v : it->first.data)
					{
						id += ",";
						id += v.toString();
					}
					ids.push_back(id);
				}
				setInitialIds(ids);
			}
			else cout<<"Error: table "<<temp<<" has no primary key"<<endl;
		}
	}
	else
	{
		//Create database file
		ofstream out(fileName);
		out<<current_version<<endl;
		out.close();
	}
}

Database::~Database()
{
	for(Table *t : tables)delete t;
}

bool Database::performCommand(const Package &message, Package &answer, Package &/*toSend*/)
{
	bool success = false;
	DatabaseOperation operation;

	//Extracting all packages
	while(message>>operation)
	{
		switch(operation)
		{
		case DatabaseOperation::query: {
			SQLQuery q;
			SQLResult r;
			if(!(message>>q))return false;
			q.execute(*this, &r, false);
			answer<<r;
			success = true;
			break;
		}
		case DatabaseOperation::select_next: {
			IndexIterator it;
			if(!(message>>it))return false;
			SQLFetchResult r(it.columns.size());
			selectNext(0, it, r);
			answer<<it;
			answer<<r;
			success = true;
			break;
		}
		default:
			break;
		}
	}

	return success;
}

bool Database::performInsert(const Package &message, string &idOut, string &error)
{
	SQLTableFetchResult fr;

	//Extract fetch result
	if(!(message>>fr))
	{
		error = "Unable to get fetchResult";
		return false;
	}

	Table *table = getTable(fr.table);
	if(!table)
	{
		error = string("Table ")+fr.table+" does not exist";
		return false;
	}

	try {
		const vector<DataValue> &key = table->insert(fr.data);
		idOut = table->getName();
		for(const DataValue &v : key)
		{
			idOut += ",";
			idOut += v.toString();
		}
	} catch(const SQLException &ex) {
		error = ex.text;
		return false;
	}

	cout<<"Inserting "<<fr.table;
	for(unsigned int i = 0; i < fr.data.size(); ++i)
	{
		cout<<", ";
		cout<<fr.data[i].toString();
	}
	cout<<endl;
	
	return true;
}

bool Database::performFetch(const std::string &id, Package &answer)
{
	stringstream ss(id);
	SQLTableFetchResult fr;

	//Extract table name
	if(!getline(ss, fr.table, ','))return false;

	const Table *table = getTable(fr.table);
	if(!table)return false;

	const Index *index = table->getPrimaryKey();
	if(!index)return false;

	string item;
	vector<DataValue> indexValues(index->getColumns().size());
	for(std::size_t i = 0; i < index->getColumns().size(); ++i)
	{
		//Get current column for primary key
		const Column &c = table->getColumns()[(std::size_t)index->getColumns()[i]];

		//Get next primary key value
		if(!getline(ss, item, ','))return false;

		//Set data type
		indexValues[i] = c.type;

		//Set value
		indexValues[i].setValue(item);
	}

	//Fetch result from table
	fr.data.resize(table->getColumns().size());
	try {
		table->select(index->get(indexValues)->second, &fr.data[0]);
	} catch(const SQLException &ex) {
		return false;
	}

	//Pack result into Package
	answer<<fr;
	return true;
}

void Database::getRebuildPackage(Package &out) const
{
	ClusterObjectDistributed::getRebuildPackage(out);

	//Adding every table to the package
	for(const Table *t : tables)
	{
		out<<t->getName();
		out<<(*t);
	}
}

void Database::rebuild(const Package &out, const Address &address)
{
	ClusterObjectDistributed::rebuild(out, address);

	databaseMutex.lock();
	for(unsigned int i = 0; i < tables.size(); ++i)
		delete tables[i];
	tables.clear();

	string tableName;
	while(out>>tableName)
	{
		Table *t = new Table(tableName, string("databases/") + name);
		if(!(out>>(*t)))break;
		tables.push_back(t);
	}
	databaseMutex.unlock();
}


SQLResult Database::execute(SQLQuery query)
{
	SQLResult res;

	if(!query.createQueryTree(*this, &res))return res;
	query.execute(*this, &res, true);

	return res;
}

void Database::select(const string &table, const vector<Column> &cols, SQLResult *result, bool isCoordinator, const SQLQuery &q)
{
	mutex m;
	if(isCoordinator)
	{
		thread th([this,q,&result,&m] () { sendToNetwork(q, &m, result); });
		select(table, cols, result, false, q);
		th.join();
	}
	else
	{
		const Table *t = getTable(table);
		if(!t)throw SQLException(string("Table ")+table+" does not exist");

		IndexIterator it;
		it.table = table;
		t->select(cols, it);

		m.lock();
		if(result)result->addResult(it);
		m.unlock();
	}
}

Table* Database::createTable(const string &tableName, SQLResult *result, bool isCoordinator, const SQLQuery &q)
{
	if(isCoordinator)
	{
		mutex m;
		thread th([this,q,&result,&m] () { sendToNetwork(q, &m, result); });
		Table *t = createTable(tableName, result, false, q);
		th.join();
		return t;
	}
	else
	{
		const string fileName = string("databases/") + name + ".db";
		ofstream out(fileName, ios_base::app);
		out<<tableName<<endl;
		out.close();

		Table *t = new Table(tableName, string("databases/") + name);
		tables.push_back(t);

		return t;
	}
}

void Database::selectNext(std::size_t onlineClient, IndexIterator &it, SQLFetchResult &out)
{
	if(onlineClient == 0)
	{
		//Local select
		if(const Table *t = getTable(it.table))
		{
			out.data.resize(it.columns.size());
			t->selectNext(it, &out.data[0], out.key);
		}
		else throw SQLException(string("Table ") + it.table + " does not exist");
	}
	else
	{
		list<SQLFetchResult> result;
		sendToNetwork(onlineClient, it, nullptr, result);
		if(result.empty())SQLException("Got no response from client");
		out = result.front();
	}
}

void Database::insert(const std::string &table, const vector<string> &cols, const vector<string> &values, SQLResult *result, bool /*isCoordinator*/, const SQLQuery &/*q*/)
{
	Table *t = getTable(table);
	vector<DataValue> dataValues;

	list<Package> packagesToInsert;
	
	for(std::size_t i = 0; i < values.size(); i += t->getColumns().size())
	{
		t->getInsertValues(cols, &values[i], dataValues);
		SQLTableFetchResult fr(table, dataValues);

		//Create insert package
		Package pkg;
		pkg<<fr;
		packagesToInsert.push_back(pkg);
	}

	string error;
	ClusterObjectDistributed::insertData(packagesToInsert, error);
	if(!error.empty())
	{
		if(result)result->localFail(error);
		return;
	}
}
