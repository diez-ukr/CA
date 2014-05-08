#include "Contour.h"

namespace CA
{
	Contour::Contour()
	{
	}


	Contour::Contour(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint /*= std::make_pair(-1, -1)*/)
	{
		Initialize(parent, childs, base, startPoint);
	}
	Contour::~Contour()
	{
		for (auto &child : childs)
		{
			delete child;
		}
	}

	void Contour::Initialize(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint)
	{
		this->parent = parent;
		this->childs = childs;
		this->base = base;
		this->startPoint = startPoint;
	}

	void Contour::addChild(Contour *child)
	{
		childs.push_back(child);
	}

	void Contour::setBase(std::vector<std::pair<int, int>> &base)
	{
		this->base = base;
	}

	void Contour::setStartPoint(std::pair<int, int> &startPoint)
	{
		this->startPoint = startPoint;
	}
}