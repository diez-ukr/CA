#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include "Image.h"
#include "ContoursHierarchy.h"
#include "Gauge.h"
#include "dirent.h"
#include <memory>
#include <cmath>
#include "Analyser.h"

using namespace cv;
using namespace std;


//hide the local functions in an anon namespace
namespace {

	int process(VideoCapture& capture) {

		CA::Analyser analyser(std::string("gauges1/"));

		/*{
			std::vector<std::pair<float, float>> acf1 = gauges[0]->contoursHierarchy->getRoot()->childs[0]->acf;
			std::vector<std::pair<float, float>> acf2 = gauges[1]->contoursHierarchy->getRoot()->childs[0]->acf;
			std::vector<std::pair<float, float>> acf3 = gauges[2]->contoursHierarchy->getRoot()->childs[0]->acf;

			std::pair<float, float> acfd1 = CA::Contour::akfDistance(
				*gauges[1]->contoursHierarchy->getRoot()->childs[0],
				*gauges[0]->contoursHierarchy->getRoot()->childs[0]
				);
			std::pair<float, float> acfd2 = CA::Contour::akfDistance(
				*gauges[1]->contoursHierarchy->getRoot()->childs[0],
				*gauges[2]->contoursHierarchy->getRoot()->childs[0]
				);

			std::pair<float, float> d1 = CA::Contour::maxInterrelationFunction(
				*gauges[1]->contoursHierarchy->getRoot()->childs[0],
				*gauges[0]->contoursHierarchy->getRoot()->childs[0]
				);
			std::pair<float, float> d2 = CA::Contour::maxInterrelationFunction(
				*gauges[1]->contoursHierarchy->getRoot()->childs[0],
				*gauges[2]->contoursHierarchy->getRoot()->childs[0]
				);

			cout << "1";
		}

		{
			std::map<float, std::string> diffs;
			for (auto i = 1; i < gauges.size(); i++)
			{
				std::pair<float, float> d = CA::Contour::maxInterrelationFunction(
					*gauges[0]->contoursHierarchy->getRoot()->childs[0],
					*gauges[i]->contoursHierarchy->getRoot()->childs[0]
				);
				cout << gauges[i]->gaugeCharacter << " " << d.first << " " << d.second << endl;
				//diffs(gauges[i]->gaugeCharacter, pow(d.first * d.first + d.second * d.second, 0.5)));
				diffs[pow(d.first * d.first + d.second * d.second, 0.5)] = gauges[i]->gaugeCharacter;
			}

		}*/
		


		int n = 0;
		char filename[200];
		string window_name = "video | q or esc to quit";
		cout << "press space to save a picture. q or esc to quit" << endl;
		namedWindow(window_name, CV_WINDOW_AUTOSIZE); //resizable window;
		Mat frame;
		for (;;) {
			static std::string mode = "norm";
			capture >> frame;
			if (frame.empty())
				break;



			Mat frameContrast = Mat::zeros(frame.size(), frame.type());

			for (int y = 0; y < frame.rows; y++)
			{
				for (int x = 0; x < frame.cols; x++)
				{
					for (int c = 0; c < 3; c++)
					{
						frameContrast.at<Vec3b>(y, x)[c] =
							saturate_cast<uchar>(2.0 * (frame.at<Vec3b>(y, x)[c]));
					}
				}
			}




			clock_t t1 = clock();
			IplImage ipl(frameContrast);
			/*cout << ((clock() - t1)) << endl; t1 = clock();
			{
				CA::Contour *a1 = gauges[0]->contoursHierarchy->getRoot()->childs[0];
				CA::Contour *a2 = gauges[1]->contoursHierarchy->getRoot()->childs[0];
				std::pair<float, float> mirlf = CA::Contour::maxInterrelationFunction(*a1, *a2);
				cout << mirlf.first;
			}*/
			//cout << "maxInterrelationFunction" << ((clock() - t1) ) << endl; t1 = clock();
			CA::Image caim_color(ipl);
			cout << ((clock() - t1) ) << endl; t1 = clock();
			CA::Image caim_grey(caim_color.grayScale());
			cout << ((clock() - t1) ) << endl; t1 = clock();

			int threshold = caim_grey.otsuThreshold();
			cout << ((clock() - t1)) << endl; t1 = clock();
			cout << "threshold = " << threshold << endl;
			CA::Image caim_bin(caim_grey.binarization(threshold));
			cout << ((clock() - t1) ) << endl; t1 = clock();

			Mat frameBin(cv::Size(caim_bin.width, caim_bin.height), caim_bin.getCvType(), caim_bin.getBase());
			
			
			vector<vector<Point>> cvContours;
			vector<Vec4i> cvHierarchy;

			//findContours(fim, cvContours, cvHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			Mat imageToFindContour = frameBin.clone();
			findContours(imageToFindContour, cvContours, cvHierarchy, RETR_TREE, CHAIN_APPROX_TC89_KCOS);

			CA::ContoursHierarchy ca_ch(cvContours, cvHierarchy, frameBin.cols, frameBin.rows);
			vector<CA::Result> resV = analyser.analyse(ca_ch);

			static int fps = 0;
			{ /*FPS*/
				static int sec = clock() / 1000;
				static int frameCount = 0;
				frameCount++;
				if (sec != clock() / 1000)
				{
					fps = frameCount;
					frameCount = 0;
					sec = clock() / 1000;
				}
			}

			{/*print FPS*/
				stringstream ss;
				ss << fps;
				ss.flush();
				Mat *matToType = &frameBin;
				putText(*matToType, ss.str(), Point(matToType->cols / 20, matToType->rows / 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(127, 127, 127));
			}

			{/*print Res*/
				for (CA::Result &res : resV)
				{
					stringstream ss;
					ss << res.gauge->gaugeCharacter;
					ss.flush();
					Mat *matToType = &frame;
					putText(*matToType, ss.str(), Point(res.contour->startPoint.first, res.contour->startPoint.second), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0));
				}
				
			}
			
			if (mode == "norm")
				imshow(window_name, frame);
			if (mode == "bin")
				imshow(window_name, frameBin);
			if (mode == "con")
				imshow(window_name, frameContrast);




			char key = (char)waitKey(5); //delay N millis, usually long enough to display and capture input
			switch (key) {
			case 'n':
			case 'N':
				mode = "norm";
				break;
			case 'b':
			case 'B':
				mode = "bin";
				break;
			case 'c':
			case 'C':
				mode = "con";
				break;
			case 'q':
			case 'Q':
			case 27: //escape key
				return 0;
			case ' ': //Save an image
				sprintf(filename, "filename%.3d.jpg", n++);
				imwrite(filename, frameBin);
				cout << "Saved " << filename << endl;
				break;
			default:
				break;
			}
		}
		return 0;
	}

}

int main(int ac, char** av) {

	
	VideoCapture capture(0);
	
	if (!capture.isOpened()) {
		cerr << "Failed to open a video device or video file!\n" << endl;
		return 1;
	}
	return process(capture);
}
