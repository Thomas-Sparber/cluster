/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/ipv4/ipv4.hpp>
#include <cluster/ipv6/ipv6.hpp>
#include <cluster/package.hpp>
#include <cluster/p2p.hpp>
#include <cluster/clustercontainer.hpp>
#include <cluster/clustermutex.hpp>
#include <cluster/database/database.hpp>
#include <cluster/clusterspeedtest.hpp>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/time.h>

using namespace std;
using namespace cluster;

void testDatabase(const string &ip1, const string &ip2);
void testClusterContainer(const string &ip1, const string &ip2);
void testSpeed(const string &ip1, const string &ip2);
void signalHandler(int signal);
template<class Index, class Container> void controller(const ClusterContainer<Index, int, Container> *c);

bool running = true;

int main(int /*argc*/, char* /*args*/[])
{
	signal(SIGINT, signalHandler);

	string ip1, ip2;
	ifstream in("addresses.txt");
	in>>ip1;
	in>>ip2;

	//testClusterContainer(ip1, ip2);
	testDatabase(ip1, ip2);
	//testSpeed(ip1, ip2);
}

void testDatabase(const string &ip1, const string &ip2)
{
	IPv4 p(1234);
	p2p network(p);
	if(!ip1.empty() && !ip2.empty())network.addAddressRange(IPv4Address(ip1), IPv4Address(ip2));
	Database db(&network, "test", 2, 2);

	cout<<"Network structure:"<<endl<<network.getWholeStructure();

	string input;
	while(running)
	{
		cout<<"sql> "<<flush;
		getline(cin, input);

		if(input == "quit")
		{
			running = false;
			break;
		}

		if(!input.empty())
		{
			struct timeval t1;
			struct timeval t2;
			gettimeofday(&t1, nullptr);
			SQLResult res = db.execute(input);
			gettimeofday(&t2, nullptr);
			if(res.wasSuccess())
			{
				if(res.hasResult())
				{
					vector<DataValue> row(res.colsCount());
					while(res.fetchRow(db, row))
					{
						bool first = true;
						for(unsigned int i = 0; i < row.size(); ++i)
						{
							if(first)first = false;
							else cout<<", ";
							cout<<row[i].toString();
						}
						cout<<endl;
					}
				}
				const double time = (t2.tv_sec-t1.tv_sec) + double(t2.tv_usec-t1.tv_usec)/1000000;
				cout<<"Query executed successfully: "<<time<<" s"<<endl;
			}
			else
			{
				cout<<res.getErrorMessage()<<endl;
			}
		}
	}
}

void testClusterContainer(const string &ip1, const string &ip2)
{
	IPv4 p(1234);
	p2p network(p);
	if(!ip1.empty() && !ip2.empty())network.addAddressRange(IPv4Address(ip1), IPv4Address(ip2));
	ClusterList<int> v(&network);
	ClusterMutex m(&network);

	cout<<"Network structure:"<<endl<<network.getWholeStructure();

	//thread t(controller, &v);
	srand(unsigned(time(nullptr)));
	while(running)
	{
		m.lock();

		int number = 0;
		if(v.size() != 0)number = (int)v.get(v.size()-1);
		number++;

		//cout<<"Main: adding "<<number<<endl;
		v.add(number);

		if(v.size() > 2)
		{
			const int a = v.get(v.size()-3);
			const int b = v.get(v.size()-2);

			if(a > b)cout<<"Error: "<<a<<", "<<b<<endl;
		}

		m.unlock();

		usleep(rand() % 10000);
	}
	//t.join();

	network.close();
}

void testSpeed(const string &ip1, const string &ip2)
{
	IPv4 p(1234);
	p2p network(p);
	if(!ip1.empty() && !ip2.empty())network.addAddressRange(IPv4Address(ip1), IPv4Address(ip2));
	ClusterSpeedTest test(&network, 10000);

	cout<<"Network structure:"<<endl<<network.getWholeStructure();

	test.start();
	while(running)
	{
		test.printStatistics();
		sleep(5);
	}
	test.stop();

	network.close();
}

template<class Index, class Container> void controller(const ClusterContainer<Index, int, Container> *c)
{
	while(running)
	{
		if(c->size() != 0)
		{
			const int a = c->get(c->size()-1);
			cout<<a<<endl;
		}
		sleep(2);
	}
	running = false;
}

void signalHandler(int signal)
{
	static unsigned int counter = 0;

	switch(signal)
	{
	case SIGINT:
		running = false;
		if(counter++)exit(0);
		break;
	default:
		cout<<"Signal "<<signal<<" not defined."<<endl;
	}
}
