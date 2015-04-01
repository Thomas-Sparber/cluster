/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTERCONTAINERFUNCTIONS_HPP
#define CLUSTERCONTAINERFUNCTIONS_HPP

namespace cluster
{

/**
  * This function is called internally by the
  * ClusterContainer to get an element
 **/
template<class Index, class T, class Container>
inline T& getObjectFromContainer(Container&, const Index&)
{
	static_assert(sizeof(T) == -1, "You need to override this function for your container");
	T *t = nullptr;
	return *t;
}

/**
  * This function is called internally by the
  * ClusterContainer to get an element
 **/
template<class Index, class T, class Container>
inline const T& getObjectFromContainer(const Container&, const Index&)
{
	static_assert(sizeof(T) == -1, "You need to override this function for your container");
	T *t = nullptr;
	return *t;
}

/**
  * This function is called internally by the
  * ClusterContainer to replace an element
 **/
template<class Index, class T, class Container>
inline void replaceObjectInContainer(Container&, const Index&, const T&)
{
	static_assert(sizeof(T) == -1, "You need to override this function for your container");
}

/**
  * This function is called internally by the
  * ClusterContainer to remove an element
 **/
template<class Index, class T, class Container>
inline void removeObjectFromContainer(Container&, const Index&)
{
	static_assert(sizeof(T) == -1, "You need to override this function for your container");
}

/****************** ClusterList **************************/

template<class Index, class T>
inline T& getObjectFromContainer(std::list<T> &l, const Index &i)
{
	auto it = l.begin();
	for(Index j = 0; j < i; ++j)++it;
	return (*it);
}

template<class Index, class T>
inline const T& getObjectFromContainer(const std::list<T> &l, const Index &i)
{
	auto it = l.begin();
	for(Index j = 0; j < i; ++j)++it;
	return (*it);
}

template<class Index, class T>
inline void replaceObjectInContainer(std::list<T> &l, const Index &i, const T &t)
{
	auto it = l.begin();
	for(Index j = 0; j < i; ++j)++it;
	(*it) = t;
}

template<class Index, class T>
inline void removeObjectFromContainer(std::list<T> &l, const Index &i)
{
	auto it = l.begin();
	for(Index j = 0; j < i; ++j)++it;
	l.erase(it);
}

/****************** ClusterVector **************************/

template<class Index, class T>
inline T& getObjectFromContainer(std::vector<T> &v, const Index &i)
{
	return v[i];
}

template<class Index, class T>
inline const T& getObjectFromContainer(const std::vector<T> &v, const Index &i)
{
	return v[i];
}

template<class Index, class T>
inline void replaceObjectInContainer(std::vector<T> &v, const Index &i, const T &t)
{
	v[i] = t;
}

template<class Index, class T>
inline void removeObjectFromContainer(std::vector<T> &v, const Index &i)
{
	v.erase(v.begin() + i);
}

} //end namespace cluster

#endif //CLUSTERCONTAINERFUNCTIONS_HPP
