#pragma once
#include <opencv2\highgui\highgui.hpp>

namespace CA
{
	class Image
	{
	private:
		void *base;
		void Initalize(int nchannels, int bpp, int height, int width, const char colorModel[] = nullptr);
	public:
		int nchannels;
		int depth;
		int height;
		int width;
		char colorModel[4];
		Image(int nchannels, int depth, int height, int width, const char colorModel[] = nullptr);
		Image(const IplImage &iplImage);
		Image(const Image &image);
		Image();
		~Image();
		//Image& operator=(Image& from);

		
		void* getBase();
		int getCvType();
		unsigned char get(int i, int j);
		void set(int i, int j, const unsigned char &val);
		Image to8b();
		Image binarization(unsigned char threshold);
		Image clone();
		Image grayScale();
	};
}
