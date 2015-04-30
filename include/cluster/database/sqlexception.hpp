/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SQLEXCEPTION_HPP
#define SQLEXCEPTION_HPP

#include <string>

namespace cluster
{

class SQLException
{

public:
	/**
	  * Constructs the SQLException using the
	  * given description text
	 **/
	SQLException(const std::string &str_text) :
		text(str_text)
	{}

	/**
	  * The description text
	 **/
	std::string text;

}; //end class SQLException

} //end namespace cluster

#endif //SQLEXCEPTION_HPP
