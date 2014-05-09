#pragma once

#include "ContoursHierarchy.h"

namespace CA
{
	class Gauge
	{
	private:
		std::string getGaugeCharacterFromFileName(std::string filename);
	public:
		std::string gaugeCharacter;
		ContoursHierarchy *contoursHierarchy;
		Gauge(std::string filename);
		~Gauge();
	};
}

