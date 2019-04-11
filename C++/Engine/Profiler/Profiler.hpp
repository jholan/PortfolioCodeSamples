#pragma once

#include <string>
#include <vector>
#include <map>



class ProfilerNode
{
public:
	ProfilerNode(const char* name);
	~ProfilerNode();

	std::string					m_name;
	
	uint64_t					m_startHPC;
	uint64_t					m_endHPC;

	ProfilerNode*				m_parent;
	std::vector<ProfilerNode*>	m_children;

	uint64_t GetSelfTimeHPC() const;
	uint64_t GetElapsedTimeHPC() const {return m_endHPC - m_startHPC;};



private:
};



class ReportNode
{
public:
	ReportNode() {};
	ReportNode(const ProfilerNode* profilerNode);
	~ReportNode();

	std::string m_name;
	int			m_callCount = 0;

	uint64_t	m_elapsedTimeHPC;
	uint64_t	m_selfTimeHPC;



	ReportNode*							m_parent = nullptr;
	std::map<std::string, ReportNode*>	m_children;
private:
};



class PrintableReportLine
{
public:
	int			m_indent = 0;

	std::string m_nameFront;
	std::string m_nameBack;

	int			m_callCount;

	int			m_totalTimePercent_intPart;
	float		m_totalTimePercent_floatPart;

	int			m_totalTime_intPart;
	float		m_totalTime_floatPart;
	std::string m_totalTime_units;

	int			m_selfTimePercent_intPart;
	float		m_selfTimePercent_floatPart;

	int			m_selfTime_intPart;
	float		m_selfTime_floatPart;
	std::string m_selfTime_units;

private:
};



enum eFlatReportSortMode
{
	FLAT_REPORT_SORT_MODE_INVALID = -1,

	FLAT_REPORT_SORT_MODE_SELF_TIME = 0,
	FLAT_REPORT_SORT_MODE_TOTAL_TIME,

	FLAT_REPORT_SORT_MODE_COUNT
};



// Composition
void Profiler_Initialize();
void Profiler_Destroy();

// Updating
void Profiler_BeginFrame();
void Profiler_EndFrame();

// Current Frame Stack Manipulation
void Profiler_Push(char const* name); 
void Profiler_Pop(); 

// Pausing
bool Profiler_IsPaused();
void Profiler_Pause();
void Profiler_Unpause();

// Frame Number
unsigned int Profiler_GetFrameNumber();

// Reports
ReportNode*						 Profiler_GenerateReportTree(ProfilerNode* tree);
std::vector<PrintableReportLine> Profiler_GenerateFlatReportFromTree(ReportNode* tree, eFlatReportSortMode sortMode);
std::vector<PrintableReportLine> Profiler_GenerateNestedReportFromTree(ReportNode* tree);

// Access Old Reports
int							Profiler_GetMaxNumPreviousTrees();
int							Profiler_GetNumPreviousTrees();
ProfilerNode*				Profiler_GetPreviousTree(unsigned int xTreesAgo = 0); 
std::vector<ProfilerNode*>	Profiler_GetAllPreviousTrees();

// Compiled In
bool						Profiler_IsCompiledIn();