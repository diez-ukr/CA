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
		std::vector<std::shared_ptr<Gauge>> gauges;
		int loadGauses(std::string &dirName);
		std::pair<std::shared_ptr<Gauge>, std::pair<float, float>> getNearest(Contour *contour);
		float mininalValidyty = 0.6;
		int minimalArea = 100;
		std::vector<Result> analyse(Contour *contour);
	public:
		Analyser(std::string &dirName);
		Analyser();
		~Analyser();
		std::vector<Result> analyse(ContoursHierarchy &hierachy);
	};

}

