#include "Analyser.h"
#include "dirent.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>
#include "Logger.h"
#include <thread>

using namespace std;

namespace CA
{
	Analyser::Analyser()
	{
	}

	Analyser::Analyser(std::string &dirName)
	{
		int n = loadGauses(dirName);
		stringstream ss;
		ss << "Loaded " << n << " gauges";
		Logger::logEvent(ss.str());
	}

	Analyser::~Analyser()
	{
	}

	int Analyser::loadGauses(std::string &dir)
	{
		DIR *dp;
		struct dirent *dirp;
		struct stat filestat;
		int loadCount = 0;

		dp = opendir(dir.c_str());
		while (dp && (dirp = readdir(dp)))
		{
			string filepath;
			filepath = dir + "/" + dirp->d_name;
			// If the file is a directory (or is in some way invalid) we'll skip it 
			if (stat(filepath.c_str(), &filestat)) continue;
			if (S_ISDIR(filestat.st_mode))         continue;
			char ext[255];
			_splitpath(filepath.c_str(), nullptr, nullptr, nullptr, ext);
			std::string s_ext(ext);
			if (s_ext != ".bmp" && s_ext != ".BMP" && s_ext != ".jpg" && s_ext != ".JPG") continue;
			gauges.push_back(std::shared_ptr<CA::Gauge>(new CA::Gauge(filepath)));
			loadCount++;
		}
		closedir(dp);
		/*std::sort(gauges.begin(), gauges.end(), [this](std::shared_ptr<CA::Gauge> g1, std::shared_ptr<CA::Gauge> g2)
		{
			std::pair<float, float> p1 = g1->contoursHierarchy->getRoot()->childs[0]->getAcf()[1];
			std::pair<float, float> p2 = g2->contoursHierarchy->getRoot()->childs[0]->getAcf()[1];
			bool retval = (com2float(p1) > com2float(p2));
			return retval;
		}
		);*/
		return loadCount;
	}

	std::vector<Result> Analyser::analyse(ContoursHierarchy &hierachy)
	{
		std::vector<Result> retval;
		retval = analyse(hierachy.getRoot());
		return retval;
	}

	std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> Analyser::getNearest(Contour *contour)
	{
		return getNearest(contour, gauges);
	}

	std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> Analyser::getNearest(Contour *contour, std::vector<std::shared_ptr<Gauge>> gauges)
	{
		std::vector<std::pair<std::shared_ptr<Gauge>, std::pair<float, float>>> diffs(gauges.size());
		float max = 0;
		int maxIdx = 0;
		for (auto i = 0; i < gauges.size(); i++)
		{
			gauges[i]->contoursHierarchy->getRoot()->childs[0]->getAcf();
			std::pair<float, float> d = Contour::maxInterrelationFunction(
				*gauges[i]->contoursHierarchy->getRoot()->childs[0],
				*contour
				);
			float cur = pow(d.first * d.first + d.second * d.second, 0.5);
			if (cur > max)
			{
				maxIdx = i;
				max = cur;
			}
			diffs[i] = std::make_pair(gauges[i], d);
		}
		return diffs[maxIdx];
	}
	
	std::vector<Result> Analyser::analyse(Contour *contour)
	{
		//time_t t1 = clock();
		std::vector<std::shared_ptr<Gauge>> filteredGauges = filterByAcf(contour, 0.1F);
		//cout << "acf filtering: " << ((clock() - t1)) << endl; t1 = clock();
		std::vector<Result> retval;
		if (contour->childs.size())
		{
			std::vector<std::pair<int, int>> limits;
			int threadCount = 4;
			if (threadCount > contour->childs.size())
				threadCount = contour->childs.size();
			int stripSize = contour->childs.size() / threadCount;
			int from = 0;
			for (int k = 0; k < threadCount - 1; k++)
			{
				limits.push_back(std::make_pair(from, from + stripSize));
				from += stripSize;
			}
			limits.push_back(std::make_pair(from, contour->childs.size()));

			std::vector<std::vector<Result>> resutls(threadCount);


			std::vector<std::thread> workers;
			for (int k = 0; k < threadCount; k++)
			{
				workers.push_back(std::thread([k, &limits, this, contour, &resutls]()
				{

					int from = limits[k].first;
					int to = limits[k].second;
					for (unsigned i = from; i < to; i++)
					{
						if (std::abs(contour->childs[i]->getArea()) > this->minimalArea)
						{
							std::vector<Result> childResult = analyse(contour->childs[i]);
							resutls[k].insert(resutls[k].end(), childResult.begin(), childResult.end());
						}
					}
				}));
			}

			for (auto& thread : workers)
			{
				thread.join();
			}

			for (auto i = 0; i < threadCount; i++)
			{
				retval.insert(retval.end(), resutls[i].begin(), resutls[i].end());
			}
		}
		//t1 = clock();
		std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> nearest = getNearest(contour, filteredGauges);
		//contour->getAcf();
		Result res;
		res.contour = contour;
		res.gauge = nearest.first;
		res.validity = nearest.second;
		float fValidity = pow(res.validity.first * res.validity.first + res.validity.second * res.validity.second, 2);
		if (fValidity > mininalValidyty)
		{
			retval.push_back(res);
			//cout << "find: " << res.gauge->gaugeCharacter << " (" << contour->startPoint.first << ", " <<
//				contour->startPoint.second << ")" << endl;
		
			//system("pause");
		}
		//cout << "recognizing by " << filteredGauges.size() << " gauges: " << ((clock() - t1)) << endl; t1 = clock();
		return retval;

	}

	std::vector<std::shared_ptr<Gauge>> Analyser::filterByAcf(Contour *contour, int leftCount)
	{
		if (leftCount > gauges.size())
			return gauges;
		std::vector<std::shared_ptr<Gauge>> retval(leftCount);
		std::vector<std::pair<float, float>> acf = contour->getAcf();
		//std::vector<std::pair<std::shared_ptr<Gauge>, float>> dist_stat(gauges.size());



		std::vector<std::pair<int, int>> limits;
		int threadCount = 1;
		if (threadCount > gauges.size())
			threadCount = gauges.size();
		int stripSize = gauges.size() / threadCount;
		int from = 0;
		for (int k = 0; k < threadCount - 1; k++)
		{
			limits.push_back(std::make_pair(from, from + stripSize));
			from += stripSize;
		}
		limits.push_back(std::make_pair(from, gauges.size()));

		std::vector<std::vector<std::pair<std::shared_ptr<Gauge>, float>>> distanceStatisticParts(threadCount);


		std::vector<std::thread> workers;
		for (int k = 0; k < threadCount; k++)
		{
			workers.push_back(std::thread([k, &limits, this, &distanceStatisticParts, &acf]()
			{

				int from = limits[k].first;
				int to = limits[k].second;
				for (auto i = from; i < to; i++)
				{

					float dist = 0;
					std::vector<std::pair<float, float>> gacf = (gauges[i]->contoursHierarchy->getRoot()->childs[0]->getAcf());
					for (auto j = 0; j < acf.size(); j++)
					{
						std::pair<float, float> pd(acf[j].first - gacf[j].first, acf[j].second - gacf[j].second);
						dist += pd.first * pd.first + pd.second * pd.second;
					}
					dist = pow(dist, 0.5F);
					distanceStatisticParts[k].push_back(std::make_pair(gauges[i], dist));
				}
			}));
		}
		for (auto& thread : workers)
		{
			thread.join();
		}

		std::vector<std::pair<std::shared_ptr<Gauge>, float>> distanceStatistic(distanceStatisticParts[0]);
		for (auto i = 1; i < threadCount; i++)
		{
			distanceStatistic.insert(distanceStatistic.end(), distanceStatisticParts[i].begin(), distanceStatisticParts[i].end());
		}

		std::sort(distanceStatistic.begin(), distanceStatistic.end(), [](
			std::pair<std::shared_ptr<Gauge>, float> r1,
			std::pair<std::shared_ptr<Gauge>, float> r2
			) { return r1.second < r2.second; }
		);

		/*for (auto i = 0; i < dist_stat.size(); i++)
		{
			cout << dist_stat[i].first << ": delta = " << dist_stat[i].second << endl;
		}*/
		for (auto i = 0; i < leftCount; i++)
		{
			retval[i] = distanceStatistic[i].first;
		}
		return retval;
	}

	std::vector<std::shared_ptr<Gauge>> Analyser::filterByAcf(Contour *contour, float leftPart)
	{
		return filterByAcf(contour, static_cast<int>(gauges.size() * leftPart));
	}


	float Analyser::com2float(std::pair<float, float> &p)
	{
		float module = pow(p.first * p.first + p.second * p.second, 0.5F);
		float fi = acos(p.first / module);
		/*if (p.second < 0)
			fi += 3.1415F;*/
		return module * pow(2.718F, fi);
	}
}