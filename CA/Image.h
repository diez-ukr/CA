#pragma once
#include <opencv2\highgui\highgui.hpp>

namespace CA
{
	class Image
	{
	private:
		std::vector<unsigned char> base;;
		void Initalize(int nchannels, int height, int width, const char colorModel[] = nullptr);
		std::vector<int> buildHistogram();
	public:
		int nchannels;
		int height;
		int width;
		char colorModel[4];
		Image(int nchannels, int height, int width, const char colorModel[] = nullptr);
		Image(const IplImage &iplImage);
		Image(const Image &image);
		Image();
		~Image();

		
		void* getBase();
		int getCvType();
		unsigned char get(int i, int j);
		void set(int i, int j, const unsigned char &val);
		Image binarization(unsigned char threshold);
		Image clone();
		Image grayScale();
		void outRawFile(std::string filename);
		unsigned char otsuThreshold();
	};
}
