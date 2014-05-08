#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "Image.h"
#include <time.h>

using namespace cv;
using namespace std;



//hide the local functions in an anon namespace
namespace {

	int process(VideoCapture& capture) {
		int n = 0;
		char filename[200];
		string window_name = "video | q or esc to quit";
		cout << "press space to save a picture. q or esc to quit" << endl;
		namedWindow(window_name, CV_WINDOW_KEEPRATIO); //resizable window;
		Mat frame;
		for (;;) {
			capture >> frame;
			if (frame.empty())
				break;

			clock_t t1 = clock();
			IplImage ipl(frame);
				cout << (((float)clock() - (float)t1) / 1000000.0F) * 1000 << endl; t1 = clock();
			CA::Image caim_color(ipl);
				cout << (((float)clock() - (float)t1) / 1000000.0F) * 1000 << endl; t1 = clock();
			CA::Image caim_grey(caim_color.grayScale());
				cout << (((float)clock() - (float)t1) / 1000000.0F) * 1000 << endl; t1 = clock();
			CA::Image caim_bin(caim_grey.binarization(127));
				cout << (((float)clock() - (float)t1) / 1000000.0F) * 1000 << endl; t1 = clock();
			CA::Image caim_bin_8b(caim_bin.to8b());
				cout << (((float)clock() - (float)t1) / 1000000.0F) * 1000 << endl; t1 = clock();

			/*{
				for (int i = 0; i < caim_grey.height; i++)
				for (int j = 0; j < caim_grey.width; j++)
					caim_grey.set(i, j, caim_grey.get(i, j));

			}*/

			Mat frame1(cv::Size(caim_bin_8b.width, caim_bin_8b.height), caim_bin_8b.getCvType(), caim_bin_8b.getBase());

			/*{
				FILE *f = fopen("of.txt", "w+");
				if (f)
				{
					for (int i = 0; i < caim_color.height; i++)
					{
						for (int j = 0; j < caim_color.width; j+=3)
							fprintf(f, "%d ", caim_color.get(i, j));
						fprintf(f, "\n");
					}
					fclose(f);
				}
			}*/

			imshow(window_name, frame1);

			/*for (;;)
			{
				Mat fim;
				fim = imread("test.jpg", CV_LOAD_IMAGE_COLOR);
				IplImage ipl(fim);
				CA::Image caim_color(ipl);
				//CA::Image caim_grey = caim_color.clone();
				CA::Image caim_grey(caim_color.grayScale());
				Mat frame1(cv::Size(caim_grey.width, caim_grey.height), caim_grey.getCvType(), caim_grey.getBase());
				imshow(window_name, frame1);	
				waitKey(0);
			}*/



			char key = (char)waitKey(5); //delay N millis, usually long enough to display and capture input
			switch (key) {
			case 'q':
			case 'Q':
			case 27: //escape key
				return 0;
			case ' ': //Save an image
				sprintf(filename, "filename%.3d.jpg", n++);
				imwrite(filename, frame1);
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
