#include "Image.h"
#include <exception>
#include <sstream>
#include <thread>
#include <xutility>

using namespace std;

namespace CA
{

	Image::Image() 
	{
	}

	Image::Image(int nchannels, int height, int width, const char colorModel[])
	{
		this->Initalize(nchannels, height, width, colorModel);
	}

	Image::Image(const IplImage &iplImage)
	{
		this->Initalize(iplImage.nChannels, iplImage.height, iplImage.width, iplImage.channelSeq);
		base.assign((unsigned char*)(iplImage.imageData), (unsigned char*)(iplImage.imageData) + nchannels * width * height);
	}

	Image::Image(const Image &image)
	{
		strcpy(colorModel, image.colorModel);
		this->height = image.height;
		this->nchannels = image.nchannels;
		this->width = image.width;
		this->base = image.base;
	}


	Image::~Image()
	{

	}

	void Image::Initalize(int nchannels, int height, int width, const char colorModel[] /*= nullptr*/)
	{
		if (nchannels != 1 && nchannels != 3)
		{
			stringstream ss;
			ss << "Cannot create image. nchannels = " << nchannels << " is unsupported";
			ss.flush();
			throw exception(ss.str().c_str());
		}
		
		this->nchannels = nchannels;
		this->height = height;
		this->width = width;
		if (colorModel)
			strcpy(this->colorModel, colorModel);
		else
		{
			if (nchannels == 1)
				strcpy(this->colorModel, "GRE");
				else if (nchannels == 3)
				strcpy(this->colorModel, "BGR");
		}
		this->base.resize(nchannels * width * height);
	}

	int Image::getCvType()
	{
		if (nchannels == 1) return CV_8U;
		if (nchannels == 3) return CV_8UC3;	
	}

	unsigned char Image::get(int i, int j)
	{
		return base[nchannels * width * i + j];
	}

	void Image::set(int i, int j, const unsigned char &val)
	{
		base[nchannels * width * i + j] = val;
		return;
	}

	Image Image::clone()
	{
		return Image(*this);
	}

	Image Image::grayScale()
	{
		if (nchannels == 1)
			return clone();
		Image retval(1, height, width);
		std::vector<std::pair<int, int>> limits;
		int threadCount = 4;
		int stripSize = height / threadCount;
		int from = 0;
		for (int k = 0; k < threadCount - 1; k++)
		{
			limits.push_back(std::make_pair(from, from + stripSize));
			from += stripSize;
		}
		limits.push_back(std::make_pair(from, height));


		std::vector<std::thread> workers;
		for (int k = 0; k < threadCount; k++) 
		{
			workers.push_back(std::thread([k, &limits, this, &retval]()
			{
				int from = limits[k].first;
				int to = limits[k].second;
				float channelCoef1, channelCoef2, channelCoef3;
				if (!strcmp("BGR", this->colorModel))
				{
					channelCoef1 = 0.114;
					channelCoef2 = 0.587;
					channelCoef3 = 0.299;
				}
				else if (!strcmp("RGB", this->colorModel))
				{

					channelCoef1 = 0.299;
					channelCoef2 = 0.587;
					channelCoef3 = 0.114;
				}

				for (int i = from; i < to; i++)
				{
					for (int j = 0; j < width; j ++)
					{
						int ch1, ch2, ch3;
						ch1 = this->get(i, this->nchannels * j);
						ch2 = this->get(i, this->nchannels * j + 1);
						ch3 = this->get(i, this->nchannels * j + 2);
						unsigned char color = static_cast<unsigned char>(
							channelCoef1 * ch1
							+ channelCoef2 * ch2
							+ channelCoef3 * ch3
						);
						retval.set(i, j, color);
					}
				}
			}));
		}


		for (auto& thread : workers)
		{
			thread.join();
		}



		return retval;
	}

	void* Image::getBase()
	{
		return &(base[0]);
	}

	Image Image::binarization(unsigned char threshold)
	{
		Image *src = this;
		Image _src;
		if (nchannels == 3)
			src = &(Image(this->grayScale()));
		Image retval(1, height, width);
		std::vector<std::pair<int, int>> limits;
		int threadCount = 4;
		int stripSize = height / threadCount;
		int from = 0;
		for (int k = 0; k < threadCount - 1; k++)
		{
			limits.push_back(std::make_pair(from, from + stripSize));
			from += stripSize;
		}
		limits.push_back(std::make_pair(from, height));


		std::vector<std::thread> workers;
		for (int k = 0; k < threadCount; k++)
		{
			workers.push_back(std::thread([k, &limits, this, &retval, &threshold]()
			{
				int from = limits[k].first;
				int to = limits[k].second;
				for (int i = from; i < to; i++)
				{
					for (int j = 0; j < width; j++)
					{
						if (this->get(i, this->nchannels * j) > threshold)
							retval.set(i, j, 255);
						else
							retval.set(i, j, 0);
					}
				}
			}));
		}


		for (auto& thread : workers)
		{
			thread.join();
		}



		return retval;
	}
}