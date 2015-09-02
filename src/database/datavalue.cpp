/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/datavalue.hpp>

using namespace std;
using namespace cluster;

/**
  * Converts a date represented by 3 ints
  * into a long value that represents the days
 **/
inline long convertDateToDays(uint16_t y, uint8_t m, uint8_t d)
{
	m = uint8_t((m + 9) % 12);
	y = uint16_t(y - m/10);
	return 365l*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + (d-1);
}

/**
  * Converts a date represented by a long value
  * into three ints
 **/
inline void convertDaysToDate(long g, uint16_t &y, uint8_t &m, uint8_t &d)
{
	long yy = (10000*g + 14780)/3652425;
	long ddd = g - (365*yy + yy/4 - yy/100 + yy/400);
	if(ddd < 0)
	{
		yy = yy - 1;
		ddd = g - (365*yy + yy/4 - yy/100 + yy/400);
	}
	long mi = (100*ddd + 52)/3060;

	m = uint8_t((mi + 2)%12 + 1);
	y = uint16_t(yy + (mi + 2)/12);
	d = uint8_t(ddd - (mi*306 + 5)/10 + 1);
}

/**
  * Converts a datetime represented by 6 ints
  * into a long value that represents the seconds
 **/
inline long convertDateTimeToSeconds(uint16_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t min, uint8_t s)
{
	m = uint8_t((m + 9) % 12);
	y = uint16_t(y - m/10);
	return (365l*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + (d-1)) * 86400 + h*3600 + min*60 + s;
}

/**
  * Converts a datetime represented by a long value
  * into six ints
 **/
inline void convertSecondsToDateTime(long g, uint16_t &y, uint8_t &m, uint8_t &d, uint8_t &h, uint8_t &min, uint8_t &s)
{
	s = uint8_t(g % 60);
	g = g / 60;
	min = uint8_t(g % 60);
	g = g / 60;
	h = uint8_t(g % 24);
	g = g / 24;

	long yy = (10000*g + 14780)/3652425;
	long ddd = g - (365*yy + yy/4 - yy/100 + yy/400);
	if(ddd < 0)
	{
		yy = yy - 1;
		ddd = g - (365*yy + yy/4 - yy/100 + yy/400);
	}
	long mi = (100*ddd + 52)/3060;

	m = uint8_t((mi + 2)%12 + 1);
	y = uint16_t(yy + (mi + 2)/12);
	d = uint8_t(ddd - (mi*306 + 5)/10 + 1);
}

/**
  * Extracts the length parameter from the given string
 **/
inline unsigned int extractLength(const std::string &t, unsigned int currentPosition, unsigned int defaultValue)
{
	if(t.length() > currentPosition)
	{
		while(t.length() > currentPosition && isspace(t[currentPosition]))++currentPosition;

		if(t[currentPosition] != '(')throw SQLException(std::string("Unexpected character ") + t[currentPosition]);
		if(t[t.length()-1] != ')')throw SQLException(std::string("Unexpected character ") + t[t.length()-1]);
		unsigned int length = atoi(t.substr(currentPosition+1, t.length()-(currentPosition+2)).c_str());
		if(length == 0)throw SQLException("Invalid data type length");
		return length;
	}
	return defaultValue;
}



void DataValue::copyFrom(const DataValue &v)
{
	deleteValue();

	type = v.type;
	length = v.length;

	if(v.value == nullptr)return;

	switch(type)
	{
	case ValueType::c_bit:
		value = new char[(length / 8) + (length % 8 == 0 ? 0 : 1)];
		std::copy(static_cast<const char*>(v.value), static_cast<const char*>(v.value) + (length / 8) + (length % 8 == 0 ? 0 : 1), static_cast<char*>(value));
		break;
	case ValueType::c_year:
		value = new uint8_t();
		(*static_cast<uint8_t*>(value)) = (*static_cast<const uint8_t*>(v.value));
		break;
	case ValueType::c_tinyint:
		value = new int8_t();
		(*static_cast<int8_t*>(value)) = (*static_cast<const int8_t*>(v.value));
		break;
	case ValueType::c_bool:
		value = new bool();
		(*static_cast<bool*>(value)) = (*static_cast<const bool*>(v.value));
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		value = new int16_t();
		(*static_cast<int16_t*>(value)) = (*static_cast<const int16_t*>(v.value));
		break;
	case ValueType::c_mediumint:
		value = new int32_t();
		(*static_cast<int32_t*>(value)) = (*static_cast<const int32_t*>(v.value));
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		value = new int64_t();
		(*static_cast<int64_t*>(value)) = (*static_cast<const int64_t*>(v.value));
		break;
	case ValueType::c_text:
	case ValueType::c_decimal:
		value = new std::string();
		(*static_cast<std::string*>(value)) = (*static_cast<const std::string*>(v.value));
		break;
	case ValueType::c_float:
		value = new float();
		(*static_cast<float*>(value)) = (*static_cast<const float*>(v.value));
		break;
	case ValueType::c_double:
		value = new double();
		(*static_cast<double*>(value)) = (*static_cast<const double*>(v.value));
		break;
	case ValueType::c_date:
		value = new char[4];	//YYMD
		std::copy(static_cast<const char*>(v.value), static_cast<const char*>(v.value) + 4, static_cast<char*>(value));
		break;
	case ValueType::c_datetime:
		value = new char[7];	//YYMDHMS
		std::copy(static_cast<const char*>(v.value), static_cast<const char*>(v.value) + 7, static_cast<char*>(value));
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		value = new char[length];
		std::copy(static_cast<const char*>(v.value), static_cast<const char*>(v.value) + length, static_cast<char*>(value));
		break;
	default:
		throw SQLException("Invalid data type when setting the value");
	}
}

void DataValue::initDefaultValue()
{
	deleteValue();

	switch(type)
	{
	case ValueType::c_bit:
		value = new char[(length / 8) + (length % 8 == 0 ? 0 : 1)]();
		break;
	case ValueType::c_year:
		value = new uint8_t(0);
		break;
	case ValueType::c_tinyint:
		value = new int8_t(0);
		break;
	case ValueType::c_bool:
		value = new bool(false);
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		value = new int16_t(0);
		break;
	case ValueType::c_mediumint:
		value = new int32_t(0);
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		value = new int64_t(0);
		break;
	case ValueType::c_text:
		value = new std::string();
		break;
	case ValueType::c_decimal:
		value = new std::string("0");
		break;
	case ValueType::c_float:
		value = new float(0);
		break;
	case ValueType::c_double:
		value = new double(0);
		break;
	case ValueType::c_date:
		value = new char[4]();	//YYMD
		break;
	case ValueType::c_datetime:
		value = new char[7]();	//YYMDHMS
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		value = new char[length]();
		break;
	default:
		throw SQLException("Invalid data type when creating the default value");
	}
}

std::string DataValue::toString() const
{
	if(isNull())return "null";

	std::stringstream ss;
	switch(type)
	{
	case ValueType::c_bit:
		for(unsigned int i = 0; i < (length / 8) + (length % 8 == 0 ? 0 : 1); ++i)
		{
			ss<<(((static_cast<char*>(value)[i/8] >> i%8) & 1) == 1 ? '1' : '0');
		}
		break;
	case ValueType::c_year:
		if(length == 2)
		{
			ss<<(*static_cast<uint8_t*>(value));
		}
		else
		{
			if((*static_cast<uint8_t*>(value)) == 0)ss<<"0000";
			else ss<<(1900+(*static_cast<uint8_t*>(value)));
		}
		break;
	case ValueType::c_tinyint:
		ss<<(*static_cast<int8_t*>(value));
		break;
	case ValueType::c_bool:
		if((*static_cast<bool*>(value)) == true)ss<<"true";
		else ss<<"false";
		break;
	case ValueType::c_time: {
		const int16_t &t = (*static_cast<int16_t*>(value));
		ss<<(t/3600)<<":"<<std::abs((t/60)%60)<<std::abs(t%60);
		break;
	}
	case ValueType::c_smallint:
		ss<<(*static_cast<int16_t*>(value));
		break;
	case ValueType::c_mediumint:
		ss<<(*static_cast<int32_t*>(value));
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		ss<<(*static_cast<int64_t*>(value));
		break;
	case ValueType::c_text:
	case ValueType::c_decimal:
		return (*static_cast<std::string*>(value));
	case ValueType::c_float:
		ss<<(*static_cast<float*>(value));
		break;
	case ValueType::c_double:
		ss<<(*static_cast<double*>(value));
		break;
	case ValueType::c_date:
		ss<<(*(static_cast<uint16_t*>(value)))<<"-"<<
			(*(static_cast<uint8_t*>(value)+2))<<"-"<<
			(*(static_cast<uint8_t*>(value)+3));
		break;
	case ValueType::c_datetime:
		ss<<(*(static_cast<uint16_t*>(value)))<<"-"<<
			(*(static_cast<uint8_t*>(value)+2))<<"-"<<
			(*(static_cast<uint8_t*>(value)+3))<<" "<<
			(*(static_cast<uint8_t*>(value)+4))<<":"<<
			(*(static_cast<uint8_t*>(value)+5))<<":"<<
			(*(static_cast<uint8_t*>(value)+6));
		break;
	case ValueType::c_char:
		ss.write(static_cast<char*>(value), length);
		break;
	case ValueType::c_binary:
		for(unsigned int i = 0; i < length; ++i)
		{
			ss<<std::hex<<static_cast<char*>(value)[i];
		}
		break;
	default:
		throw SQLException("Invalid data type when setting the value");
	}

	return ss.str();
}

void DataValue::setValue(const std::string &v)
{
	if(v == "null")
	{
		deleteValue();
		return;
	}

	if(isNull())initDefaultValue();

	switch(type)
	{
	case ValueType::c_bit:
		for(unsigned int i = 0; i < (length / 8) + (length % 8 == 0 ? 0 : 1); ++i)
		{
			if(v.length() > i && v[i] != '0' && v[i] != '1')throw SQLException(std::string("Invalid format for bitfield: ")+v);
			int number = static_cast<char*>(value)[i/8];
			number ^= (-(v.length()<=i || v[i]=='0' ? 0 : 1) ^ number) & (1 << (i%8));
			static_cast<char*>(value)[i/8] = char(number);
		}
		break;
	case ValueType::c_year:
		if(length == 2)
		{
			const std::string &str = (v[0] == '\'' && v[v.length()-1] == '\'') ? v.substr(1, v.length()-2) : v;
			char *test;
			const long y = strtol(str.c_str(), &test, 0);
			if(*test != '\0' || y > 99 || y < 0)throw SQLException(v+" is not valid for year");
			(*static_cast<uint8_t*>(value)) = uint8_t(y);
		}
		else
		{
			const std::string &str = (v[0] == '\'' && v[v.length()-1] == '\'') ? v.substr(1, v.length()-2) : v;
			char *test;
			const long y = strtol(str.c_str(), &test, 0);
			if(*test != '\0' || y > 255 || y < 0)throw SQLException(v+" is not valid for year");
			(*static_cast<uint8_t*>(value)) = uint8_t(y);
		}
		break;
	case ValueType::c_tinyint: {
		char *test;
		const long val = strtol(v.c_str(), &test, 0);
		if(*test != '\0')throw SQLException(v+" is not valid for tinyint");
		(*static_cast<int8_t*>(value)) = int8_t(val);
		break;
	}
	case ValueType::c_bool:
		if(v == "true")(*static_cast<bool*>(value)) = true;
		else if(v == "false")(*static_cast<bool*>(value)) = false;
		else if(v == "1")(*static_cast<bool*>(value)) = true;
		else if(v == "0")(*static_cast<bool*>(value)) = false;
		else throw SQLException(v+" is not valid for bool");
		break;
	case ValueType::c_time:
		if(v.find(':') != std::string::npos)
		{
			const std::string &str = (v[0] == '\'' && v[v.length()-1] == '\'') ? v.substr(1, v.length()-2) : v;
			bool minus = (str[0] == '-');
			const std::size_t index1 = str.find(':');
			const std::size_t index2 = str.find(':', index1+1);
			if(index2 == std::string::npos)throw SQLException(v+" is not valid for time");
			const std::string &hour = str.substr(0, index1);
			const std::string &minute = str.substr(index1+1, index2-index1-1);
			const std::string &second = str.substr(index2+1);
			char *test;
			const long h = std::abs(strtol(hour.c_str(), &test, 10));
			if(*test != '\0')throw SQLException(v+" is not valid for time");
			const long m = std::abs(strtol(minute.c_str(), &test, 10));
			if(*test != '\0')throw SQLException(v+" is not valid for time");
			const long s = std::abs(strtol(second.c_str(), &test, 10));
			if(*test != '\0')throw SQLException(v+" is not valid for time");
			(*static_cast<int16_t*>(value)) = int16_t((h*3600 + m*60 + s) * (minus ? -1 : 1));
		}
		else
		{
			char *test;
			const long val = strtol(v.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for time");
			(*static_cast<int16_t*>(value)) = int16_t(val);
		}
		break;
	case ValueType::c_smallint: {
		char *test;
		const long val = strtol(v.c_str(), &test, 0);
		if(*test != '\0')throw SQLException(v+" is not valid for smallint");
		(*static_cast<int16_t*>(value)) = int16_t(val);
		break;
	}
	case ValueType::c_mediumint: {
		char *test;
		const long val = strtol(v.c_str(), &test, 0);
		if(*test != '\0')throw SQLException(v+" is not valid for mediumint");
		(*static_cast<int32_t*>(value)) = int32_t(val);
		break;
	}
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint: {
		char *test;
		const long long val = strtoll(v.c_str(), &test, 0);
		if(*test != '\0')throw SQLException(v+" is not valid for bigint");
		(*static_cast<int64_t*>(value)) = val;
		break;
	}
	case ValueType::c_text: {
		std::string str = (v[0] == '\'' && v[v.length()-1] == '\'') ? v.substr(1, v.length()-2) : v;
		std::size_t index;
		while((index=str.find("\\'")) != std::string::npos)
			str.replace(index, 2, "'");
		(*static_cast<std::string*>(value)) = str;
		break;
	}
	case ValueType::c_decimal:
		for(std::size_t i = 0; i < v.size(); ++i)if(!isdigit(v[i]) && v[i] != '.')throw SQLException(v+" is not valid for decimal");
		(*static_cast<std::string*>(value)) = v;
		break;
	case ValueType::c_float: {
		char *test;
		const float val = strtof(v.c_str(), &test);
		if(*test != '\0')throw SQLException(v+" is not valid for float");
		(*static_cast<float*>(value)) = val;
		break;
	}
	case ValueType::c_double: {
		char *test;
		const double val = strtod(v.c_str(), &test);
		if(*test != '\0')throw SQLException(v+" is not valid for double");
		(*static_cast<double*>(value)) = val;
		break;
	}
	case ValueType::c_date:
		if(v.find('-') != std::string::npos)
		{
			const std::string &str = (v[0] == '\'' && v[v.length()-1] == '\'') ? v.substr(1, v.length()-2) : v;
			const std::size_t index1 = str.find('-');
			const std::size_t index2 = str.find('-', index1+1);
			if(index2 == std::string::npos)throw SQLException(v+" is not valid for date");
			const std::string &year = str.substr(0, index1);
			const std::string &month = str.substr(index1+1, index2-index1-1);
			const std::string &day = str.substr(index2+1);
			char *test;
			const long y = strtol(year.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for date");
			const long m = strtol(month.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for date");
			const long d = strtol(day.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for date");
			(*(static_cast<uint16_t*>(value))) = uint16_t(y);
			(*(static_cast<uint8_t*>(value)+2)) = uint8_t(m);
			(*(static_cast<uint8_t*>(value)+3)) = uint8_t(d);
		}
		else
		{
			char *test;
			const long val = strtol(v.c_str(), &test, 0);
			if(*test != '\0')throw SQLException(v+" is not valid for date");
			convertDaysToDate(val,
				(*(static_cast<uint16_t*>(value))),
				(*(static_cast<uint8_t*>(value)+2)),
				(*(static_cast<uint8_t*>(value)+3)));
		}
		break;
	case ValueType::c_datetime:
		if(v.find('-') != std::string::npos)
		{
			const std::string &str = (v[0] == '\'' && v[v.length()-1] == '\'') ? v.substr(1, v.length()-2) : v;
			const std::size_t index1 = str.find('-');
			const std::size_t index2 = str.find('-', index1+1);
			const std::size_t index3 = str.find(' ', index2+1);
			const std::size_t index4 = str.find(':', index3+1);
			const std::size_t index5 = str.find(':', index4+1);
			if(index2 == std::string::npos)throw SQLException(v+" is not valid for datetime");
			if(index3 == std::string::npos)throw SQLException(v+" is not valid for datetime");
			if(index4 == std::string::npos)throw SQLException(v+" is not valid for datetime");
			if(index5 == std::string::npos)throw SQLException(v+" is not valid for datetime");
			const std::string &year = str.substr(0, index1);
			const std::string &month = str.substr(index1+1, index2-index1-1);
			const std::string &day = str.substr(index2+1, index3-index2-1);
			const std::string &hour = str.substr(index3+1, index4-index3-1);
			const std::string &minute = str.substr(index4+1, index5-index4-1);
			const std::string &second = str.substr(index5+1);
			char *test;
			const long y = strtol(year.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			const long m = strtol(month.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			const long d = strtol(day.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			const long h = strtol(hour.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			const long min = strtol(minute.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			const long s = strtol(second.c_str(), &test, 10);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			(*(static_cast<uint16_t*>(value))) = uint16_t(y);
			(*(static_cast<uint8_t*>(value)+2)) = uint8_t(m);
			(*(static_cast<uint8_t*>(value)+3)) = uint8_t(d);
			(*(static_cast<uint8_t*>(value)+4)) = uint8_t(h);
			(*(static_cast<uint8_t*>(value)+5)) = uint8_t(min);
			(*(static_cast<uint8_t*>(value)+6)) = uint8_t(s);
		}
		else
		{
			char *test;
			const long val = strtol(v.c_str(), &test, 0);
			if(*test != '\0')throw SQLException(v+" is not valid for datetime");
			convertSecondsToDateTime(val, 
				(*(static_cast<uint16_t*>(value))),	//Year
				(*(static_cast<uint8_t*>(value)+2)),	//Month
				(*(static_cast<uint8_t*>(value)+3)),	//Day
				(*(static_cast<uint8_t*>(value)+4)),	//Hour
				(*(static_cast<uint8_t*>(value)+5)),	//Minute
				(*(static_cast<uint8_t*>(value)+6)));//Seconds
		}
		break;
	case ValueType::c_char:
		for(unsigned int i = 0; i < length; ++i)
		{
			static_cast<char*>(value)[i] = (v.length() > i) ? v[i] : '\0';
		}
		break;
	case ValueType::c_binary:
		for(unsigned int i = 0; i < length; ++i)
		{
			const std::string str = (v.length() > (i*2)) ? v.substr(i*2, 2) : "0";
			char *test;
			const long val = strtol(str.c_str(), &test, 16);
			if(*test != '\0')throw SQLException(v+" is not valid for binary");
			static_cast<char*>(value)[i] = char(val);
		}
		break;
	default:
		throw SQLException("Invalid data type when setting the value");
	}
}

DataValue& DataValue::operator++ ()
{
	if(value == nullptr)throw SQLException("Can't increase a null value");

	switch(type)
	{
	case ValueType::c_year:
		if((*static_cast<uint8_t*>(value)) == 99 && length == 2)throw SQLException("Maximum value for type year(2) reached");
//			if((*static_cast<int8_t*>(value)) == 0xFF)throw SQLException("Maximum value for type year(4) reached");
		++(*static_cast<uint8_t*>(value));
		break;
	case ValueType::c_tinyint:
//			if((*static_cast<int8_t*>(value)) == 0xFF)throw SQLException("Maximum value for type tinyint reached");
		++(*static_cast<int8_t*>(value));
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
//			if((*static_cast<int16_t*>(value)) == 0xFFFF)throw SQLException("Maximum value for type smallint reached");
		++(*static_cast<int16_t*>(value));
		break;
	case ValueType::c_mediumint:
//			if((*static_cast<int32_t*>(value)) == 0xFFFFFFFF)throw SQLException("Maximum value for type mediumint reached");
		++(*static_cast<int32_t*>(value));
		break;
	case ValueType::c_int:
	case ValueType::c_bigint:
//			if((*static_cast<int64_t*>(value)) == 0xFFFFFFFFFFFFFFFF)throw SQLException("Maximum value for type bigint reached");
		++(*static_cast<int64_t*>(value));
		break;
	case ValueType::c_decimal:
		throw SQLException("Not implemented yet");
	case ValueType::c_date: {
		long g = convertDateToDays(
			(*(static_cast<const uint16_t*>(value))),
			(*(static_cast<const uint8_t*>(value)+2)),
			(*(static_cast<const uint8_t*>(value)+3)));
		convertDaysToDate(g+1,
			(*(static_cast<uint16_t*>(value))),
			(*(static_cast<uint8_t*>(value)+2)),
			(*(static_cast<uint8_t*>(value)+3)));
		break;
	}
	case ValueType::c_datetime: {
		long g = convertDateTimeToSeconds(
			(*(static_cast<const uint16_t*>(value))),	//Year
			(*(static_cast<const uint8_t*>(value)+2)),	//Month
			(*(static_cast<const uint8_t*>(value)+3)),	//Day
			(*(static_cast<const uint8_t*>(value)+4)),	//Hour
			(*(static_cast<const uint8_t*>(value)+5)),	//Minute
			(*(static_cast<const uint8_t*>(value)+6)));	//Seconds
		convertSecondsToDateTime(g, 
			(*(static_cast<uint16_t*>(value))),	//Year
			(*(static_cast<uint8_t*>(value)+2)),	//Month
			(*(static_cast<uint8_t*>(value)+3)),	//Day
			(*(static_cast<uint8_t*>(value)+4)),	//Hour
			(*(static_cast<uint8_t*>(value)+5)),	//Minute
			(*(static_cast<uint8_t*>(value)+6)));//Seconds
		break;
	}
	default:
		throw SQLException("Invalid data type when setting the value");
	}

	return (*this);
}

void DataValue::setType(const std::string &t)
{
	if(t.length() >= 3 && t.substr(0,3) == "bit")
	{
		type = ValueType::c_bit;
		length = extractLength(t, 3, 1);
		return;
	}
	if(t == "tinyint")
	{
		type = ValueType::c_tinyint;
		return;
	}
	if(t == "bool" || t == "boolean")
	{
		type = ValueType::c_bool;
		return;
	}
	if(t == "smallint")
	{
		type = ValueType::c_smallint;
		return;
	}
	if(t == "mediumint")
	{
		type = ValueType::c_mediumint;
		return;
	}
	if(t == "int" || t == "integer")
	{
		type = ValueType::c_int;
		return;
	}
	if(t == "bigint")
	{
		type = ValueType::c_bigint;
		return;
	}
	if(t == "decimal" || t == "dec")
	{
		type = ValueType::c_decimal;
		return;
	}
	if(t == "float")
	{
		type = ValueType::c_float;
		return;
	}
	if(t == "double")
	{
		type = ValueType::c_double;
		return;
	}
	if(t == "date")
	{
		type = ValueType::c_date;
		return;
	}
	if(t == "datetime")
	{
		type = ValueType::c_datetime;
		return;
	}
	if(t == "timestamp")
	{
		type = ValueType::c_timestamp;
		return;
	}
	if(t == "time")
	{
		type = ValueType::c_time;
		return;
	}
	if(t.length() >= 4 && t.substr(0,4) == "year")
	{
		type = ValueType::c_year;
		length = extractLength(t, 4, 4);
		return;
	}
	if(t.length() >= 4 && t.substr(0,4) == "char")
	{
		type = ValueType::c_char;
		length = extractLength(t, 4, 1);
		return;
	}
	if(t.length() >= 7 && t.substr(0,7) == "varchar")
	{
		type = ValueType::c_char;
		length = extractLength(t, 7, 1);
		return;
	}
	if(t.length() >= 6 && t.substr(0,6) == "binary")
	{
		type = ValueType::c_binary;
		length = extractLength(t, 6, 1);
		return;
	}
	if(t.length() >= 9 && t.substr(0,9) == "varbinary")
	{
		type = ValueType::c_binary;
		length = extractLength(t, 9, 1);
		return;
	}
	if(t == "tinyblob" || t == "tinytext" || t == "blob" || t == "text" ||
		t == "mediumblob" || t == "mediumtext" || t == "longblob" || t == "longtext")
	{
		type = ValueType::c_text;
		return;
	}
	throw SQLException(std::string("Invalid data type: ")+t);
}

bool DataValue::retrieve(const Package &pkg)
{
	deleteValue();

	if(!(pkg>>type))return false;
	if(!(pkg>>length))return false;

	bool valueSet;
	if(!(pkg>>valueSet))return false;
	if(!valueSet)return true;

	bool success = false;

	switch(type)
	{
	case ValueType::c_bit:
		value = new char[(length / 8) + (length % 8 == 0 ? 0 : 1)];
		success = pkg.getAndNext(static_cast<char*>(value), (length / 8) + (length % 8 == 0 ? 0 : 1));
		break;
	case ValueType::c_year:
		value = new uint8_t();
		success = pkg>>(*static_cast<uint8_t*>(value));
		break;
	case ValueType::c_tinyint:
		value = new int8_t();
		success = pkg>>(*static_cast<int8_t*>(value));
		break;
	case ValueType::c_bool:
		value = new bool();
		success = pkg>>(*static_cast<bool*>(value));
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		value = new int16_t();
		success = pkg>>(*static_cast<int16_t*>(value));
		break;
	case ValueType::c_mediumint:
		value = new int32_t();
		success = pkg>>(*static_cast<int32_t*>(value));
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		value = new int64_t();
		success = pkg>>(*static_cast<int64_t*>(value));
		break;
	case ValueType::c_text:
	case ValueType::c_decimal:
		value = new std::string();
		success = pkg>>(*static_cast<std::string*>(value));
		break;
	case ValueType::c_float:
		value = new float();
		success = pkg>>(*static_cast<float*>(value));
		break;
	case ValueType::c_double:
		value = new double();
		success = pkg>>(*static_cast<double*>(value));
		break;
	case ValueType::c_date:
		value = new char[4];	//YYMD
		success = pkg.getAndNext(static_cast<char*>(value), 4);
		break;
	case ValueType::c_datetime:
		value = new char[7];	//YYMDHMS
		success = pkg.getAndNext(static_cast<char*>(value), 7);
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		value = new char[length];
		success = pkg.getAndNext(static_cast<char*>(value), length);
		break;
	default:
		throw SQLException("Invalid data type when retrieving");
	}

	return success;
}

void DataValue::insert(Package &pkg) const
{
	pkg<<type;
	pkg<<length;

	const bool valueSet = (value != nullptr);
	pkg<<valueSet;
	if(!valueSet)return;

	switch(type)
	{
	case ValueType::c_bit:
		pkg.write(static_cast<const char*>(value), (length / 8) + (length % 8 == 0 ? 0 : 1));
		break;
	case ValueType::c_year:
		pkg<<(*static_cast<const uint8_t*>(value));
		break;
	case ValueType::c_tinyint:
		pkg<<(*static_cast<const int8_t*>(value));
		break;
	case ValueType::c_bool:
		pkg<<(*static_cast<const bool*>(value));
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		pkg<<(*static_cast<const int16_t*>(value));
		break;
	case ValueType::c_mediumint:
		pkg<<(*static_cast<const int32_t*>(value));
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		pkg<<(*static_cast<const int64_t*>(value));
		break;
	case ValueType::c_text:
	case ValueType::c_decimal:
		pkg<<(*static_cast<const std::string*>(value));
		break;
	case ValueType::c_float:
		pkg<<(*static_cast<const float*>(value));
		break;
	case ValueType::c_double:
		pkg<<(*static_cast<const double*>(value));
		break;
	case ValueType::c_date:
		pkg.write(static_cast<const char*>(value), 4);	//YYMD
		break;
	case ValueType::c_datetime:
		pkg.write(static_cast<const char*>(value), 7);	//YYMDHMS
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		pkg.write(static_cast<const char*>(value), length);
		break;
	default:
		throw SQLException("Invalid data type when inserting to package");
	}
}

void DataValue::deleteValue()
{
	if(!value)return;

	switch(type)
	{
	case ValueType::c_bit:
		delete [] static_cast<char*>(value);
		break;
	case ValueType::c_year:
		delete static_cast<uint8_t*>(value);
		break;
	case ValueType::c_tinyint:
		delete static_cast<int8_t*>(value);
		break;
	case ValueType::c_bool:
		delete static_cast<bool*>(value);
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		delete static_cast<int16_t*>(value);
		break;
	case ValueType::c_mediumint:
		delete static_cast<int32_t*>(value);
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		delete static_cast<int64_t*>(value);
		break;
	case ValueType::c_text:
	case ValueType::c_decimal:
		delete static_cast<std::string*>(value);
		break;
	case ValueType::c_float:
		delete static_cast<float*>(value);
		break;
	case ValueType::c_double:
		delete static_cast<double*>(value);
		break;
	case ValueType::c_date:
		delete [] static_cast<char*>(value);
		break;
	case ValueType::c_datetime:
		delete [] static_cast<char*>(value);
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		delete [] static_cast<char*>(value);
		break;
	default:
		throw SQLException("Invalid data type when deleting");
	}

	value = nullptr;
}

int DataValue::compare(const DataValue &other) const
{
	switch(type)
	{
	case ValueType::c_bit:
		for(unsigned int i = 0; i < (length / 8) + (length % 8 == 0 ? 0 : 1) && i < (other.length / 8) + (other.length % 8 == 0 ? 0 : 1); ++i)
		{
			if(static_cast<char*>(value)[i] < static_cast<char*>(other.value)[i])return -1;
			if(static_cast<char*>(value)[i] > static_cast<char*>(other.value)[i])return 1;
		}
		return length - other.length;
	case ValueType::c_year:
		return int(*static_cast<uint8_t*>(value)) - int(*static_cast<uint8_t*>(other.value));
	case ValueType::c_tinyint:
		return (*static_cast<int8_t*>(value)) - (*static_cast<int8_t*>(other.value));
	case ValueType::c_bool:
		return (*static_cast<bool*>(value)) - (*static_cast<bool*>(other.value));
	case ValueType::c_time:
	case ValueType::c_smallint:
		return (*static_cast<int16_t*>(value)) - (*static_cast<int16_t*>(other.value));
	case ValueType::c_mediumint:
		return (*static_cast<int32_t*>(value)) - (*static_cast<int32_t*>(other.value));
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		return (*static_cast<int64_t*>(value)) < (*static_cast<int64_t*>(other.value)) ? -1 :
			(*static_cast<int64_t*>(value)) > (*static_cast<int64_t*>(other.value)) ? 1 :
			0;
	case ValueType::c_text:
		return static_cast<std::string*>(value)->compare(*static_cast<std::string*>(other.value));
	case ValueType::c_decimal:
		throw SQLException("Not implemented yet");
	case ValueType::c_float:
		return (*static_cast<float*>(value)) < (*static_cast<float*>(other.value)) ? -1 :
			(*static_cast<float*>(value)) > (*static_cast<float*>(other.value)) ? 1 :
			0;
	case ValueType::c_double:
		return (*static_cast<double*>(value)) < (*static_cast<double*>(other.value)) ? -1 :
			(*static_cast<double*>(value)) > (*static_cast<double*>(other.value)) ? 1 :
			0;
	case ValueType::c_date:
		for(unsigned int i = 0; i < 4; ++i)
		{
			if(static_cast<unsigned char*>(value)[i] < static_cast<unsigned char*>(other.value)[i])return -1;	//Conversion needed
			if(static_cast<unsigned char*>(value)[i] > static_cast<unsigned char*>(other.value)[i])return 1;	//Conversion needed
		}
		return 0;
	case ValueType::c_datetime:
		for(unsigned int i = 0; i < 7; ++i)
		{
			if(static_cast<unsigned char*>(value)[i] < static_cast<unsigned char*>(other.value)[i])return -1;	//Conversion needed
			if(static_cast<unsigned char*>(value)[i] > static_cast<unsigned char*>(other.value)[i])return 1;	//Conversion needed
		}
		return 0;
	case ValueType::c_char:
	case ValueType::c_binary:
		for(unsigned int i = 0; i < length && i < other.length; ++i)
		{
			if(static_cast<char*>(value)[i] < static_cast<char*>(other.value)[i])return -1;
			if(static_cast<char*>(value)[i] > static_cast<char*>(other.value)[i])return 1;
		}
		return length - other.length;
	default:
		throw SQLException("Invalid data type when comparing");
	}
}

long long DataValue::getDataSize() const
{
	if(isNull())return 0;

	switch(type)
	{
	case ValueType::c_bit:
		return sizeof(char) * ((length / 8) + (length % 8 == 0 ? 0 : 1));
	case ValueType::c_year:
		return sizeof(uint8_t);
	case ValueType::c_tinyint:
		return sizeof(int8_t);
	case ValueType::c_bool:
		return sizeof(bool);
	case ValueType::c_time:
	case ValueType::c_smallint:
		return sizeof(int16_t);
	case ValueType::c_mediumint:
		return sizeof(int32_t);
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		return sizeof(int64_t);
	case ValueType::c_text:
	case ValueType::c_decimal:
		return sizeof(unsigned int) + sizeof(char) * static_cast<const std::string*>(value)->size();
	case ValueType::c_float:
		return sizeof(float);
	case ValueType::c_double:
		return sizeof(double);
	case ValueType::c_date:
		return sizeof(char) * 4;
	case ValueType::c_datetime:
		return sizeof(char) * 7;
	case ValueType::c_char:
	case ValueType::c_binary:
		return sizeof(char) * length;
	default:
		throw SQLException("Invalid data type when checking data size");
	}
}

void DataValue::writeTo(std::ostream &o) const
{
	if(isNull())return;

	switch(type)
	{
	case ValueType::c_bit:
		o.write(static_cast<const char*>(value), sizeof(char) * ((length / 8) + (length % 8 == 0 ? 0 : 1)));
		break;
	case ValueType::c_year:
		o.write(static_cast<const char*>(value), sizeof(uint8_t));
	case ValueType::c_tinyint:
		o.write(static_cast<const char*>(value), sizeof(int8_t));
		break;
	case ValueType::c_bool:
		o.write(static_cast<const char*>(value), sizeof(bool));
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		o.write(static_cast<const char*>(value), sizeof(int16_t));
		break;
	case ValueType::c_mediumint:
		o.write(static_cast<const char*>(value), sizeof(int32_t));
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		o.write(static_cast<const char*>(value), sizeof(int64_t));
		break;
	case ValueType::c_text:
	case ValueType::c_decimal: {
		const unsigned int size = (unsigned int)static_cast<const std::string*>(value)->size();
		o.write(reinterpret_cast<const char*>(&size), sizeof(size));
		o.write(static_cast<const std::string*>(value)->c_str(), sizeof(char)*size);
		break;
	}
	case ValueType::c_float:
		o.write(static_cast<const char*>(value), sizeof(float));
		break;
	case ValueType::c_double:
		o.write(static_cast<const char*>(value), sizeof(double));
		break;
	case ValueType::c_date:
		o.write(static_cast<const char*>(value), sizeof(char)*4);	//YYMD
		break;
	case ValueType::c_datetime:
		o.write(static_cast<const char*>(value), sizeof(char)*7);	//YYMDHMS
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		o.write(static_cast<const char*>(value), sizeof(char)*length);
		break;
	default:
		throw SQLException("Invalid data type when inserting to package");
	}
}

bool DataValue::loadFrom(std::istream &i)
{
	bool success = false;

	switch(type)
	{
	case ValueType::c_bit:
		value = new char[(length / 8) + (length % 8 == 0 ? 0 : 1)];
		success = i.read(static_cast<char*>(value), sizeof(char) * ((length / 8) + (length % 8 == 0 ? 0 : 1)));
		break;
	case ValueType::c_year:
		value = new uint8_t();
		success = i.read(static_cast<char*>(value), sizeof(uint8_t));
		break;
	case ValueType::c_tinyint:
		value = new int8_t();
		success = i.read(static_cast<char*>(value), sizeof(int8_t));
		break;
	case ValueType::c_bool:
		value = new bool();
		success = i.read(static_cast<char*>(value), sizeof(bool));
		break;
	case ValueType::c_time:
	case ValueType::c_smallint:
		value = new int16_t();
		success = i.read(static_cast<char*>(value), sizeof(int16_t));
		break;
	case ValueType::c_mediumint:
		value = new int32_t();
		success = i.read(static_cast<char*>(value), sizeof(int32_t));
		break;
	case ValueType::c_timestamp:
	case ValueType::c_int:
	case ValueType::c_bigint:
		value = new int64_t();
		success = i.read(static_cast<char*>(value), sizeof(int64_t));
		break;
	case ValueType::c_text:
	case ValueType::c_decimal: {
		unsigned int size;
		i.read(reinterpret_cast<char*>(&size), sizeof(size));
		std::vector<char> data(size);
		success = i.read(&data[0], sizeof(char)*size);
		value = new std::string(&data[0], &data[0]+size);
		break;
	}
	case ValueType::c_float:
		value = new float();
		success = i.read(static_cast<char*>(value), sizeof(float));
		break;
	case ValueType::c_double:
		value = new double();
		success = i.read(static_cast<char*>(value), sizeof(double));
		break;
	case ValueType::c_date:
		value = new char[4];	//YYMD
		success = i.read(static_cast<char*>(value), sizeof(char)*4);
		break;
	case ValueType::c_datetime:
		value = new char[7];	//YYMDHMS
		success = i.read(static_cast<char*>(value), sizeof(char)*7);
		break;
	case ValueType::c_char:
	case ValueType::c_binary:
		value = new char[length];
		success = i.read(static_cast<char*>(value), length);
		break;
	default:
		throw SQLException("Invalid data type when reading from stream");
	}

	return success;
}
