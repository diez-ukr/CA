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

	Image::Image(int nchannels, int depth, int height, int width, const char colorModel[])
	{
		this->Initalize(nchannels, depth, height, width, colorModel);
	}

	Image::Image(const IplImage &iplImage)
	{
		this->Initalize(iplImage.nChannels, iplImage.depth, iplImage.height, iplImage.width, iplImage.channelSeq);
		if (depth == 8)
		{
			std::vector<unsigned char> *b = new std::vector<unsigned char>();
			b->assign((unsigned char*)(iplImage.imageData), (unsigned char*)(iplImage.imageData) + nchannels * width * height);
			base = b;
		}
		else if (depth == 1)
		{
			std::vector<bool> *b = new std::vector<bool>();
			b->assign((bool*)(iplImage.imageData), (bool*)(iplImage.imageData) + nchannels * width * height);
			base = b;
		}

	}

	Image::Image(const Image &image)
	{
		strcpy(colorModel, image.colorModel);
		this->depth = image.depth;
		this->height = image.height;
		this->nchannels = image.nchannels;
		this->width = image.width;
		if (depth == 1)
		{
			assert(nchannels == 1);

			std::vector<bool> *src = (std::vector<bool> *)(image.base);
			std::vector<bool> *dst = new std::vector<bool>(src->begin(), src->end());
			base = dst;
		}
		else if (depth == 8)
		{
			std::vector<unsigned char> *src = (std::vector<unsigned char> *)(image.base);
			std::vector<unsigned char> *dst = new std::vector<unsigned char>(src->begin(), src->end());
			base = dst;
		}
	}


	Image::~Image()
	{
		if (depth == 1)
		{
			assert(nchannels == 1);
			delete base;
		}
		else if (depth == 8)
		{
			delete base;
		}
	}

	void Image::Initalize(int nchannels, int depth, int height, int width, const char colorModel[] /*= nullptr*/)
	{
		if (nchannels != 1 && nchannels != 3)
		{
			stringstream ss;
			ss << "Cannot create image. nchannels = " << nchannels << " is unsupported";
			ss.flush();
			throw exception(ss.str().c_str());
		}
		if (depth != 1 && depth != 8)
		{
			stringstream ss;
			ss << "Cannot create image. nchannels = " << nchannels << " is unsupported";
			ss.flush();
			throw exception(ss.str().c_str());
		}
		this->nchannels = nchannels;
		this->depth = depth;
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
		if (depth == 1)
		{
			assert(nchannels == 1);
			base = new std::vector<bool>(height * width * nchannels);
		}
		else if (depth == 8)
		{
			base = new std::vector<unsigned char>(height * width * nchannels);
		}
	}

	int Image::getCvType()
	{
		if (depth == 8 && nchannels == 1) return CV_8U;
		if (depth == 8 && nchannels == 3) return CV_8UC3;	
	}

	unsigned char Image::get(int i, int j)
	{
		if (depth == 8)
			return ((std::vector<unsigned char>*)base)->at(nchannels * width * i + j);
		if (depth == 1)
			return ((std::vector<bool>*)base)->at(nchannels * width * i + j);
		return 0;
	}

	void Image::set(int i, int j, const unsigned char &val)
	{
		if (depth == 8)
		{
			((std::vector<unsigned char>*)base)->at(nchannels * width * i + j) = val;
			return;
		}
		if (depth == 1)
		{
			((std::vector<bool>*)base)->at(nchannels * width * i + j) = val;
			return;
		}
		return;
	}

	Image Image::to8b()
	{
		if (depth == 8)
			return clone();
		Image retval(1, 8, height, width);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				retval.set(i, j, 255 * get(i, j));
			}
		}
		return retval;
	}

	Image Image::clone()
	{
		return Image(*this);
	}

	Image Image::grayScale()
	{
		if (nchannels == 1)
			return clone();
		Image retval(1, 8, height, width);
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
				for (int i = from; i < to; i++)
				{
					for (int j = 0; j < width; j ++)
					{
						int r, g, b;
						if (!strcmp("RGB", this->colorModel))
						{
							r = this->get(i, this->nchannels * j);
							g = this->get(i, this->nchannels * j + 1);
							b = this->get(i, this->nchannels * j + 2);
						}
						else if (!strcmp("BGR", this->colorModel))
						{
							b = this->get(i, this->nchannels * j);
							g = this->get(i, this->nchannels * j + 1);
							r = this->get(i, this->nchannels * j + 2);
						}
						unsigned char color = static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b);
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
		if (depth == 8)
		{
			std::vector<unsigned char> *b = (std::vector<unsigned char> *)base;
			return &((*b)[0]);
		}
	}

	Image Image::binarization(unsigned char threshold)
	{
		Image *src = this;
		Image _src;
		if (nchannels == 3)
			src = &(Image(this->grayScale()));
		Image retval(1, 1, height, width);
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
							retval.set(i, j, 1);
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