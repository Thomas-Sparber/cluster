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

int main(int /*argc*/, char* /*args*/[])
{
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

	//thread t(controller, &v);
	srandom(unsigned(time(nullptr)));
	while(running)
	{
		m.lock();	//TODO only works with members

		int number = 0;
		if(v.size() != 0)number = v.get(v.size()-1);
		number++;

		v.add(number);

		if(v.size() > 2)
		{
			const int a = v.get(v.size()-3);
			const int b = v.get(v.size()-2);

			if(a > b)cout<<"Error: "<<a<<", "<<b<<endl;
		}

		m.unlock();

		usleep(random() % 1000000);
	}
	cout<<"Quitting"<<endl;
	//t.join();

	network.close();
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
