#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <algorithm>
#include <sstream>

namespace cluster
{

class Package
{

public:
	Package(unsigned int ui_allocationSize=1024) :
		allocationSize(ui_allocationSize),
		data(new unsigned char[ui_allocationSize]),
		length(0),
		capacity(ui_allocationSize),
		iteratorPosition(0)
	{}

	Package(const Package &p, bool compress=false) :
		allocationSize(p.allocationSize),
		data(new unsigned char[compress ? p.length : p.capacity]),
		length(p.length),
		capacity(compress ? p.length : p.capacity),
		iteratorPosition(p.iteratorPosition)
	{
		std::copy(p.data, p.data+p.length, data);
	}

	~Package()
	{
		delete [] data;
	}

	Package& operator=(const Package &p)
	{
		allocationSize = p.allocationSize;
		length = p.length;
		if(length > capacity)reallocate(length);
		std::copy(p.data, p.data+p.length, data);
		iteratorPosition = p.iteratorPosition;
		return (*this);
	}

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

	void reallocate(unsigned int size)
	{
		capacity = (size/allocationSize) * allocationSize;
		if(size % allocationSize != 0)capacity += allocationSize;
		unsigned char *newData = new unsigned char[capacity];
		std::copy(data, data+length, newData);
		delete [] data;
		data = newData;
	}

	void clear()
	{
		length = 0;
	}

	Package subPackageFromCurrentPosition() const
	{
		Package p;
		p.append(data + iteratorPosition, length - iteratorPosition);
		return p;
	}

	void next(unsigned int size) const
	{
		if(iteratorPosition+size > length)return;
		iteratorPosition += size;
	}

	const unsigned int& getLength() const
	{
		return length;
	}

	bool empty() const
	{
		return (length == 0);
	}

	const unsigned char* getData() const
	{
		return data;
	}

	const unsigned int& getIteratorPosition() const
	{
		return iteratorPosition;
	}

	void resetIterator()
	{
		iteratorPosition = 0;
	}

	std::string toString() const
	{
		std::stringstream os;
		bool lastWasBinary = false;
		for(unsigned int i = 0; i < length; i++)
		{
			if(isalnum(data[i]))
			{
				if(lastWasBinary)os<<'.';
				os<<data[i];
				lastWasBinary = false;
			}
			else
			{
				if(!lastWasBinary)os<<'.';
				os<<std::hex<<int(data[i]);
				lastWasBinary = true;
			}
		}
		if(lastWasBinary)os<<'.';
		return os.str();
	}

private:
	template <class T>
	void append_internal(const T &t)
	{
		const unsigned int size = sizeof(T);
		if(length+size > capacity)reallocate(length + size);
		const unsigned char *dataToAppend = reinterpret_cast<const unsigned char*>(&t);
		std::copy(dataToAppend, dataToAppend+size, data+length);
		length += size;
	}

	template <class T>
	void append_internal(const T *t, unsigned int size)
	{
		if(length+size > capacity)reallocate(length + size);
		const unsigned char *dataToAppend = reinterpret_cast<const unsigned char*>(t);
		std::copy(dataToAppend, dataToAppend+size, data+length);
		length += size;
	}

	template <class T>
	bool getAndNext_internal(T &t) const
	{
		bool success = get_internal(t);
		next(sizeof(T));
		return success;
	}

	template <class T>
	bool getAndNext_internal(T *&t, unsigned int size) const
	{
		bool success = get_internal(t, size);
		next(size);
		return success;
	}

	template <class T>
	bool get_internal(T &t) const
	{
		unsigned int size = sizeof(T);
		if(iteratorPosition+size > length)return false;
		const void *tempData = reinterpret_cast<const void*>(data+iteratorPosition);
		t = *reinterpret_cast<const T*>(tempData);
		return true;
	}

	template <class T>
	bool get_internal(T *&t, unsigned int size) const
	{
		if(iteratorPosition+size > length)return false;
		std::copy(data+iteratorPosition, data+iteratorPosition+size, reinterpret_cast<unsigned char*>(t));
		return true;
	}

private:
	unsigned int allocationSize;
	unsigned char *data;
	unsigned int length;
	unsigned int capacity;
	mutable unsigned int iteratorPosition;

}; //end class package


/*-----	Functions for adding data to package	-----*/

template <class T>
inline void operator<<(Package &p, const T &t)
{
	p.append(t);
}

template <typename T, unsigned int N>
inline void operator<<(Package &p, const T (&t)[N])
{
	p.append(t, N);
}

template <class T>
 __attribute__ ((deprecated("Inserting a pointer into a package may not be what you want!")))
inline void operator<<(Package &p, const T *t)
{
	p.append(t, sizeof(t));
}

template <>
inline void operator<<(Package &p, const Package &t)
{
	p.append(t.getData(), t.getLength());
}

template <>
inline void operator<<(Package &p, const std::string &t)
{
	p<<t.size();
	p.append(t.c_str(), t.size());
}


/*-----	Functions for getting data from package	-----*/

template <class T>
inline bool operator>>(const Package &p, T &t)
{
	return p.getAndNext(t);
}

template <typename T, unsigned int N>
bool operator>>(const Package &p, T(&t)[N])
{
	for(unsigned int i = 0; i < N; i++)
	{
		if(!p.getAndNext(t[i]))return false;
	}
	return true;
}

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
