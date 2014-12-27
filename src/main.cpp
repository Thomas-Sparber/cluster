#include <cluster/ipv4/ipv4.hpp>
#include <cluster/ipv6/ipv6.hpp>
#include <cluster/package.hpp>
#include <cluster/p2p.hpp>
#include <cluster/clustercontainer.hpp>
#include <cluster/clustermutex.hpp>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cluster;

void signalHandler(int signal);
void controller(const ClusterContainer<int> *c);

bool running = true;

int main(int __attribute__((__unused__)) argc, char __attribute__((__unused__)) *args[])
{
	signal(SIGABRT, signalHandler);
	signal(SIGINT, signalHandler);

	string ip1, ip2;
	ifstream in("addresses.txt");
	in>>ip1;
	in>>ip2;

	IPv4 p(1234);
	p2p network(p);
	if(ip1.size() && ip2.size())network.addAddressRange(IPv4Address(ip1), IPv4Address(ip2));
	ClusterContainer<int> v(&network);
	ClusterMutex m(&network);

	cout<<"Network structure:"<<endl;
	network.printWholeStructure();

	thread t(controller, &v);

	while(running)
	{
		if(network.getMembersCount() > 1)
		{
			m.lock();

			int number = 0;
			if(v.size() != 0)number = v.get(v.size()-1);
			number++;

			v.add(number);

			if(v.size() > 2)
			{
				const int a = v.get(v.size()-3);
				const int b = v.get(v.size()-2);
				if(a > b)
				{
					cout<<"Error: "<<a<<", "<<b<<endl;
//					((char*)nullptr)[1] = 'c';
				}
			}

			m.unlock();
		}
		else sleep(1);
	}
	t.join();
}

void controller(const ClusterContainer<int> *c)
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
	switch(signal)
	{
	case SIGABRT:
		((int*)nullptr)[0]++;
		break;
	case SIGINT:
		running = false;
		break;
	default:
		cout<<"Signal "<<signal<<" not defined."<<endl;
	}
}
