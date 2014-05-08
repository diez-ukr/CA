#pragma once
#include <vector>

namespace CA
{
	class Contour
	{
		std::vector<std::pair<int, int>> base;
		std::pair<int, int> startPoint;
		void Initialize(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint);
	public:
		Contour *parent;
		std::vector<Contour*> childs;
		void addChild(Contour *child);
		void setBase(std::vector<std::pair<int, int>> &base);
		void setStartPoint(std::pair<int, int> &startPoint);
		Contour(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint = std::make_pair(-1, -1));
		Contour();
		~Contour();
	};
}
