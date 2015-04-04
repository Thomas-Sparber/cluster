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
	Package(const unsigned char *uc_data, unsigned int length) :
		data(uc_data, uc_data+length),
		iteratorPosition(0)
	{}

	/**
	  * Constructor to create a Package using
	  * the given content
	 **/
	Package(const std::vector<unsigned char> &v_data) :
		data(v_data),
		iteratorPosition(0)
	{}

	void append(const char &t){ append_internal(t); }
	void append(const char16_t &t){ append_internal(t); }
	void append(const char32_t &t){ append_internal(t); }	
	void append(const wchar_t &t){ append_internal(t); }
	void append(const signed char &t){ append_internal(t); }
	void append(const signed short int &t){ append_internal(t); }
	void append(const signed int &t){ append_internal(t); }
	void append(const signed long int &t){ append_internal(t); }
	void append(const signed long long int &t){ append_internal(t); }
	void append(const unsigned char &t){ append_internal(t); }
	void append(const unsigned short int &t){ append_internal(t); }
	void append(const unsigned int &t){ append_internal(t); }
	void append(const unsigned long int &t){ append_internal(t); }
	void append(const unsigned long long int &t){ append_internal(t); }
	void append(const float &t){ append_internal(t); }
	void append(const double &t){ append_internal(t); }
	void append(const long double &t){ append_internal(t); }
	void append(const bool &t){ append_internal(t); }

	void append(const char *array, unsigned int size) { append_internal(array, size); }
	void append(const char16_t *array, unsigned int size) { append_internal(array, size); }
	void append(const char32_t *array, unsigned int size) { append_internal(array, size); }
	void append(const wchar_t *array, unsigned int size) { append_internal(array, size); }
	void append(const signed char *array, unsigned int size) { append_internal(array, size); }
	void append(const signed short int *array, unsigned int size) { append_internal(array, size); }
	void append(const signed int *array, unsigned int size) { append_internal(array, size); }
	void append(const signed long int *array, unsigned int size) { append_internal(array, size); }
	void append(const signed long long int *array, unsigned int size) { append_internal(array, size); }
	void append(const unsigned char *array, unsigned int size) { append_internal(array, size); }
	void append(const unsigned short int *array, unsigned int size) { append_internal(array, size); }
	void append(const unsigned int *array, unsigned int size) { append_internal(array, size); }
	void append(const unsigned long int *array, unsigned int size) { append_internal(array, size); }
	void append(const unsigned long long int *array, unsigned int size) { append_internal(array, size); }
	void append(const float *array, unsigned int size) { append_internal(array, size); }
	void append(const double *array, unsigned int size) { append_internal(array, size); }
	void append(const long double *array, unsigned int size) { append_internal(array, size); }
	void append(const bool *array, unsigned int size) { append_internal(array, size); }

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

	bool get(char *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(char16_t *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(char32_t *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(wchar_t *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(signed char *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(signed short int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(signed int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(signed long int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(signed long long int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(unsigned char *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(unsigned short int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(unsigned int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(unsigned long int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(unsigned long long int *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(float *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(double *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(long double *&array, unsigned int size) const { return get_internal(array, size); }
	bool get(bool *&array, unsigned int size) const { return get_internal(array, size); }

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

	bool getAndNext(char *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(char16_t *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(char32_t *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(wchar_t *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed char *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed short int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed long int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(signed long long int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned char *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned short int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned long int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(unsigned long long int *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(float *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(double *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(long double *&array, unsigned int size) const { return getAndNext_internal(array, size); }
	bool getAndNext(bool *&array, unsigned int size) const { return getAndNext_internal(array, size); }

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
	void next(unsigned int size) const
	{
		if(iteratorPosition+size > data.size())return;
		iteratorPosition += size;
	}

	/**
	  * Returns the current content size of the Package
	 **/
	unsigned int getLength() const
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
	const unsigned char* getData() const
	{
		return &(data[0]);
	}

	/**
	  * Returns the current read iterator position
	 **/
	unsigned int getIteratorPosition() const
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
				if(lastWasBinary)os<<'.';
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
		if(lastWasBinary)os<<'.';
		return os.str();
	}

private:
	/**
	  * Appends the given object to the Package
	 **/
	template <class T>
	void append_internal(const T &t)
	{
		const unsigned char *dataToAppend = reinterpret_cast<const unsigned char*>(&t);
		data.insert(data.end(), dataToAppend, dataToAppend + sizeof(T));
	}

	/**
	  * Appends the given array of objects to the Package
	 **/
	template <class T>
	void append_internal(const T *t, unsigned int size)
	{
		if(size <= 0)return;
		const unsigned char *dataToAppend = reinterpret_cast<const unsigned char*>(t);
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
	bool getAndNext_internal(T *&t, unsigned int size) const
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
		unsigned int size = sizeof(T);
		if(iteratorPosition+size > data.size())return false;
		const void *tempData = reinterpret_cast<const void*>(&data[iteratorPosition]);
		t = *reinterpret_cast<const T*>(tempData);
		return true;
	}

	/**
	  * Returns an array of objects of the given type
	 **/
	template <class T>
	bool get_internal(T *&t, unsigned int size) const
	{
		if(size <= 0)return true;
		if(iteratorPosition+size > data.size())return false;
		std::copy(&data[iteratorPosition], &data[iteratorPosition+size], reinterpret_cast<unsigned char*>(t));
		return true;
	}

private:

	/**
	  * The char array where the data are stored
	 **/
	std::vector<unsigned char> data;

	/**
	  * The current interator position
	 **/
	mutable unsigned int iteratorPosition;

}; //end class package


/*-----	Functions for adding data to package	-----*/

/**
  * Adds the given object to the Package
 **/
template <class T>
inline void operator<<(Package &p, const T &t)
{
	p.append(t);
}

/**
  * Adds the given array to the Package
 **/
template <typename T, unsigned int N>
inline void operator<<(Package &p, const T (&t)[N])
{
	p.append(t, N);
}

/**
  * Appends the given Package to the Package
 **/
template <>
inline void operator<<(Package &p, const Package &t)
{
	p.append(t.getData(), t.getLength());
}

/**
  * Adds the given string to the Package
 **/
template <>
inline void operator<<(Package &p, const std::string &t)
{
	p<<t.size();
	p.append(t.c_str(), t.size());
}


/*-----	Functions for getting data from package	-----*/

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
template <typename T, unsigned int N>
bool operator>>(const Package &p, T(&t)[N])
{
	for(unsigned int i = 0; i < N; i++)
	{
		if(!p.getAndNext(t[i]))return false;
	}
	return true;
}

/**
  * Retrieves the given string from the Package
 **/
template <>
inline bool operator>>(const Package &p, std::string &t)
{
	size_t size;
	if(!p.getAndNext(size))return false;
	char *temp = new char[size];
	bool success = p.getAndNext(temp, size);
	t.clear();
	t.append(temp, size);
	delete [] temp;
	return success;
}

} //end namespace cluster

#endif // PACKAGE_HPP
