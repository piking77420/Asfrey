
#include <iostream>

#include <Fiber.hpp>
#include <Job.hpp>
#include <string>
#include <thread>
#include <Windows.h>
#include <vector>

#include "AsfreyPool.h"

constexpr size_t CountToValue = 100;
Asfrey::AsfreyPool asfreyPool;

void CountTo(void* _param)
{
	using namespace std;
	int i, j;
	int nx = 5500;
	int ny = 300;
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




void MulthiThread()
{
	using namespace Asfrey;

	std::vector<Asfrey::Job> jobs1;
	jobs1.resize(CountToValue);

	for (auto& j : jobs1)
	{
		j.func = CountTo;
		j.arg = nullptr;
		j.jobPriorities = JobPriorities::MEDIUM;
	}

	AtomicCounter* jobCounter;
	asfreyPool.RunJob(jobs1.data(), jobs1.size(), &jobCounter);

	asfreyPool.WaitForCounterAndFree(jobCounter);
}

void MonoThread()
{
	for (size_t i = 0; i < CountToValue; i++)
	{
		CountTo(nullptr);
	}
}


int main()
{
	asfreyPool.Initialize();

	
	auto start = std::chrono::high_resolution_clock::now();
	MulthiThread();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedMulti = end - start;
	std::cout << "Multithreaded execution time: " << elapsedMulti.count() << '\n';

	start = std::chrono::high_resolution_clock::now();
	MonoThread();
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double>elapsedMono = end - start;
	std::cout << "MonoThread execution time: " << elapsedMono.count() << '\n';

	float percent = 0.f;

	if (elapsedMono.count() > elapsedMulti.count())
	{
		std::cout << "Multithreaded is faster\n";
		percent = (elapsedMono.count() - elapsedMulti.count()) / elapsedMono.count() * 100.f;
	}
	else if (elapsedMono.count() < elapsedMulti.count())
	{
		std::cout << "Single-threaded is faster\n";
		percent = (elapsedMulti.count() - elapsedMono.count()) / elapsedMulti.count() * 100.f;
	}
	else
	{

	}

	std::cout << "by " << percent << "%\n";

	std::getchar();

	asfreyPool.Destroy();
	return 0;
}