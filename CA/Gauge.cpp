#include "Gauge.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Image.h"

using namespace std;

namespace CA
{
	Gauge::Gauge(std::string filename)
	{
		contoursHierarchy = nullptr;
		cv::Mat mat;
		mat = cv::imread(filename);
		if (mat.empty())
			return;
		IplImage ipl(mat);
		Image col(ipl);
		Image grey(col.grayScale());
		Image bin(grey.binarization(grey.otsuThreshold()));
		cv::Mat matBin(cv::Size(bin.width, bin.height), bin.getCvType(), bin.getBase());
		vector<vector<cv::Point>> cvContours;
		vector<cv::Vec4i> cvHierarchy;
		findContours(matBin.clone(), cvContours, cvHierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		this->contoursHierarchy = new ContoursHierarchy(cvContours, cvHierarchy, matBin.cols, matBin.rows);
		this->gaugeCharacter = getGaugeCharacterFromFileName(filename);
	}


	Gauge::~Gauge()
	{
		if (contoursHierarchy)
			delete contoursHierarchy;
	}

	std::string Gauge::getGaugeCharacterFromFileName(std::string filename)
	{
		char gauge[255];
		_splitpath(filename.c_str(), nullptr, nullptr, gauge, nullptr);
		return std::string(gauge);
	}
}