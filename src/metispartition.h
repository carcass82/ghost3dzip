#pragma once
#ifndef METISPARTITION_H
#define METISPARTITION_H

#include <tr1/unordered_map>
#include <vector>

class MetisPartition
{
public:
	MetisPartition() : partCounter(0) {}

	void AddVertex(int originalIdx)
	{
		vList.push_back(originalIdx);
		ovList[originalIdx] = partCounter++;
	}

	void AddTriangle(int originalIdx)
	{
		fList.push_back(originalIdx);
	}

	int VertexCount() const
	{
		return vList.size();
	}

	int TriangleCount() const
	{
		return fList.size();
	}

	int GetVertexOriginalIndex(int partIndex) const
	{
		return vList[partIndex];
	}

	int GetVertexPartitionIndex(int origIndex) const
	{
		std::tr1::unordered_map<int, int>::const_iterator it;
		return ((it = ovList.find(origIndex)) != ovList.end())? (*it).second : -1;
	}

	int GetTriangleOriginalIndex(int partIndex) const
	{
		return fList[partIndex];
	}


private:
	int partCounter;
	std::vector<int> vList;
	std::vector<int> fList;
	std::tr1::unordered_map<int, int> ovList;
};

#endif // METISPARTITION_H
