/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/sqlresult.hpp>

using namespace std;
using namespace cluster;

SQLResult::SQLResult() :
	success(1),
	errorMessage()
{}

SQLResult::~SQLResult()
{}
