#include "gtest/gtest.h"
#include "../Asfrey/AsfreyPool.h"

#include "../Asfrey/Job.hpp"
#include <vector>

constexpr size_t CountToValue = 1;

void CountTo(void* _param)
{
	using namespace std;
	int i, j;
	int nx = 5000;
	int ny = 100;
	vector<double> vv(nx, 0);
	for (j = 0; j < nx; j++)vv[j] = j;
	double xx = 0;
	for (i = 0; i < ny; i++) {
		for (j = 0; j < nx; j++) {
			vv[j] += 1.;
			xx += vv[j];
		}
	}
	for (j = 0; j < nx; j++) {
		xx += vv[j];
	}
}



TEST(Monothread, Monothread)
{
	for (size_t i = 0; i < CountToValue; i++)
	{
		CountTo(nullptr);
	}
}


TEST(Init, Init)
{
}
