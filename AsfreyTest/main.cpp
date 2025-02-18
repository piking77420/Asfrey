
#include <iostream>

#include <Fiber.hpp>
#include <Job.hpp>
#include <string>
#include <thread>
#include <Windows.h>
#include <vector>

#include "AsfreyPool.h"

bool MonoVsMultiTest = true;


constexpr size_t CountToValue = 1000;

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
	Asfrey::AsfreyPool::RunJob(jobs1.data(), jobs1.size(), &jobCounter);

	Asfrey::AsfreyPool::WaitForCounterAndFree(jobCounter);
}

void MonoThread()
{
	for (size_t i = 0; i < CountToValue; i++)
	{
		CountTo(nullptr);
	}
}

void MonoMulti()
{
	Asfrey::AsfreyPool::Initialize();

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

	Asfrey::AsfreyPool::Destroy();
}

void SetCondition()
{

	if (std::getchar())
	{
		Asfrey::AsfreyPool::SetCondition(Asfrey::JobCondition::C1, true);
	}
	
	while (true)
	{

	}
	
}

void WaitCondition(void*)
{
	std::cout << "wait" << std::endl;

	Asfrey::AsfreyPool::YieldOnCondition(Asfrey::JobCondition::C1);
	std::cout << "Run" << std::endl;

}


void TestConditionTest()
{

	Asfrey::AsfreyPool::Initialize();

	std::jthread d = std::jthread(SetCondition);
	

	using namespace Asfrey;

	std::vector<Asfrey::Job> jobs1;
	jobs1.resize(CountToValue);

	for (auto& j : jobs1)
	{
		j.func = WaitCondition;
		j.arg = nullptr;
		j.jobPriorities = JobPriorities::MEDIUM;
	}

	AtomicCounter* jobCounter;
	Asfrey::AsfreyPool::RunJob(jobs1.data(), jobs1.size(), &jobCounter);

	Asfrey::AsfreyPool::WaitForCounterAndFree(jobCounter);
	Asfrey::AsfreyPool::Destroy();
}


int main()
{
	if (MonoVsMultiTest)
	{
		MonoMulti();
	}
	else
	{
		TestConditionTest();
	}

	
	return 0;
}