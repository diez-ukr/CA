#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "Contour.h"

namespace CA
{
	class ContoursHierarchy
	{
	private:
		Contour *root;
		void cvContour2CACountour(std::vector<cv::Point> cvContour, Contour* retval);
	public:
		Contour* getRoot();
		ContoursHierarchy(std::vector<std::vector<cv::Point>> cvContours, std::vector<cv::Vec4i> cvHierarchy, int imgWidth, int imgHeigth);
		ContoursHierarchy();
		~ContoursHierarchy();
	};
}