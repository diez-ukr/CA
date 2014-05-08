#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include "Image.h"
#include "ContoursHierarchy.h"

using namespace cv;
using namespace std;


//hide the local functions in an anon namespace
namespace {

	int process(VideoCapture& capture) {
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

			Mat fim;
			fim = imread("test.jpg", CV_LOAD_IMAGE_COLOR);
			IplImage ipl(fim);

			clock_t t1 = clock();
			//IplImage ipl(frame);
			cout << ((clock() - t1) ) << endl; t1 = clock();
			CA::Image caim_color(ipl);
			cout << ((clock() - t1) ) << endl; t1 = clock();
			CA::Image caim_grey(caim_color.grayScale());
			cout << ((clock() - t1) ) << endl; t1 = clock();

			int threshold = caim_grey.otsuThreshold();
			cout << ((clock() - t1)) << endl; t1 = clock();

			CA::Image caim_bin(caim_grey.binarization(threshold));
			cout << ((clock() - t1) ) << endl; t1 = clock();

			Mat frameBin(cv::Size(caim_bin.width, caim_bin.height), caim_bin.getCvType(), caim_bin.getBase());
			
			
			vector<vector<Point>> cvContours;
			vector<Vec4i> cvHierarchy;

			//findContours(fim, cvContours, cvHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			findContours(frameBin, cvContours, cvHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			CA::ContoursHierarchy ca_ch(cvContours, cvHierarchy, frameBin.cols, frameBin.rows);

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
			
			if (mode == "norm")
				imshow(window_name, frame);
			if (mode == "bin")
				imshow(window_name, frameBin);




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
