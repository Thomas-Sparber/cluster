/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/database/table.hpp>
#include <iostream>

using namespace std;
using namespace cluster;

namespace cluster
{

template <>
bool operator>>(const Package &p, Table &t)
{
	vector<Column> tempColumns;
	vector<Index> tempIndices;

	if(!(p>>tempColumns))return false;
	if(!(p>>tempIndices))return false;

	if(t.columns.empty())t.columns = tempColumns;
//	else if(t.columns != tempColumns) {} //TODO

	if(t.indices.empty())t.indices = tempIndices;
//	else if(t.indices != tempIndices) {} //TODO

	return true;
}

template <>
void operator<<(Package &p, const Table &t)
{
	p<<t.columns;
	p<<t.indices;
}

} //end namespace cluster

/********************************************/

Table::Table(const string &str_name, const string &str_folder) :
	name(str_name),
	folder(str_folder),
	columns(),
	indices(),
	localFile(),
	indexFile(),
	availableSpace()
{
	const string fileName = folder + "/" + str_name + ".table";
	localFile.open(fileName, ios_base::in | ios_base::out | ios_base::binary);

	//If the file didn't exist it is not created because fstream::in
	//is set in the constructor. The file needs to be reopened
	//without fstream::in.
	if(!localFile)
	{
		localFile.open(fileName, std::fstream::trunc | std::fstream::out | std::fstream::binary);
		localFile.close();

		localFile.open(fileName, std::fstream::in | std::fstream::out | std::fstream::binary);
	}
	if(!localFile)throw SQLException(string("Can't create table file ")+fileName);

	//Load columns from file
	const string columnsFileName = folder + "/" + name + ".cols";
	ifstream colFile(columnsFileName, ios_base::binary);
	if(colFile)
	{
		colFile.seekg(0, ios_base::end);
		std::size_t fileSize = std::size_t(colFile.tellg());
		colFile.seekg(0);
		vector<char> buffer(fileSize);
		if(!colFile.read(&buffer[0], fileSize))throw SQLException("Unable to read column file");
		Package pkg(buffer);
		pkg>>columns;
	}

	//Load indices from file
	const string indicesFileName = folder + "/" + name + ".indices";
	ifstream indFile(indicesFileName, ios_base::binary);
	if(indFile)
	{
		indFile.seekg(0, ios_base::end);
		std::size_t fileSize = std::size_t(indFile.tellg());
		indFile.seekg(0);
		vector<char> buffer(fileSize);
		if(!indFile.read(&buffer[0], fileSize))throw SQLException("Unable to read indices file");
		Package pkg(buffer);
		pkg>>indices;
	}

	//Load table index
	const string indexFileName = folder + "/" + name + ".index";
	indexFile.open(indexFileName, ios_base::in | ios_base::binary);
	long long start = 0;
	long long end = 0;
	vector<DataValue> row(columns.size());
	while(indexFile.read(reinterpret_cast<char*>(&start), sizeof(start)) && indexFile.read(reinterpret_cast<char*>(&end), sizeof(end)))
	{
		readRow(start, end, &row[0]);

		cout<<"Row loaded: ";
		bool first = true;
		for(const DataValue &v : row)
		{
			if(first)first = false;
			else cout<<",";
			cout<<v.toString();
		}
		cout<<endl;

		for(std::size_t i = 0; i < indices.size(); ++i)
		{
			const vector<std::size_t> &cols = indices[i].getColumns();
			vector<DataValue> indexColumns(cols.size());
			for(std::size_t j = 0; j < cols.size(); ++j)
			{
				indexColumns[j] = row[cols[j]];
			}
			indices[i].insert(indexColumns, start, end);
		}
	}
	indexFile.close();

	indexFile.open(indexFileName, std::fstream::out | std::fstream::binary | std::fstream::app);
	if(!indexFile)throw SQLException(string("Can't create table index file ")+indexFileName);
}

Table::~Table()
{
	localFile.close();
	indexFile.close();
}

void Table::addColumn(const Column &column)
{
	if(!column.notNull && !empty())throw SQLException("Can't add a non-null column to a table with data");

	if(!empty())
	{
		//TODO add column to existing data
	}
	columns.push_back(column);

	if(column.primaryKey)
	{
		bool found = false;
		for(Index &i : indices)
		{
			if(i.isForPrimary())
			{
				i.addColumn(columns.size()-1);
				found = true;
				break;
			}
		}
		if(!found)indices.push_back(Index(columns.size()-1, true));
	}
	else if(column.unique)indices.push_back(Index(columns.size()-1, false));

	//Save columns to file
	const string columnsFileName = folder + "/" + name + ".cols";
	ofstream colFile(columnsFileName, ios_base::binary | ios_base::trunc);
	Package pkg;
	pkg<<columns;
	colFile.write(pkg.getData(), pkg.getLength());

	//Save indices to file
	const string indicesFileName = folder + "/" + name + ".indices";
	ofstream indFile(indicesFileName, ios_base::binary | ios_base::trunc);
	Package pkg2;
	pkg2<<indices;
	indFile.write(pkg2.getData(), pkg2.getLength());
}

void Table::getInsertValues(const vector<string> &c, const string *values, vector<DataValue> &out)
{
	if(out.size() < columns.size())out.resize(columns.size());

	for(size_t i = 0; i < columns.size(); ++i)
	{
		out[i] = columns[i].type;

		size_t index = c.size();
		for(size_t j = 0; j < c.size(); ++j)
		{
			if(columns[i].name == c[j])
			{
				index = j;
				break;
			}
		}

		if(index == c.size())
		{
			if(columns[i].autoIncrement)out[i] = columns[i].nextAutoIncrementValue();
			else if(!columns[i].defaultValue.empty())out[i].setValue(columns[i].defaultValue);
			else if(!columns[i].autoIncrement)throw SQLException(string("Column ")+columns[i].name+" can't be null");
		}
		else
		{
			out[i].setValue(values[index]);
		}
	}
}

vector<DataValue> Table::insert(const vector<DataValue> &values)
{
	//Calculate row size in byte
	long long rowSize = 0;
	for(size_t i = 0; i < values.size(); ++i)rowSize += sizeof(bool) + values[i].getDataSize();

	//Look if a position within the file can be reused
	auto indexElement = availableSpace.end();
	for(auto it = availableSpace.begin(); it != availableSpace.end(); ++it)
	{
		if(it->end - it->begin >= rowSize)
		{
			indexElement = it;
			break;
		}
	}

	//Seek to position in file
	long long position = 0;
	if(indexElement == availableSpace.cend())
	{
		localFile.seekp(0, ios_base::end);
		position = localFile.tellp();
	}
	else
	{
		position = indexElement->begin;
		localFile.seekp(position);
		if(indexElement->end - indexElement->begin == rowSize)indexElement->begin = indexElement->end - rowSize;
		else availableSpace.erase(indexElement);
	}

	//Write data to disk
	for(size_t i = 0; i < values.size(); ++i)
	{
		const bool notNull = !values[i].isNull();
		localFile.write(reinterpret_cast<const char*>(&notNull), sizeof(notNull));
		values[i].writeTo(localFile);
	}

	vector<DataValue> dataInPrimaryKey;

	//Insert data into indices
	const long long end = position+rowSize;
	for(size_t i = 0; i < indices.size(); ++i)
	{
		const vector<size_t> &cols = indices[i].getColumns();
		vector<DataValue> indexColumns(cols.size());
		for(size_t j = 0; j < cols.size(); ++j)
		{
			indexColumns[j] = values[cols[j]];
		}
		if(!indices[i].insert(indexColumns, position, end))
		{
			throw SQLException("Constraint violation for index");
		}
		if(indices[i].isForPrimary())dataInPrimaryKey = indexColumns;
	}

	//Write to data file
	if(!indexFile.write(reinterpret_cast<const char*>(&position), sizeof(position)))throw SQLException("Can't write to index file");
	if(!indexFile.write(reinterpret_cast<const char*>(&end), sizeof(end)))throw SQLException("Can't write to index file");

	return dataInPrimaryKey;
}

void Table::select(const std::vector<Column> &cols, IndexIterator &out) const
{
	if(indices.empty())throw SQLException("Can't select data without an index");

	size_t index = indices.size();
	for(size_t i = 0; i < indices.size(); ++i)
	{
		if(indices[i].isForPrimary())
		{
			index = i;
			break;
		}
	}

	if(index == indices.size())index = 0;

	out.index = index;
	if(!empty())out.iterator = indices[index].begin()->first;
	out.columns = cols;
	out.finished = empty();
//	return IndexIterator(this, index, indices[index].begin().first, cols, empty());
}

void Table::selectNext(IndexIterator &it, DataValue *out, vector<DataValue> &key) const
{
	const Index &index = indices[it.index];
	auto iterator = index.find(it.iterator);

	if(iterator == index.end())
	{
		it.finished = true;
		return;
	}

	readRow(iterator->second.begin, iterator->second.end, out);
	key = iterator->first.data;

	++iterator;

	if(iterator == index.end())it.finished = true;
	else it.iterator = iterator->first;
}

void Table::select(const IndexElement &element, DataValue *out) const
{
	readRow(element.begin, element.end, out);
}

void Table::readRow(long long start, long long end, DataValue *out) const
{
	localFile.seekg(start);

	bool notNull;
	for(size_t i = 0; i < columns.size(); ++i)
	{
		out[i] = columns[i].type;
		localFile.read(reinterpret_cast<char*>(&notNull), sizeof(notNull));
		if(notNull)out[i].loadFrom(localFile);
	}

	if(localFile.tellg() != end)throw SQLException("Broken index");
}
