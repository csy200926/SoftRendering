#pragma once

#include <map>
#include <string>
#include <stack>
#include <set>
#define ENABLE_PROFILING

#if defined( ENABLE_PROFILING )


struct Accumulator
{
	double m_Count;
	double m_Sum;
	double m_Max;
	double m_Min;
	Accumulator *m_pParent;
	std::map<std::string, Accumulator*> m_subAccums;

	Accumulator()
	{
		m_Count = 0.0;
		m_Sum = 0.0;
		m_Max = 0.0;
		m_Min = UINT_MAX;
	}
};

class Profiling
{
	std::map<std::string,Accumulator*> s_Accumulators;
	std::map<std::string,Accumulator*> s_DummyAccums;
public:
	std::stack<std::string> s_Sequences;

	Profiling(void);
	~Profiling(void);

	void InitAccumulator(const char * i_pName);
	void addTiming( const char * i_pName, double i_MS );

	void addTiming_old(const char * i_pName, double i_MS);
	void PrintAccumulators_old( void );

	void PrintAccumulators();
	void PrintAccumulatorsHelper(std::map<std::string, Accumulator*> *i_pAccumulator_List,int i_count);
};
extern Profiling g_Profiler;

class ScopedTimer
{
	const char * m_pScopeName;
	double m_startTime;
public:
	ScopedTimer(const char * i_pName);

	~ScopedTimer();

};



#define PROFILE_SCOPE_BEGIN(str) { ScopedTimer pro( str );
#define PROFILE_SCOPE_END }

#define PROFILE_UNSCOPED(str) ScopedTimer pro( str );
#define PROFILE_PRINT_RESULTS g_Profiler.PrintAccumulators_old();
#else
#define PROFILE_SCOPE_BEGIN(str) __noop;
#define PROFILE_SCOPE_END __noop;
#define PROFILE_UNSCOPED(str) __noop;
#define PROFILE_PRINT_RESULTS __noop;
#endif // ENABLE_PROFILING