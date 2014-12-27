#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <algorithm>
#include <sstream>

namespace cluster
{

class Package
{

public:
	Package(unsigned int allocationSize=1024) :
		allocationSize(allocationSize),
		data(new unsigned char[allocationSize]),
		length(0),
		capacity(allocationSize),
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

	template <class T>
	void append(const T &t, unsigned int size)
	{
		if(length+size > capacity)reallocate(length + size);
		const unsigned char *dataToAppend = reinterpret_cast<const unsigned char*>(&t);
		std::copy(dataToAppend, dataToAppend+size, data+length);
		length += size;
	}

	template <class T>
	void append(const T *t, unsigned int size)
	{
		append(*t, size);
	}

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

	template <class T>
	bool getAndNext(T &t) const
	{
		bool success = get(t);
		next<T>();
		return success;
	}

	template <class T>
	bool getAndNext(T *&t, unsigned int size) const
	{
		bool success = get(t, size);
		next(size);
		return success;
	}

	template <class T>
	bool get(T &t) const
	{
		unsigned int size = sizeof(T);
		if(iteratorPosition+size > length)return false;
		const void *tempData = reinterpret_cast<const void*>(data+iteratorPosition);
		t = *reinterpret_cast<const T*>(tempData);
		return true;
	}

	template <class T>
	bool get(T *&t, unsigned int size) const
	{
		if(iteratorPosition+size > length)return false;
		std::copy(data+iteratorPosition, data+iteratorPosition+size, reinterpret_cast<unsigned char*>(t));
		return true;
	}

	template <class T>
	void next() const
	{
		unsigned int size = sizeof(T);
		next(size);
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
	unsigned int size = sizeof(t);
	p.append(t, size);
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
	unsigned int size = sizeof(t);
	p.append(t, size);
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
	return p.getAndNext<T>(t);
}

template <typename T, unsigned int N>
bool operator>>(const Package &p, T(&t)[N])
{
	for(unsigned int i = 0; i < N; i++)
	{
		if(!p.getAndNext<T>(t[i]))return false;
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
