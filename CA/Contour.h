#pragma once
#include <vector>

namespace CA
{
	struct Rect
	{
		std::pair<int, int> tl;
		std::pair<int, int> hw;
	}; 

	class Contour
	{
		std::vector<std::pair<int, int>> base;
		void Initialize(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint);
		std::vector<std::pair<float, float>> calculateACF();
		std::vector<std::pair<float, float>> acf;
		bool isAcfActual;
		void changeVectorCount(unsigned count = 40);
		void changeVectorCount1(unsigned count = 40);
	public:
		std::pair<int, int> startPoint;
		Contour *parent;
		std::vector<Contour*> childs;

		std::vector<std::pair<float, float>> getAcf();

		Contour(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint = std::make_pair(-1, -1));
		Contour();
		~Contour();

		float evLength(std::pair<float, float> ev);
		void addChild(Contour *child);
		void setBase(std::vector<std::pair<int, int>> &base);
		void setStartPoint(std::pair<int, int> &startPoint);
		float getArea();
		float getPerimeter();
		float getNorm();
		Rect getRect();
		
		
		static std::pair<int, int> scalarMultiply(Contour &l, Contour &r);
		static std::pair<float, float> normalScalarMultiply(Contour &l, Contour &r);
		static std::pair<int, int> interrelationFunction(Contour &l, Contour &r, int m);
		static std::pair<float, float> maxInterrelationFunction(Contour &l, Contour &r);
		static std::pair<float, float> akfDistance(Contour &l, Contour &r);
	};
}
