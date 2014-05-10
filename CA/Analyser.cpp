#include "Analyser.h"
#include "dirent.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>
#include "Logger.h"

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
		std::vector<std::pair<std::shared_ptr<Gauge>, std::pair<float, float>>> diffs(gauges.size());
		float max = 0;
		int maxIdx = 0;
		for (auto i = 0; i < gauges.size(); i++)
		{
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
		std::vector<Result> retval;
		for (int i = 0; i < contour->childs.size(); i++)
		{
			if (std::abs(contour->childs[i]->getArea()) > this->minimalArea)
			{
				std::vector<Result> childResult = analyse(contour->childs[i]);
				retval.insert(retval.end(), childResult.begin(), childResult.end());
			}
		}
		std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> nearest = getNearest(contour);
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
		return retval;
	}
}