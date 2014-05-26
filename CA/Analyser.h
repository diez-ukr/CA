#pragma once

#include <vector>
#include <memory>
#include "Gauge.h"

namespace CA
{
	struct Result
	{
		std::shared_ptr<Gauge> gauge;
		Contour *contour;
		std::pair<float, float> validity;
	};

	class Analyser
	{
	private:
		float com2float(std::pair<float, float> &p);
		std::vector<std::shared_ptr<Gauge>> gauges;
		int loadGauses(std::string &dirName);
		std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> getNearest(Contour *contour);
		std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> getNearest(Contour *contour, std::vector<std::shared_ptr<Gauge>> gauges);
		std::vector<std::shared_ptr<Gauge>> filterByAcf(Contour *contour, int leftCount);
		std::vector<std::shared_ptr<Gauge>> filterByAcf(Contour *contour, float leftPart);
		float mininalValidyty = static_cast<float>(0.6);
		std::vector<Result> analyse(Contour *contour);
	public:
		int minimalArea = 100;
		Analyser(std::string &dirName);
		Analyser();
		~Analyser();
		std::vector<Result> analyse(ContoursHierarchy &hierachy);
	};

}

