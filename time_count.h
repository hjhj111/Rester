#pragma once
#include<iostream>
#include<vector>
#include<chrono>
#include<unordered_map>

//using namespace std;

//for average time of operations; remember to subtract default time used by TimeCount
class TimeCount
{
private:
    std::chrono::high_resolution_clock::time_point t0_;
    std::chrono::high_resolution_clock::time_point t1_;
    std::vector<long long> sections_;
    std::vector<long long> nums_;
public:
	TimeCount(int n)
	{
		sections_.resize(n, 0);
		nums_.resize(n, 0);
	}

	inline void CheckIn(int i)
	{
        t0_ = std::chrono::high_resolution_clock::now();
	}
	inline void CheckOut(int i)
	{
        t1_ = std::chrono::high_resolution_clock::now();
        sections_[i] += std::chrono::duration_cast<std::chrono::nanoseconds>(t1_ - t0_).count();
		nums_[i]++;
	}
	inline void Show()
	{
		int i = 0;
		for (auto section : sections_)
		{
			std::cout << i << "   " << section << " nums " << nums_[i] << " mean" << (nums_[i] > 0 ? section / nums_[i] : 0) << std::endl;
			i++;
		}
	}
};

class TimeDot
{
private:
	std::chrono::high_resolution_clock::time_point t00_;
	std::chrono::high_resolution_clock::time_point t0_;
	std::unordered_map<std::string, long long> times_;
public:
    TimeDot()
		: t00_(std::chrono::high_resolution_clock::now()),
          t0_(std::chrono::high_resolution_clock::now())
	{	
		std::cout << "TS start...\n";
	}
	~TimeDot()
	{
		std::cout << "TS end...\n";
		std::cout << "sum of time  " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - t00_).count() << std::endl;
	}
	void check(std::string str)
	{
		long long time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - t0_).count();
		std::cout << str << "   " << time <<std::endl;
		times_[str] = time;
        t0_ = std::chrono::high_resolution_clock::now();
	}
};