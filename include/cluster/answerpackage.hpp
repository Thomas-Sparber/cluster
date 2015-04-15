/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef ANSWERPACKAGE_HPP
#define ANSWERPACKAGE_HPP

#include <cluster/prototypes/address.hpp>
#include <cluster/package.hpp>
#include <list>

namespace cluster
{

/**
  * Is class is used to store the answer of a
  * cluster network. It contains the addresses
  * and the corresponding answer packages
 **/
class AnswerPackage
{

public:
	AnswerPackage() :
		packages()
	{}

	~AnswerPackage()
	{
		for(auto pkg : packages)
			delete pkg.first;
	}

	void add(const Address &a, const Package &pkg)
	{
		packages.push_back(std::pair<const Address*, const Package>(a.clone(), pkg));
	}

	const std::list<std::pair<const Address*, const Package> >& getPackages() const
	{
		return packages;
	}

	std::list<std::pair<const Address*, const Package> >::const_iterator cbegin() const
	{
		return packages.begin();
	}

	std::list<std::pair<const Address*, const Package> >::const_iterator cend() const
	{
		return packages.end();
	}

	std::list<std::pair<const Address*, const Package> >::const_iterator begin() const
	{
		return packages.begin();
	}

	std::list<std::pair<const Address*, const Package> >::const_iterator end() const
	{
		return packages.end();
	}

	std::size_t size() const
	{
		return packages.size();
	}

	bool empty() const
	{
		return packages.empty();
	}

private:
	std::list<std::pair<const Address*, const Package> > packages;

}; //end class AnswerPackage

} //end namespace cluster

#endif
