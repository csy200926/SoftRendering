#include "Profiling.h"
#include <iostream>
#include <fstream>
#include <set>
#include "WinTiming.h"
#if defined( ENABLE_PROFILING )
Profiling	g_Profiler;

Profiling::Profiling(void)
{
}


Profiling::~Profiling(void)
{
	//TODO:Delete everything
}

void Profiling::InitAccumulator(const char * i_pName)
{
	using namespace std;
	map<string, Accumulator*>::iterator it = s_Accumulators.find(i_pName);
	Accumulator *pAccumulator = nullptr;
	if (it == s_Accumulators.end())
	{
		pAccumulator = new Accumulator();
		if (g_Profiler.s_Sequences.size() == 0)
			s_DummyAccums.insert(make_pair(i_pName, pAccumulator));

		s_Accumulators.insert(make_pair(i_pName, pAccumulator));
	}
	else
		pAccumulator = it->second;



	if (g_Profiler.s_Sequences.size() > 0)
	{
		string parentName = g_Profiler.s_Sequences.top();
		
		// Get parent child_list
		map<string, Accumulator*>&subAccums = s_Accumulators.find(parentName)->second->m_subAccums;
		
		if (subAccums.find(i_pName) == subAccums.end())
			subAccums.insert(make_pair(i_pName, pAccumulator));

	}
}
#define MAX_COUNT 100
void Profiling::addTiming_old(const char * i_pName, double i_MS)
{
	using namespace std;

	map<string, Accumulator*>::iterator it = s_Accumulators.find(i_pName);
	Accumulator *pAccumulator = nullptr;
	if (it == s_Accumulators.end())
	{
		pAccumulator = new Accumulator();
		s_Accumulators.insert(make_pair(i_pName, pAccumulator));
	}
	else
		pAccumulator = it->second;

	if (pAccumulator->m_Count >= MAX_COUNT)
		return;

	if (i_MS < pAccumulator->m_Min)
		pAccumulator->m_Min = i_MS;
	if (i_MS > pAccumulator->m_Max)
		pAccumulator->m_Max = i_MS;

	pAccumulator->m_Count++;
	pAccumulator->m_Sum += i_MS;
}


void Profiling::addTiming( const char * i_pName, double i_MS )
{
	using namespace std;

	Accumulator *pAccumulator = nullptr;

	map<string, Accumulator*>::iterator it = s_Accumulators.find(i_pName);
	if (it != s_Accumulators.end())
		pAccumulator = it->second;
	else
		return;

	if (pAccumulator->m_Count >= MAX_COUNT)
		return;

	if (i_MS < pAccumulator->m_Min)
		pAccumulator->m_Min = i_MS;
	if (i_MS > pAccumulator->m_Max)
		pAccumulator->m_Max = i_MS;

	pAccumulator->m_Count++;
	pAccumulator->m_Sum += i_MS;


}



void Profiling::PrintAccumulators_old(void)
{
	using namespace std;

	std::map<std::string, Accumulator*>::iterator iter = s_Accumulators.begin();
	
	fstream myfile;
	myfile.open("D:\\Profiling.txt");
	
	for( iter ; iter != s_Accumulators.end(); ++iter )
	{
		double Average = iter->second->m_Count ? ((double)iter->second->m_Sum) / iter->second->m_Count : 0.0f;
		//printf( "[%s] \nCount: %d \nSum: %f[MS] \nMin: %f[MS] \nMax: %f[MS] \nAve: %f[MS]\n",
		//	 iter->first.c_str(),
		//	 (int)iter->second->m_Count,
		//	 iter->second->m_Sum,
		//	 iter->second->m_Min,
		//	 iter->second->m_Max,
		//	 Average);
		printf("[ %-12s: %f(MS) ]\n",
			iter->first.c_str(), Average);
		
		myfile << iter->first.c_str() << " Count: " << iter->second->m_Count << " Sum: " << iter->second->m_Sum << " Min: " << iter->second->m_Min << " Max: " << iter->second->m_Max << " Ave: " << Average << "\n";
	}

	myfile.close();
} 

void Profiling::PrintAccumulators()
{
	PrintAccumulatorsHelper(&s_DummyAccums, 0);
}
void Profiling::PrintAccumulatorsHelper(std::map<std::string, Accumulator*> *i_pAccumulator_List,int i_count)
{
	using namespace std;
	std::map<std::string, Accumulator*>::iterator iter = i_pAccumulator_List->begin();
	std::map<std::string, Accumulator*>::iterator iter_end = i_pAccumulator_List->end();

	string offset;

	for (int i = 0; i < i_count; i++)
	{
		offset += "     ";
	}
		

	for (iter; iter != iter_end; ++iter)
	{
		double Average = iter->second->m_Count ? ((double)iter->second->m_Sum) / iter->second->m_Count : 0.0f;

		printf("%s[ %-12s: %f(MS) ]\n",
			offset.c_str(), iter->first.c_str(), Average);

		if (iter->second->m_subAccums.empty() == false)
		{
			PrintAccumulatorsHelper(&iter->second->m_subAccums, i_count + 1);
		}
		if (i_count == 0)
			printf("\n");
	}


}


ScopedTimer::ScopedTimer(const char * i_pName) : m_pScopeName(i_pName)
{
	//g_Profiler.InitAccumulator(i_pName);
	//g_Profiler.s_Sequences.push(i_pName);
	m_startTime = Timing::WinTiming::getCurenntFrameTime_ms();
	
}

ScopedTimer::~ScopedTimer()
{
	//g_Profiler.s_Sequences.pop();

	double diff = Timing::WinTiming::getCurenntFrameTime_ms() - m_startTime;//second
	//g_Profiler.addTiming(m_pScopeName,diff * 1000);// add by ms
	g_Profiler.addTiming_old(m_pScopeName, diff * 1000);// add by ms
}
#endif // ENABLE_PROFILING