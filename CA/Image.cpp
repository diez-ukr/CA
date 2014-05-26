#include "Image.h"
#include <exception>
#include <sstream>
#include <thread>
#include <xutility>
#include <cmath>

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
					channelCoef1 = 0.114F;
					channelCoef2 = 0.587F;
					channelCoef3 = 0.299F;
				}
				else if (!strcmp("RGB", this->colorModel))
				{

					channelCoef1 = 0.299F;
					channelCoef2 = 0.587F;
					channelCoef3 = 0.114F;
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

	Image Image::binarization(Image &blurImage)
	{
		if (blurImage.height != height || blurImage.width != width)
			throw std::exception("Source image and blue image has different size.");
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
			workers.push_back(std::thread([k, &limits, this, &retval, &blurImage]()
			{
				int from = limits[k].first;
				int to = limits[k].second;
				for (int i = from; i < to; i++)
				{
					for (int j = 0; j < width; j++)
					{
						//if (this->get(i, this->nchannels * j) > blurImage.get(i, this->nchannels * j))
						if (static_cast<int>(this->get(i, this->nchannels * j)) * 1.2 > static_cast<int>(blurImage.get(i, this->nchannels * j)))
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

	void Image::outRawFile(std::string filename)
	{
		FILE *f = fopen(filename.c_str(), "w+");
		if (f)
		{
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width * nchannels; j++)
					fprintf(f, "%d ", get(i, j));
				fprintf(f, "\n");
			}
			fclose(f);
		}
	}

	std::vector<int> Image::buildHistogram()
	{
		Image *src = this;
		Image _src;
		if (nchannels == 3)
			src = &(Image(this->grayScale()));
		std::vector<int> retval(256);
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

		std::vector<std::vector<int>> partialHistograms(limits.size());

		std::vector<std::thread> workers;
		for (int k = 0; k < threadCount; k++)
		{
			workers.push_back(std::thread([k, &limits, this, &partialHistograms]()
			{
				std::vector<int> *partialHistogram = &partialHistograms[k];
				partialHistogram->resize(256);
				int from = limits[k].first;
				int to = limits[k].second;
				for (int i = from; i < to; i++)
				{
					for (int j = 0; j < width; j++)
					{
						partialHistogram->at(this->get(i, this->nchannels * j))++;
					}
				}
			}));
		}
		for (auto& thread : workers)
		{
			thread.join();
		}

		/*mergiing partial histograms*/

		for (unsigned i = 0; i < retval.size(); i++)
		{
			for (auto& partialHistogram : partialHistograms)
			{
				retval[i] += partialHistogram[i];
			}
		}

		return retval;
	}

	unsigned char Image::otsuThreshold()
	{
		std::vector<int> histogram = buildHistogram();
		int levelCount = histogram.size();
		int sum = 0;
		for (unsigned i = 1; i < levelCount; ++i)
			sum += i * histogram[i];
		int sumB = 0;
		int wB = 0;
		int wF = 0;
		int mB;
		int mF;
		double max = 0;
		double between;
		int threshold = 0;
		for (unsigned i = 0; i < levelCount; ++i) 
		{
			wB += histogram[i];
			if (wB == 0)
				continue;
			wF = base.size() - wB;
			if (wF == 0)
				break;
			sumB += i * histogram[i];
			mB = sumB / wB;
			mF = (sum - sumB) / wF;
			between = wB * wF * pow(static_cast<double>(mB - mF), 2.0);
			if (between > max) 
			{
				max = between;
				threshold = i;
			}
		}
		return static_cast<unsigned char>(threshold);
	}

	Image Image::binarizationGauss(unsigned char deltaThreshold, unsigned char threshold)
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
			workers.push_back(std::thread([k, &limits, this, &retval, &deltaThreshold, &threshold]()
			{
				int from = limits[k].first;
				int to = limits[k].second;
				unsigned char nei[8];
				for (int i = from; i < to; i++)
				{
					for (int j = 0; j < width; j++)
					{
						getNeighbourIntens(i, j, nei);
						unsigned char cur = this->get(i, this->nchannels * j);
						int bN = 0, wN = 0;
						int delta;
						for (auto k = 0; k < 8; k++)
						{
							delta = static_cast<int>(nei[k]) - cur;
							if (delta > deltaThreshold)
							{
								wN++;
								continue;
							}
							if (-delta > deltaThreshold)
							{
								bN++;
								continue;
							}
						}
						if (bN >= 3 && wN < bN)
						{
							retval.set(i, j, 255);
							continue;
						}
						if (wN >= 3 && wN > bN)
						{
							retval.set(i, j, 0);
							continue;
						}
						/*if (i != 0 && j != 0)
						{
							int backDelta = abs(static_cast<int>(nei[7]) - cur);
							int upDelta = abs(static_cast<int>(nei[1]) - cur);
							if (backDelta < upDelta && backDelta < deltaThreshold)
							{
								retval.set(i, j, retval.get(i, j - 1));
								continue;
							}
							if (backDelta >= upDelta && backDelta < deltaThreshold)
							{
								retval.set(i, j, retval.get(i - 1, j));
								continue;
							}
						}*/

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

	void Image::getNeighbourIntens(unsigned i, unsigned j, unsigned char *dst)
	{
		assert(NULL != dst);
		if (i == 0)
		{
			unsigned char curInt = get(i, j);
			if (j == 0)
			{
				dst[0] = curInt;
				dst[1] = curInt;
				dst[2] = curInt;
				dst[3] = get(i, j + 1);
				dst[4] = get(i + 1, j + 1);
				dst[5] = get(i + 1, j);
				dst[6] = curInt;
				dst[7] = curInt;
				return;
			}
			if (j == width - 1)
			{
				dst[0] = curInt;
				dst[1] = curInt;
				dst[2] = curInt;
				dst[3] = curInt;
				dst[4] = curInt;
				dst[5] = get(i + 1, j);
				dst[6] = get(i + 1, j - 1);
				dst[7] = get(i, j - 1);
				return;
			}
			dst[0] = curInt;
			dst[1] = curInt;
			dst[2] = curInt;
			dst[3] = get(i, j + 1);
			dst[4] = get(i + 1, j + 1);
			dst[5] = get(i + 1, j);
			dst[6] = get(i + 1, j - 1);
			dst[7] = get(i, j - 1);
			return;
		}

		if (i == height - 1)
		{
			unsigned char curInt = get(i, j);
			if (j == 0)
			{
				dst[0] = curInt;
				dst[1] = get(i - 1, j);
				dst[2] = get(i - 1, j + 1);
				dst[3] = get(i, j + 1);
				dst[4] = curInt;
				dst[5] = curInt;
				dst[6] = curInt;
				dst[7] = curInt;
				return;
			}
			if (j == width - 1)
			{
				dst[0] = get(i - 1, j - 1);
				dst[1] = get(i - 1, j);
				dst[2] = curInt;
				dst[3] = curInt;
				dst[4] = curInt;
				dst[5] = curInt;
				dst[6] = curInt;
				dst[7] = get(i, j - 1);
				return;
			}
			dst[0] = get(i - 1, j - 1);
			dst[1] = get(i - 1, j);
			dst[2] = get(i - 1, j + 1);
			dst[3] = get(i, j + 1);
			dst[4] = curInt;
			dst[5] = curInt;
			dst[6] = curInt;
			dst[7] = get(i, j - 1);
			return;
		}

		if (j == 0)
		{
			unsigned char curInt = get(i, j);
			dst[0] = curInt;
			dst[1] = get(i - 1, j);
			dst[2] = get(i - 1, j + 1);
			dst[3] = get(i, j + 1);
			dst[4] = get(i + 1, j + 1);
			dst[5] = get(i + 1, j);
			dst[6] = curInt;
			dst[7] = curInt;
			return;
		}		
		if (j == width - 1)
		{
			unsigned char curInt = get(i, j);
			dst[0] = get(i - 1, j - 1);
			dst[1] = get(i - 1, j);
			dst[2] = curInt;
			dst[3] = curInt;
			dst[4] = curInt;
			dst[5] = get(i + 1, j);
			dst[6] = get(i + 1, j - 1);
			dst[7] = get(i, j - 1);
			return;
		}
		dst[0] = get(i - 1, j - 1);
		dst[1] = get(i - 1, j);
		dst[2] = get(i - 1, j + 1);
		dst[3] = get(i, j + 1);
		dst[4] = get(i + 1, j + 1);
		dst[5] = get(i + 1, j);
		dst[6] = get(i + 1, j - 1);
		dst[7] = get(i, j - 1);
	}
}