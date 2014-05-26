#include "ContoursHierarchy.h"
#include <iostream>
#include <thread>


using namespace cv;
using namespace std;

namespace CA
{
	ContoursHierarchy::ContoursHierarchy()
	{
		root = nullptr;
	}

	ContoursHierarchy::ContoursHierarchy(std::vector<std::vector<cv::Point>> &cvContours, std::vector<cv::Vec4i> &cvHierarchy, int &imgWidth, int &imgHeigth)
	{
		//cout << "clock() = " << clock() << endl;
		root = nullptr;
		bool addOuterContour = false;
		int cvOutContorsCount = 0;
		if (cvHierarchy.size() == 0) addOuterContour = true;

		//time_t t1 = clock();

		std::vector<std::pair<int, int>> limits;
		int threadCount = 4;
		if (threadCount > cvContours.size())
			threadCount = cvContours.size();
		int stripSize = cvContours.size() / threadCount;
		int from = 0;
		for (int k = 0; k < threadCount - 1; k++)
		{
			limits.push_back(std::make_pair(from, from + stripSize));
			from += stripSize;
		}
		limits.push_back(std::make_pair(from, cvContours.size()));

		std::vector<std::vector<unsigned>> filteredIndxVectors(threadCount);


		std::vector<std::thread> workers;
		for (int k = 0; k < threadCount; k++)
		{
			workers.push_back(std::thread([k, &limits, this, &cvContours, &filteredIndxVectors]()
			{
				
				int from = limits[k].first;
				int to = limits[k].second;
				for (unsigned i = from; i < to; i++)
				{
					if (cvContours[i].size() > 2)
						filteredIndxVectors[k].push_back(i);
				}
			}));
		}
		for (auto& thread : workers)
		{
			thread.join();
		}

		std::vector<unsigned> filteredIndx(filteredIndxVectors[0]);
		for (auto i = 1; i < threadCount; i++)
		{
			filteredIndx.insert(filteredIndx.end(), filteredIndxVectors[i].begin(), filteredIndxVectors[i].end());
		}

		//cout << "filtering small contours: " << ((clock() - t1)) << endl; t1 = clock();

		for (unsigned i = 0; i < filteredIndx.size(); i++)
		{
			unsigned indx = filteredIndx[i];
			if (cvHierarchy[indx][3] == -1)
			{
				cvOutContorsCount++;
				if (cvOutContorsCount == 2)
				{
					addOuterContour = true;
					break;
				}
				if (cvContours[indx].size() != 4)
				{
					addOuterContour = true;
					break;
				}
				Point p1 = cvContours[indx][0];
				Point p2 = cvContours[indx][1];
				Point p3 = cvContours[indx][2];
				Point p4 = cvContours[indx][3];

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

		//cout << "cheching if need add outer contour: " << ((clock() - t1)) << endl; t1 = clock();

		if (addOuterContour)
		{
			std::vector<cv::Point> newBase(4);
			newBase[0] = (cv::Point(0, 0));
			newBase[1] = (cv::Point(0, imgHeigth - 3));
			newBase[2] = (cv::Point(imgWidth - 3, imgHeigth - 3));
			newBase[3] = (cv::Point(imgWidth - 3, 0));
			this->root = new Contour(nullptr, std::vector<Contour*>(), std::vector<std::pair<int, int>>(), std::make_pair(1, 1));
			cvContour2CACountour(newBase, root);
		}

		std::vector<Contour *> hierachy(filteredIndx.size());
		for (auto &contour : hierachy)
			contour = new Contour(nullptr, std::vector<Contour*>(), std::vector<std::pair<int, int>>());
		for (unsigned i = 0; i < filteredIndx.size(); i++)
		{
			unsigned indx = filteredIndx[i];
			int posInHiearchyVector = find(filteredIndx.begin(), filteredIndx.end(), indx) - filteredIndx.begin();
			if (cvHierarchy[indx][3] == -1)
			{
				if (addOuterContour)
				{
					cvContour2CACountour(cvContours[indx], hierachy[posInHiearchyVector]);
					hierachy[posInHiearchyVector]->parent = root;
					root->addChild(hierachy[posInHiearchyVector]);
				}
				else
				{
					cvContour2CACountour(cvContours[indx], hierachy[posInHiearchyVector]);
					root = hierachy[posInHiearchyVector];
				}
				continue;
			}
			int parentIndx = cvHierarchy[indx][3];
			int parent–osInHiearchyVector = find(filteredIndx.begin(), filteredIndx.end(), parentIndx) - filteredIndx.begin();
			
			cvContour2CACountour(cvContours[indx], hierachy[posInHiearchyVector]);
			hierachy[parent–osInHiearchyVector]->addChild(hierachy[posInHiearchyVector]);
			hierachy[posInHiearchyVector]->parent = hierachy[parent–osInHiearchyVector];
		}

		//cout << "creating CA hierarchy from filtered: " << ((clock() - t1)) << endl; t1 = clock();
		//cout << "clock() = " << clock() << endl;
	}

	ContoursHierarchy::~ContoursHierarchy()
	{
		if (root != nullptr)
			delete root;
	}

	void ContoursHierarchy::cvContour2CACountour(std::vector<cv::Point> &cvContour, Contour* retval)
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