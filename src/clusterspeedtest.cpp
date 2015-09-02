/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/clusterspeedtest.hpp>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

using namespace std;
using namespace cluster;

ClusterSpeedTest::ClusterSpeedTest(ClusterObject *network, unsigned int ui_dataSize) :
	ClusterObject(network),
	dataSize(ui_dataSize),
	packagesSent(),
	mapMutex(),
	running(false),
	t(nullptr)
{
	addMemberCallback(this);
	srand((unsigned int)time(nullptr));
}

/*ClusterSpeedTest::ClusterSpeedTest(ClusterSpeedTest &t) :
	ClusterObject(t.network),
	dataSize(t.dataSize),
	packagesSent(t.packagesSent),
	mapMutex(),
	running(false),
	t(nullptr)
{
	if(isRunning())start();
}*/

ClusterSpeedTest::~ClusterSpeedTest()
{
	removeMemberCallback(this);
	if(isRunning())stop();

	mapMutex.lock();
	for(auto it : packagesSent)
	{
		delete it.first;
	}
	mapMutex.unlock();
}

/*ClusterSpeedTest& ClusterSpeedTest::operator= (ClusterSpeedTest &t)
{

}*/

void ClusterSpeedTest::memberOnline(const Address &ip, bool /*isMaster*/)
{
	mapMutex.lock();
	packagesSent.push_back(pair<Address*,list<double> >(ip.clone(),list<double>()));
	mapMutex.unlock();
}

void ClusterSpeedTest::memberOffline(const Address &ip)
{
	mapMutex.lock();
	for(auto it = packagesSent.begin(); it != packagesSent.end(); ++it)
	{
		if(*it->first == ip)
		{
			delete it->first;
			packagesSent.erase(it);
			break;
		}
	}
	mapMutex.unlock();
}

void ClusterSpeedTest::fillRandomPackage(Package &p, unsigned int length) const
{
	for(unsigned int i = 0; i < length; ++i)
	{
		p<<(char)('A' + (rand()%26));
	}
}

bool ClusterSpeedTest::received(const Address &/*ip*/, const Package &/*message*/, Package &answer, Package &/*to_send*/)
{
	fillRandomPackage(answer, dataSize);
	return true;
}

void ClusterSpeedTest::sendThread()
{
	while(running)
	{
		//bool worked = false;

		mapMutex.lock();
		for(auto &it : packagesSent)
		{
			//worked = true;

			Package pkg;
			Package answer;
			struct timeval t1, t2;
			fillRandomPackage(pkg, dataSize);

			gettimeofday(&t1, nullptr);
			askPackage(*it.first, pkg, &answer);
			gettimeofday(&t2, nullptr);

			const double time = (t2.tv_sec-t1.tv_sec) + (double)((t2.tv_usec-t1.tv_usec)/1000000.0);
			const double size = dataSize + answer.getLength();
			it.second.push_back(size / time);
		}
		mapMutex.unlock();

		/*if(!worked)*/usleep(100000);
	}
}

void ClusterSpeedTest::printStatistics() const
{
	mapMutex.lock();
	for(auto it : packagesSent)
	{
		if(it.second.empty())
		{
			cout<<"No data for "<<it.first->address<<endl;
			continue;
		}

		double min = it.second.front();
		double max = it.second.front();
		double avg = 0;
		for(double val : it.second)
		{
			min = std::min(min, val);
			max = std::max(max, val);
			avg += val;
		}
		avg /= it.second.size();

		cout<<it.first->address<<": AVG: "<<avg<<", MIN: "<<min<<", MAX: "<<max<<endl;
	}
	mapMutex.unlock();
}
