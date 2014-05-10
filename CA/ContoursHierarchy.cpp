#include "ContoursHierarchy.h"


using namespace cv;
using namespace std;

namespace CA
{
	ContoursHierarchy::ContoursHierarchy()
	{
		root = nullptr;
	}

	ContoursHierarchy::ContoursHierarchy(std::vector<std::vector<cv::Point>> cvContours, std::vector<cv::Vec4i> cvHierarchy, int imgWidth, int imgHeigth)
	{
		root = nullptr;
		bool addOuterContour = false;
		int cvOutContorsCount = 0;
		if (cvHierarchy.size() == 0) addOuterContour = true;
		for (unsigned i = 0; i < cvHierarchy.size(); i++)
		{
			if (cvHierarchy[i][3] == -1)
			{
				cvOutContorsCount++;
				if (cvOutContorsCount == 2)
				{
					addOuterContour = true;
					break;
				}
				if (cvContours[i].size() != 4)
				{
					addOuterContour = true;
					break;
				}
				Point p1 = cvContours[i][0];
				Point p2 = cvContours[i][1];
				Point p3 = cvContours[i][2];
				Point p4 = cvContours[i][3];

				if (!(p1.x == 1 && p1.y == 1
					&& p2.x == 1 && p2.y == imgHeigth - 2
					&& p3.x == imgWidth - 2 && p3.y == imgHeigth - 2
					&& p4.x == imgWidth - 2 && p4.y == 1))
				{
					addOuterContour = true;
					break;
				}
			}
		}
		if (addOuterContour)
		{
			std::vector<cv::Point> newBase;
			newBase.push_back(cv::Point(0, 0));
			newBase.push_back(cv::Point(0, imgHeigth - 3));
			newBase.push_back(cv::Point(imgWidth - 3, imgHeigth - 3));
			newBase.push_back(cv::Point(imgWidth - 3, 0));
			this->root = new Contour(nullptr, std::vector<Contour*>(), std::vector<std::pair<int, int>>(), std::make_pair(1, 1));
			cvContour2CACountour(newBase, root);
		}

		std::vector<Contour *> hierachy(cvHierarchy.size());
		for (auto &contour : hierachy)
			contour = new Contour(nullptr, std::vector<Contour*>(), std::vector<std::pair<int, int>>());
		for (unsigned i = 0; i < cvHierarchy.size(); i++)
		{
			if (cvHierarchy[i][3] == -1)
			{
				if (addOuterContour)
				{
					cvContour2CACountour(cvContours[i], hierachy[i]);
					hierachy[i]->parent = root;
					root->addChild(hierachy[i]);
				}
				else
				{
					cvContour2CACountour(cvContours[i], hierachy[i]);
					root = hierachy[i];
				}
				continue;
			}
			int parentIndx = cvHierarchy[i][3];
			cvContour2CACountour(cvContours[i], hierachy[i]);
			hierachy[parentIndx]->addChild(hierachy[i]);
			hierachy[i]->parent = hierachy[parentIndx];
		}
	}

	ContoursHierarchy::~ContoursHierarchy()
	{
		if (root != nullptr)
			delete root;
	}

	void ContoursHierarchy::cvContour2CACountour(std::vector<cv::Point> cvContour, Contour* retval)
	{
		std::vector<std::pair<int, int>> base;
		std::pair<int, int> startPoint(cvContour[0].x, cvContour[0].y);
		for (unsigned i = 0; i < cvContour.size() - 1; i++)
		{
			base.push_back(std::make_pair(cvContour[i + 1].x - cvContour[i].x, cvContour[i + 1].y - cvContour[i].y));
		}
		base.push_back(std::make_pair(cvContour[0].x - cvContour[cvContour.size() - 1].x, cvContour[0].y - cvContour[cvContour.size() - 1].y));
		retval->setBase(base);
		retval->setStartPoint(startPoint);
		return;
	}

	Contour* ContoursHierarchy::getRoot()
	{
		return root;
	}
}