#include "Contour.h"
#include <assert.h>
#include <cmath>

namespace CA
{
	Contour::Contour()
	{
	}


	Contour::Contour(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint /*= std::make_pair(-1, -1)*/)
	{
		Initialize(parent, childs, base, startPoint);
	}
	Contour::~Contour()
	{
		for (auto &child : childs)
		{
			delete child;
		}
	}

	void Contour::Initialize(Contour *parent, std::vector<Contour*> childs, std::vector<std::pair<int, int>> &base, std::pair<int, int> startPoint)
	{
		this->parent = parent;
		this->childs = childs;
		this->base = base;
		this->startPoint = startPoint;
	}

	void Contour::addChild(Contour *child)
	{
		childs.push_back(child);
	}

	void Contour::setBase(std::vector<std::pair<int, int>> &base)
	{
		this->base = base;
	}

	void Contour::setStartPoint(std::pair<int, int> &startPoint)
	{
		this->startPoint = startPoint;
	}

	float Contour::getArea()
	{
		if (base.size() <= 2)
			return 0;
		int x = 0, y = 0;
		double retval = 0;
		for (unsigned i = 0; i < base.size(); i++)
		{
			int x1 = x + base[i].first;
			int y1 = y + base[i].second;
			retval += static_cast<double>((y1 + y) * (x1 - x)) / 2;
			x = x1;
			y = y1;
		}
		return static_cast<int>(retval);
	}

	std::pair<int, int> Contour::scalarMultiply(Contour &l, Contour &r)
	{
		return interrelationFunction(l, r, 0);
	}

	float Contour::getNorm()
	{
		float retval = 0;
		for (auto i = 0; i < base.size(); i++)
		{
			retval += base[i].first * base[i].first + base[i].second * base[i].second;
		}

		return pow(retval, 0.5);
	}

	std::pair<float, float> Contour::normalScalarMultiply(Contour &l, Contour &r)
	{
		std::pair<float, float> retval = scalarMultiply(l, r);
		float denominator = l.getNorm() * r.getNorm();
		retval.first /= denominator;
		retval.second /= denominator;
		return retval;
	}

	std::pair<int, int> Contour::interrelationFunction(Contour &l, Contour &r, int m)
	{
		//if (r.base.size() != l.base.size())
		{
			l.changeVectorCount();
			r.changeVectorCount();
		}
		std::pair<int, int> retval(0, 0);
		for (auto i = 0; i < r.base.size(); i++)
		{	
			auto indxR = (i + m) % r.base.size();
			retval.first += l.base[i].first * r.base[indxR].first + l.base[i].second * r.base[indxR].second;
			retval.second += r.base[indxR].first * l.base[i].second - r.base[indxR].second * l.base[i].first;
		}
		return retval;
	}

	std::pair<float, float> Contour::maxInterrelationFunction(Contour &l, Contour &r)
	{
		//if (r.base.size() != l.base.size())
		{
			l.changeVectorCount();
			r.changeVectorCount();
		}
		float denominator = l.getNorm() * r.getNorm();
		std::vector<std::pair<int, int>> irf(r.base.size());
		for (unsigned i = 0; i < l.base.size(); i++)
		{
			irf[i] = interrelationFunction(l, r, i);
		}
		std::pair<float, float> max;
		max.first = static_cast<float>(irf[0].first) / denominator;
		max.second = static_cast<float>(irf[0].second) / denominator;
		float maxModule = max.first * max.first + max.second * max.second;
		for (unsigned i = 1; i < l.base.size(); i++)
		{
			std::pair<float, float> tmp;
			tmp.first = static_cast<float>(irf[i].first) / denominator;
			tmp.second = static_cast<float>(irf[i].second) / denominator;
			float module = tmp.first * tmp.first + tmp.second * tmp.second;
			if (module > maxModule)
			{
				max = tmp;
				maxModule = module;
			}
		}
		return max;
	}

	void Contour::changeVectorCount1(unsigned count)
	{
		if (base.size() == 0)
			return;
		if (base.size() == count)
			return;
		int LCM;
		{ /*LCM calculation*/
			int a, b, c;
			a = base.size();
			b = count;
			c = a * b;
			while (a != b)
			{
				if (a > b)
					a = a - b;
				else
					b = b - a;
			}
			LCM = c / a;
		}

		std::vector<std::pair<float, float>> i_f_base(LCM);

		for (auto i = 0; i < base.size(); i++)
		{
			std::pair<float, float> ev = static_cast<std::pair<float, float>>(base[i]);
			unsigned partCount = LCM / base.size();
			for (auto j = 0; j < partCount; j++)
			{
				i_f_base[i * partCount + j] = std::pair<float, float>(ev.first / partCount, ev.second / partCount);
			}
		}

		std::vector<std::pair<int, int>> newBase(count);

		int lastEvX = 0;
		int lastEvY = 0;

		for (auto i = 0; i < newBase.size() - 1; i++)
		{
			unsigned partCount = LCM / count;
			std::pair<float, float> ev(0, 0);
			for (auto j = 0; j < partCount; j++)
			{
				ev.first += i_f_base[i * partCount + j].first;
				ev.second += i_f_base[i * partCount + j].second;
			}
			newBase[i] = static_cast<std::pair<int, int>>(ev);
			lastEvX += newBase[i].first;
			lastEvY += newBase[i].second;
		}

		newBase.back() = std::pair<int, int>(-lastEvX, -lastEvY);

		this->base = newBase;
	}

	std::vector<std::pair<float, float>> Contour::calculateACF()
	{
		unsigned akfSize = floor(static_cast<float>(base.size() + 1.0) / 2);
		std::vector<std::pair<float, float>> retval(akfSize);
		if (!akfSize) return retval;
		float norm = this->getNorm();
		norm *= norm;
		for (auto i = 0; i < akfSize; i++)
		{
			retval[i] = interrelationFunction(*this, *this, i);
			retval[i].first /= norm;
			retval[i].second /= norm;
		}
		return retval;
	}

	std::pair<float, float> Contour::akfDistance(Contour &l, Contour &r)
	{
		std::pair<float, float> retval(0, 0);
		for (auto i = 0; i < l.acf.size(); i++)
		{
			retval.first += l.acf[i].first - r.acf[i].first;
			retval.second += l.acf[i].second - r.acf[i].second;
		}
		return retval;
	}

	std::vector<std::pair<float, float>> Contour::getAcf()
	{
		if (!isAcfActual)
		{
			calculateACF();
			isAcfActual = true;
		}
		return acf;
	}

	void Contour::changeVectorCount(unsigned count /*= 40*/)
	{
		if (base.size() == 0)
			return;
		if (base.size() == count)
			return;

		std::vector<std::pair<float, float>> f_base(base.size());
		for (auto i = 0; i < base.size(); i++)
		{
			f_base[i] = static_cast<std::pair<float, float>>(base[i]);
		}


		float perfectLength = getPerimeter() / count;
		std::vector<std::pair<int, int>> new_base(0);

		float curLength = 0;

		std::pair<int, int> lastEv(0, 0);

		std::pair<float, float> newEv(0, 0);
		for (unsigned i = 0; i < f_base.size(); i++)
		{
			float evLen = evLength(f_base[i]);
			if (curLength + evLen < perfectLength)
			{
				curLength += evLen;
				newEv.first += f_base[i].first;
				newEv.second += f_base[i].second;
				continue;
			}
			float neededLenght = perfectLength - curLength;
			float proportion = neededLenght / evLen;
			std::pair<float, float> part1, part2;
			part1.first = f_base[i].first * proportion;
			part1.second = f_base[i].second * proportion;
			part2.first = f_base[i].first - part1.first;
			part2.second = f_base[i].second - part1.second;

			newEv.first += part1.first;
			newEv.second += part1.second;
			std::pair<int, int> iEv = std::make_pair(round(newEv.first), round(newEv.second));
			lastEv.first -= iEv.first;
			lastEv.second -= iEv.second;
			new_base.push_back(iEv);
			if (new_base.size() == count - 1)
				break;


			if (i != f_base.size())
				f_base.insert(f_base.begin() + i + 1, part2);
			else
				f_base.push_back(part2);

			curLength = 0;
			newEv.first = 0;
			newEv.second = 0;
		}
		new_base.push_back(lastEv);
		base = new_base;

		return;

	}

	float Contour::evLength(std::pair<float, float> ev)
	{
		return pow(ev.first * ev.first + ev.second * ev.second, 0.5);
	}

	float Contour::getPerimeter()
	{
		float retval = 0;
		for (auto i = 0; i < base.size(); i++)
		{
			retval += evLength(base[i]);
		}
		return retval;
	}
}