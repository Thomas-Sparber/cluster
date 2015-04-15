/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/database.hpp>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace cluster;

Database::Database(ClusterObject *network, unsigned int ui_maxPackagesToRemember) :
	ClusterObjectSerialized(network, ui_maxPackagesToRemember),
	tables(),
	databaseMutex()
{}

Database::~Database()
{
	for(Table *t : tables)delete t;
}

bool Database::perform(const Package &message)
{
	SQLQuery q;

	//Extracting all packages
	databaseMutex.lock();
	while(message>>q)
		perform(q, nullptr);
	databaseMutex.unlock();

	return true;
}

void Database::getRebuildPackage(Package &out)
{
	//Adding every table to the package
	for(const Table *t : tables)
	{
		out<<t->getName();
		out<<(*t);
	}
}

void Database::rebuild(const Package &out)
{
	databaseMutex.lock();
	for(Table *t : tables)delete t;
	tables.clear();
	string name;
	while(out>>name)
	{
		Table *t = new Table(name);
		if(!(out>>(*t)))break;
		tables.push_back(t);
	}
	databaseMutex.unlock();
}

void Database::doAndSend(SQLQuery query, SQLResult *result)
{
	if(!query.createQueryTree(*this, result))return;

	databaseMutex.lock();

	//Used to execute queries concurrently
	thread t([this,query,&result] ()
	{
		perform(query, result);
	});

	AnswerPackage answer;
	while(!send(query, &answer))
		usleep(1000);
	t.join();
	databaseMutex.unlock();

	if(result)
	{
		SQLResult res;
		for(const auto &package : answer)
		{
			const Address *address = package.first;
			const Package &pkg = package.second;

			if(pkg>>res)result->add(res, *address);
		}
	}
}

void Database::perform(const SQLQuery &query, SQLResult *result)
{
	query.execute(*this, result);
}
