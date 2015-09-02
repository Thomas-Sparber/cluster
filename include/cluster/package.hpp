/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <algorithm>
#include <sstream>
#include <vector>
#include <list>

namespace cluster
{

/**
  * The class Package is responsible for storing
  * the data which is transferred over the network.
  * The class supports the operators << and >>
  * for easy data insertion and retrieval.
  * Package contains functions for inserting or
  * retrieving basic data types. To insert and retrieve
  * objects of user defined classes, the operators
  * << and >> need to be overridden.
  * Package also contains a string function to get
  * the content in string representation this also
  * works fairly good with binary data.
 **/
class Package
{

public:
	/**
	  * Default constructor.
	 **/
	Package() :
		data(),
		iteratorPosition(0)
	{}

	/**
	  * Constructor to create a Package using
	  * the given content
	 **/
	Package(const char *c_data, std::size_t length) :
		data(c_data, c_data+length),
		iteratorPosition(0)
	{}

	/**
	  * Constructor to create a Package using
	  * the given content
	 **/
	Package(const std::vector<char> &v_data) :
		data(v_data),
		iteratorPosition(0)
	{}

	void write(const char &t){ write_internal(t); }
	void write(const char16_t &t){ write_internal(t); }
	void write(const char32_t &t){ write_internal(t); }	
	void write(const wchar_t &t){ write_internal(t); }
	void write(const signed char &t){ write_internal(t); }
	void write(const signed short int &t){ write_internal(t); }
	void write(const signed int &t){ write_internal(t); }
	void write(const signed long int &t){ write_internal(t); }
	void write(const signed long long int &t){ write_internal(t); }
	void write(const unsigned char &t){ write_internal(t); }
	void write(const unsigned short int &t){ write_internal(t); }
	void write(const unsigned int &t){ write_internal(t); }
	void write(const unsigned long int &t){ write_internal(t); }
	void write(const unsigned long long int &t){ write_internal(t); }
	void write(const float &t){ write_internal(t); }
	void write(const double &t){ write_internal(t); }
	void write(const long double &t){ write_internal(t); }
	void write(const bool &t){ write_internal(t); }

	void write(const char *array, std::size_t size) { write_internal(array, size); }
	void write(const char16_t *array, std::size_t size) { write_internal(array, size); }
	void write(const char32_t *array, std::size_t size) { write_internal(array, size); }
	void write(const wchar_t *array, std::size_t size) { write_internal(array, size); }
	void write(const signed char *array, std::size_t size) { write_internal(array, size); }
	void write(const signed short int *array, std::size_t size) { write_internal(array, size); }
	void write(const signed int *array, std::size_t size) { write_internal(array, size); }
	void write(const signed long int *array, std::size_t size) { write_internal(array, size); }
	void write(const signed long long int *array, std::size_t size) { write_internal(array, size); }
	void write(const unsigned char *array, std::size_t size) { write_internal(array, size); }
	void write(const unsigned short int *array, std::size_t size) { write_internal(array, size); }
	void write(const unsigned int *array, std::size_t size) { write_internal(array, size); }
	void write(const unsigned long int *array, std::size_t size) { write_internal(array, size); }
	void write(const unsigned long long int *array, std::size_t size) { write_internal(array, size); }
	void write(const float *array, std::size_t size) { write_internal(array, size); }
	void write(const double *array, std::size_t size) { write_internal(array, size); }
	void write(const long double *array, std::size_t size) { write_internal(array, size); }
	void write(const bool *array, std::size_t size) { write_internal(array, size); }

	bool get(char &t) const { return get_internal(t); }
	bool get(char16_t &t) const { return get_internal(t); }
	bool get(char32_t &t) const { return get_internal(t); }
	bool get(wchar_t &t) const { return get_internal(t); }
	bool get(signed char &t) const { return get_internal(t); }
	bool get(signed short int &t) const { return get_internal(t); }
	bool get(signed int &t) const { return get_internal(t); }
	bool get(signed long int &t) const { return get_internal(t); }
	bool get(signed long long int &t) const { return get_internal(t); }
	bool get(unsigned char &t) const { return get_internal(t); }
	bool get(unsigned short int &t) const { return get_internal(t); }
	bool get(unsigned int &t) const { return get_internal(t); }
	bool get(unsigned long int &t) const { return get_internal(t); }
	bool get(unsigned long long int &t) const { return get_internal(t); }
	bool get(float &t) const { return get_internal(t); }
	bool get(double &t) const { return get_internal(t); }
	bool get(long double &t) const { return get_internal(t); }
	bool get(bool &t) const { return get_internal(t); }

	bool get(char *array, std::size_t size) const { return get_internal(array, size); }
	bool get(char16_t *array, std::size_t size) const { return get_internal(array, size); }
	bool get(char32_t *array, std::size_t size) const { return get_internal(array, size); }
	bool get(wchar_t *array, std::size_t size) const { return get_internal(array, size); }
	bool get(signed char *array, std::size_t size) const { return get_internal(array, size); }
	bool get(signed short int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(signed int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(signed long int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(signed long long int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(unsigned char *array, std::size_t size) const { return get_internal(array, size); }
	bool get(unsigned short int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(unsigned int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(unsigned long int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(unsigned long long int *array, std::size_t size) const { return get_internal(array, size); }
	bool get(float *array, std::size_t size) const { return get_internal(array, size); }
	bool get(double *array, std::size_t size) const { return get_internal(array, size); }
	bool get(long double *array, std::size_t size) const { return get_internal(array, size); }
	bool get(bool *array, std::size_t size) const { return get_internal(array, size); }

	bool getAndNext(char &t) const { return getAndNext_internal(t); }
	bool getAndNext(char16_t &t) const { return getAndNext_internal(t); }
	bool getAndNext(char32_t &t) const { return getAndNext_internal(t); }
	bool getAndNext(wchar_t &t) const { return getAndNext_internal(t); }
	bool getAndNext(signed char &t) const { return getAndNext_internal(t); }
	bool getAndNext(signed short int &t) const { return getAndNext_internal(t); }
	bool getAndNext(signed int &t) const { return getAndNext_internal(t); }
	bool getAndNext(signed long int &t) const { return getAndNext_internal(t); }
	bool getAndNext(signed long long int &t) const { return getAndNext_internal(t); }
	bool getAndNext(unsigned char &t) const { return getAndNext_internal(t); }
	bool getAndNext(unsigned short int &t) const { return getAndNext_internal(t); }
	bool getAndNext(unsigned int &t) const { return getAndNext_internal(t); }
	bool getAndNext(unsigned long int &t) const { return getAndNext_internal(t); }
	bool getAndNext(unsigned long long int &t) const { return getAndNext_internal(t); }
	bool getAndNext(float &t) const { return getAndNext_internal(t); }
	bool getAndNext(double &t) const { return getAndNext_internal(t); }
	bool getAndNext(long double &t) const { return getAndNext_internal(t); }
	bool getAndNext(bool &t) const { return getAndNext_internal(t); }

	bool getAndNext(char *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(char16_t *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(char32_t *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(wchar_t *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed char *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed short int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed long int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed long long int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned char *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned short int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned long int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned long long int *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(float *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(double *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(long double *array, std::size_t size) const { return getAndNext_internal(array, size); }
	bool getAndNext(bool *array, std::size_t size) const { return getAndNext_internal(array, size); }

	/**
	  * Returns whether the current package equals the given one
	 **/
	bool operator== (const Package &pkg) const
	{
		return this->data == pkg.data;
	}

	/**
	  * Returns whether the current package is
	  * different than the given one
	 **/
	bool operator!= (const Package &pkg) const
	{
		return this->data != pkg.data;
	}

	/**
	  * Clears the content of the package
	 **/
	void clear()
	{
		data.clear();
	}

	/**
	  * Creates a new Package which contains
	  * the content after the current read position
	 **/
	Package subPackageFromCurrentPosition() const
	{
		return Package(&data[iteratorPosition], data.size() - iteratorPosition);
	}

	/**
	  * Increases the read operator by the
	  * given size in Bytes
	 **/
	void next(std::size_t size) const
	{
		if(iteratorPosition+size > data.size())return;
		iteratorPosition += size;
	}

	/**
	  * Returns the current content size of the Package
	 **/
	std::size_t getLength() const
	{
		return data.size();
	}

	/**
	  * Checks whether the Package is empty
	 **/
	bool empty() const
	{
		return data.empty();
	}

	/**
	  * Checks whether all data was read from the package
	 **/
	bool finished() const
	{
		return iteratorPosition == data.size();
	}

	/**
	  * Checks whether the Package is empty
	  * or contains only 0s
	 **/
	bool emptyOrNull() const
	{
		if(empty())return true;

		for(const char c : data)
		{
			if(c != '\0')return false;
		}
		return true;
	}

	/**
	  * Returns the data as an array of bytes
	 **/
	const char* getData() const
	{
		return &(data[0]);
	}

	/**
	  * Returns the current read iterator position
	 **/
	std::size_t getIteratorPosition() const
	{
		return iteratorPosition;
	}

	/**
	  * Resets the read iterator
	 **/
	void resetIterator()
	{
		iteratorPosition = 0;
	}

	/**
	  * Returns the content of the Package
	  * as string. Binary values are converted
	  * to .[HEX].
	 **/
	std::string toString() const
	{
		std::stringstream os;
		bool lastWasBinary = false;
		for(const char &c : data)
		{
			if(isalnum(c))
			{
				if(lastWasBinary)os<<'.'<<std::dec;
				os<<c;
				lastWasBinary = false;
			}
			else
			{
				if(!lastWasBinary)os<<'.';
				os<<std::hex<<int(c);
				lastWasBinary = true;
			}
		}
		if(lastWasBinary)os<<'.'<<std::dec;
		os<<" ("<<data.size()<<")";
		return os.str();
	}

private:
	/**
	  * Appends the given object to the Package
	 **/
	template <class T>
	void write_internal(const T &t)
	{
		const char *dataToAppend = reinterpret_cast<const char*>(&t);
		data.insert(data.end(), dataToAppend, dataToAppend + sizeof(T));
	}

	/**
	  * Appends the given array of objects to the Package
	 **/
	template <class T>
	void write_internal(const T *t, std::size_t size)
	{
		if(size <= 0)return;
		const char *dataToAppend = reinterpret_cast<const char*>(t);
		data.insert(data.end(), dataToAppend, dataToAppend + (sizeof(T)*size));
	}

	/**
	  * Returns an object of the given type and
	  * increases the iterator by the size of the object
	 **/
	template <class T>
	bool getAndNext_internal(T &t) const
	{
		bool success = get_internal(t);
		next(sizeof(T));
		return success;
	}

	/**
	  * Returns an array ob objects of the given type and
	  * increases the iterator by the size of the objects
	 **/
	template <class T>
	bool getAndNext_internal(T *t, std::size_t size) const
	{
		bool success = get_internal(t, size);
		next(size);
		return success;
	}

	/**
	  * Returns an object of the given type
	 **/
	template <class T>
	bool get_internal(T &t) const
	{
		std::size_t size = sizeof(T);
		if(iteratorPosition+size > data.size())return false;
		const void *tempData = reinterpret_cast<const void*>(&data[iteratorPosition]);
		t = *reinterpret_cast<const T*>(tempData);
		return true;
	}

	/**
	  * Returns an array of objects of the given type
	 **/
	template <class T>
	bool get_internal(T *t, std::size_t size) const
	{
		if(size <= 0)return true;
		if(iteratorPosition+size > data.size())return false;
		std::copy(&data[iteratorPosition], &data[iteratorPosition+size], reinterpret_cast<char*>(t));
		return true;
	}

private:

	/**
	  * The char array where the data are stored
	 **/
	std::vector<char> data;

	/**
	  * The current interator position
	 **/
	mutable std::size_t iteratorPosition;

}; //end class package


/*-----	Functions for adding data to package	-----*/

template <class T> void operator<<(Package &p, const T &t);

/**
  * Adds the given object to the Package
 **/
template <class T>
inline void operator<<(Package &p, const T &t)
{
	p.write(t);
}

/**
  * Adds the given array to the Package
 **/
template <typename T, std::size_t N>
inline void operator<<(Package &p, const T (&t)[N])
{
	p.write(t, N);
}

/**
  * Appends the given size_t to the Package
 **/
//template <> void operator<<(Package &/*p*/, const std::size_t &/*t*/) __attribute__((deprecated));

/**
  * Appends the given Package to the Package
 **/
template <>
inline void operator<<(Package &p, const Package &t)
{
	p.write(t.getData(), t.getLength());
}

/**
  * Adds the given string to the Package
 **/
template <>
inline void operator<<(Package &p, const std::string &t)
{
	const uint64_t size = t.size();
	p<<size;
	p.write(t.c_str(), (std::size_t)size);
}

/**
  * Adds the given vector to the Package
 **/
template <class T>
inline void operator<<(Package &p, const std::vector<T> &t)
{
	const uint64_t size = t.size();
	p<<size;
	for(std::size_t i = 0; i < size; ++i)
	{
		p<<t[i];
	}
}

/**
  * Adds the given list to the Package
 **/
template <class T>
inline void operator<<(Package &p, const std::list<T> &t)
{
	const uint64_t size = t.size();
	p<<size;
	for(const T &value : t)
	{
		p<<value;
	}
}

/**
  * Adds the given pair to the Package
 **/
template <class A, class B>
inline void operator<<(Package &p, const std::pair<A,B> &t)
{
	p<<t.first;
	p<<t.second;
}


/*-----	Functions for getting data from package	-----*/

template <class T> bool operator>>(const Package &p, T &t);

/**
  * Retrieves the given object from the Package
 **/
template <class T>
inline bool operator>>(const Package &p, T &t)
{
	return p.getAndNext(t);
}

/**
  * Retrieves the given array from the Package
 **/
template <typename T, std::size_t N>
inline bool operator>>(const Package &p, T(&t)[N])
{
	for(std::size_t i = 0; i < N; i++)
	{
		if(!(p>>t[i]))return false;
	}
	return true;
}

/**
  * Retrieves the given size_t from the Package
 **/
//template <> bool operator>>(const Package &/*p*/, size_t &/*t*/) __attribute__((deprecated));

/**
  * Retrieves the given string from the Package
 **/
template <>
inline bool operator>>(const Package &p, std::string &t)
{
	uint64_t size;
	if(!(p>>size))return false;
	t.resize((std::size_t)size);
	bool success = p.getAndNext(&t[0], (std::size_t)size);
	return success;
}

/**
  * Retrieves the given vector from the Package
 **/
template <class T>
inline bool operator>>(const Package &p, std::vector<T> &t)
{
	uint64_t size;
	if(!(p>>size))return false;
	t.resize((std::size_t)size);
	for(unsigned int i = 0; i < size; ++i)
	{
		if(!(p>>t[i]))return false;
	}
	return true;
}

/**
  * Retrieves the given list from the Package
 **/
template <class T>
inline bool operator>>(const Package &p, std::list<T> &t)
{
	uint64_t size;
	if(!(p>>size))return false;
	t.clear();
	for(unsigned int i = 0; i < size; ++i)
	{
		T value;
		if(!(p>>value))return false;
		t.push_back(value);
	}
	return true;
}

/**
  * Retrieves the given pair from the Package
 **/
template <class A, class B>
inline bool operator>>(const Package &p, std::pair<A,B> &t)
{
	if(!(p>>t.first))return false;
	if(!(p>>t.second))return false;
	return true;
}

} //end namespace cluster

#endif // PACKAGE_HPP
