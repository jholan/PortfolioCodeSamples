#include "Engine/Profiler/Profiler.hpp"

#include <stack>
#include <deque>
#include <algorithm>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Commands/Command.hpp"

#define PROFILER_ENABLED



// ----------------------------------------------------------------------------------------------------------------
// ProfilerNode ---------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
ProfilerNode::ProfilerNode(const char* name)
	: m_name(name)
	, m_startHPC(GetCurrentTimeInHPC())
	, m_endHPC(m_startHPC)
	, m_parent(nullptr)
{
}



ProfilerNode::~ProfilerNode()
{
	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		delete m_children[i];
	}
	m_children.clear();
}



uint64_t ProfilerNode::GetSelfTimeHPC() const
{
	uint64_t selfElapsedTime = GetElapsedTimeHPC();

	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		uint64_t childSubTreeElapsedTime = m_children[i]->GetElapsedTimeHPC();
		selfElapsedTime -= childSubTreeElapsedTime;
	}

	return selfElapsedTime;
}



// ----------------------------------------------------------------------------------------------------------------
// ReportNode----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
ReportNode::ReportNode(const ProfilerNode* profilerNode)
	: m_name(profilerNode->m_name)
	, m_callCount(1)
	, m_elapsedTimeHPC(profilerNode->GetElapsedTimeHPC())
	, m_selfTimeHPC(profilerNode->GetSelfTimeHPC())
{

}



ReportNode::~ReportNode()
{
	for (auto iter = m_children.begin(); iter != m_children.end(); ++iter)
	{
		delete iter->second;
	}
	m_children.clear();
}



// ----------------------------------------------------------------------------------------------------------------
// Profiler -------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
#ifdef PROFILER_ENABLED


// ----------------------------------------------------------------------------------------------------------------
// State ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
// Saved Trees
constexpr int				MAX_NUM_OLD_TREES = 128;
std::deque<ProfilerNode*>	s_oldTrees;

// Stack for creation of the current tree
ProfilerNode*				s_currentTree = nullptr;
std::stack<ProfilerNode*>	s_nodeStack;
unsigned int				s_frameCounter = 0;

// Pausing
bool						s_isPaused = false;
bool						s_shouldTogglePauseState = false;



// ----------------------------------------------------------------------------------------------------------------
// Profiler Commands ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
void Profiler_Pause_Command(Command& cmd)
{
	UNUSED(cmd);

	Profiler_Pause();
}



void Profiler_Unpause_Command(Command& cmd)
{
	UNUSED(cmd);

	Profiler_Unpause();
}



// ----------------------------------------------------------------------------------------------------------------
// Composition ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
void Profiler_Initialize()
{
	RegisterCommand("ProfilerPause", Profiler_Pause_Command);
	RegisterCommand("ProfilerUnpause", Profiler_Unpause_Command);
}



void Profiler_Destroy()
{
	for (int i = 0; i < (int)s_oldTrees.size(); ++i)
	{
		delete s_oldTrees[i];
	}
	s_oldTrees.clear();
}



// ----------------------------------------------------------------------------------------------------------------
// Updating -------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
void Profiler_BeginFrame()
{
	GUARANTEE_OR_DIE(s_nodeStack.size() == 0, "Uneven number of pushes/pops in the profiler");
	
	
	++s_frameCounter;

	if (s_currentTree != nullptr)
	{
		s_currentTree->m_endHPC = GetCurrentTimeInHPC();

		// If the profiler is not paused add the current tree to the list of old trees
		if (!Profiler_IsPaused())
		{
			// Here we want to save off the current tree to some location 
			// If the number of old trees we are storing is getting to large we want to remove the oldest one
			if (s_oldTrees.size() >= MAX_NUM_OLD_TREES)
			{
				delete s_oldTrees.front();
				s_oldTrees.pop_front();
			}
			s_oldTrees.push_back(s_currentTree);
		}
		else
		{
			// Else destroy it
			// NOTE: So we don't have to manage pause state all over
			//			Even if we are paused we keep generating trees we just throw them away at the end of the frame
			delete s_currentTree;
			s_currentTree = nullptr;
		}

	}


	// Create the new tree
	std::string frameName = "Frame " + std::to_string(s_frameCounter);
	s_currentTree = new ProfilerNode(frameName.c_str());


	// Toggle pause state 
	if (s_shouldTogglePauseState)
	{
		s_isPaused = !s_isPaused;
		s_shouldTogglePauseState = false;
	}
}


void Profiler_EndFrame()
{

}



// ----------------------------------------------------------------------------------------------------------------
// Current Tree Stack Manipulation --------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
void Profiler_Push(char const* name)
{
	ProfilerNode* node = new ProfilerNode(name);

	if (s_nodeStack.size() > 0)
	{
		ProfilerNode* top = s_nodeStack.top();
		top->m_children.push_back(node);
		node->m_parent = top;
	}

	s_nodeStack.push(node);
}



void Profiler_Pop()
{
	if (s_nodeStack.size() > 1)
	{
		// The stack will still have nodes on it
		ProfilerNode* top = s_nodeStack.top();


		top->m_endHPC = GetCurrentTimeInHPC();


		s_nodeStack.pop();
	}
	else if (s_nodeStack.size() == 1)
	{
		// This is the last node
		ProfilerNode* top = s_nodeStack.top();


		// Update node
		top->m_endHPC = GetCurrentTimeInHPC();


		// Remove node from stack
		s_nodeStack.pop();


		// Add subtree to current tree
		s_currentTree->m_children.push_back(top); // This subtree is a direct child of the frame
	}
	else 
	{
		// Bad Shit, we popped an empty stack
		GUARANTEE_OR_DIE(false, "Uneven number of pushes/pops in the profiler");
	}
}



// ----------------------------------------------------------------------------------------------------------------
// Pausing -------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
bool Profiler_IsPaused()
{
	return s_isPaused;
}



void Profiler_Pause()
{
	if (!s_isPaused)	
	{
		s_shouldTogglePauseState = true;
	}
}



void Profiler_Unpause()
{
	if (s_isPaused)
	{
		s_shouldTogglePauseState = true;
	}
}



// ----------------------------------------------------------------------------------------------------------------
// Frame Number ---------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
unsigned int Profiler_GetFrameNumber()
{
	return s_frameCounter;
}



// ----------------------------------------------------------------------------------------------------------------
// Report Generation ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
// Report Formatting Constants(PX) ------------------------------------------------------------------------
const int INDENT_SIZE			= 1;
const int NAME_SIZE				= 72;
const int CALL_COUNT_SIZE		= 8;
const int ELAPSED_PERCENT_SIZE	= 8;
const int ELAPSED_TIME_SIZE		= 14;
const int SELF_PERCENT_SIZE		= 8;
const int SELF_TIME_SIZE		= 14;


void ParseTime(float timeInSeconds, int& intPart, float& floatPart, std::string& units)
{
	int unitUps = 0;

	float time = timeInSeconds;
	while(time < 1.0f)
	{
		time *= 1000.0f;

		++unitUps;
		if (unitUps >= 3)
		{
			break;
		}
	}


	intPart = (int)time;
	floatPart = time - (int)time;

	if (unitUps == 0)
	{
		units = "s";
	}
	else if (unitUps == 1)
	{
		units = "ms";
	}
	else if (unitUps == 2)
	{
		units = "us";
		floatPart = 0.0f;
	}
	else 
	{
		units = "ns";
		floatPart = 0.0f;
	}
}



// Sort Functions -------------------------------------------------------------------------------------
bool SortFunction_TotalTime(const ReportNode* lhs, const ReportNode* rhs)
{
	bool isLHSLess = false;

	if (lhs->m_elapsedTimeHPC > rhs->m_elapsedTimeHPC)
	{
		isLHSLess = true;
	}

	return isLHSLess;
}



bool SortFunction_SelfTime(const ReportNode* lhs, const ReportNode* rhs)
{
	bool isLHSLess = false;

	if (lhs->m_selfTimeHPC > rhs->m_selfTimeHPC)
	{
		isLHSLess = true;
	}

	return isLHSLess;
}



// Report Tree Generation------------------------------------------------------------------------------
ReportNode* Report_GenerateNode_Recursively(ReportNode* parentNode, ProfilerNode* nodeToExtractDataFrom)
{
	ReportNode* node = new ReportNode(nodeToExtractDataFrom);

	if (parentNode != nullptr)
	{
		node->m_parent = parentNode;
		
		auto foundChild = parentNode->m_children.find(node->m_name);
		if (foundChild == parentNode->m_children.end())
		{
			// No node with this name in the map
			parentNode->m_children[node->m_name] = node;
		}
		else
		{
			// There is a node with this name in the map
			ReportNode* childNode = parentNode->m_children[node->m_name];
			childNode->m_callCount += 1;
			childNode->m_elapsedTimeHPC += node->m_elapsedTimeHPC;
			childNode->m_selfTimeHPC += node->m_selfTimeHPC;

			delete node;
			node = childNode;
		}

	}

	// Process Children
	for (int i = 0; i < (int)nodeToExtractDataFrom->m_children.size(); ++i)
	{
		ProfilerNode* profilerChildNode = nodeToExtractDataFrom->m_children[i];
		Report_GenerateNode_Recursively(node, profilerChildNode);
	}

	return node;
}



ReportNode*	Profiler_GenerateReportTree(ProfilerNode* node)
{
	GUARANTEE_OR_DIE(node != nullptr, "Cannot generate a report tree for a nullptr");

	ReportNode* report = Report_GenerateNode_Recursively(nullptr, node);
	return report;
}



// Flat Report------------------------------------------------------------------------------------------
void FlatReport_GenerateNode_Recursive(std::map<std::string, ReportNode*>& report, ReportNode* node)
{
	// Get the node from the report
	ReportNode* reportNode = report[node->m_name];

	// If one doesn't exist yet create it
	if (reportNode == nullptr)
	{
		report[node->m_name] = new ReportNode();
		reportNode = report[node->m_name];
		reportNode->m_name = node->m_name;
	}

	reportNode->m_callCount += node->m_callCount;
	reportNode->m_elapsedTimeHPC += node->m_elapsedTimeHPC;
	reportNode->m_selfTimeHPC += node->m_selfTimeHPC;

	for (auto iter = node->m_children.begin(); iter != node->m_children.end(); ++iter)
	{
		FlatReport_GenerateNode_Recursive(report, iter->second);
	}
}



std::vector<PrintableReportLine> Profiler_GenerateFlatReportFromTree(ReportNode* tree, eFlatReportSortMode sortMode)
{
	GUARANTEE_OR_DIE(tree != nullptr, "Cannot generate a profiler report for a null tree");


	// Generate the report
	std::map<std::string, ReportNode*> report;
	FlatReport_GenerateNode_Recursive(report, tree);


	// Move the map into a vector so we can sort it
	std::vector<ReportNode*> sortableReport;
	sortableReport.reserve(report.size());
	for (auto iter = report.begin(); iter != report.end(); ++iter)
	{
		sortableReport.push_back(iter->second);
	}


	// Sort the map
	switch(sortMode)
	{
	case FLAT_REPORT_SORT_MODE_SELF_TIME:
	{
		std::sort(sortableReport.begin(), sortableReport.end(), SortFunction_SelfTime);
		break;
	}
	case FLAT_REPORT_SORT_MODE_TOTAL_TIME:
	{
		std::sort(sortableReport.begin(), sortableReport.end(), SortFunction_TotalTime);
		break;
	}
	case FLAT_REPORT_SORT_MODE_INVALID:
	case FLAT_REPORT_SORT_MODE_COUNT:
	default:
	{
		GUARANTEE_OR_DIE(false, "Please use flat report sort mode self or total time");
		break;
	}
	}


	// Generate the strings
	std::vector<PrintableReportLine> printableReport;
	//printableReport.push_back(Stringf("%-*s %-*s %-*s %-*s %-*s %-*s", NAME_SIZE, "FUNCTION NAME", CALL_COUNT_SIZE, "CALLS", ELAPSED_PERCENT_SIZE, "TOTAL%", ELAPSED_TIME_SIZE, "TOTAL TIME", SELF_PERCENT_SIZE, "Self%", SELF_TIME_SIZE, "SELF TIME"));
	for (int i = 0; i < (int)sortableReport.size(); ++i)
	{
		ReportNode* node = sortableReport[i];

		PrintableReportLine prl;

		// Name
		size_t delimiter = node->m_name.find_last_of(':');
		if (delimiter != std::string::npos)
		{
			prl.m_nameFront = node->m_name.substr(0, delimiter + 1);
			prl.m_nameBack = node->m_name.substr(delimiter+1);
		}
		else
		{
			prl.m_nameFront = node->m_name;
		}

		// Call Count
		prl.m_callCount = node->m_callCount;

		// Total %
		float elapsedPercent = ((float)node->m_elapsedTimeHPC / (float)tree->m_elapsedTimeHPC) * 100.0f;
		prl.m_totalTimePercent_intPart = (int)elapsedPercent;
		prl.m_totalTimePercent_floatPart = elapsedPercent - (float)prl.m_totalTimePercent_intPart;

		// Total Time
		ParseTime((float)ConvertHPCtoSeconds(node->m_elapsedTimeHPC), prl.m_totalTime_intPart, prl.m_totalTime_floatPart, prl.m_totalTime_units);

		// Self %
		float selfPercent;
		if (node->m_elapsedTimeHPC == 0)
		{
			selfPercent = 100.0f;
		}
		else
		{
			selfPercent = ((float)node->m_selfTimeHPC / (float)node->m_elapsedTimeHPC) * 100.0f;
		}
		prl.m_selfTimePercent_intPart = (int)selfPercent;
		prl.m_selfTimePercent_floatPart = selfPercent - (float)prl.m_selfTimePercent_intPart;

		// Self Time
		ParseTime((float)ConvertHPCtoSeconds(node->m_selfTimeHPC), prl.m_selfTime_intPart, prl.m_selfTime_floatPart, prl.m_selfTime_units);
		
		printableReport.push_back(prl);
	}


	// Free the memory we allocated to create the report
	for (auto iter = report.begin(); iter != report.end(); ++iter)
	{
		delete iter->second;
	}
	report.clear();
	sortableReport.clear();


	return printableReport;
}


// Nested Report-----------------------------------------------------------------------------------
void NestedReport_AddNodeToReport_Recursive(std::vector<PrintableReportLine>& report, ReportNode* node, ReportNode* treeRoot, int level = 0)
{
	PrintableReportLine prl;

	prl.m_indent = level;

	// Name
	size_t delimiter = node->m_name.find_last_of(':');
	if (delimiter != std::string::npos)
	{
		prl.m_nameFront = node->m_name.substr(0, delimiter + 1);
		prl.m_nameBack = node->m_name.substr(delimiter+1);
	}
	else
	{
		prl.m_nameFront = node->m_name;
	}

	// Call Count
	prl.m_callCount = node->m_callCount;

	// Total %
	float elapsedPercent = ((float)node->m_elapsedTimeHPC / (float)treeRoot->m_elapsedTimeHPC) * 100.0f;
	prl.m_totalTimePercent_intPart = (int)elapsedPercent;
	prl.m_totalTimePercent_floatPart = elapsedPercent - (float)prl.m_totalTimePercent_intPart;

	// Total Time
	ParseTime((float)ConvertHPCtoSeconds(node->m_elapsedTimeHPC), prl.m_totalTime_intPart, prl.m_totalTime_floatPart, prl.m_totalTime_units);

	// Self %
	float selfPercent;
	if (node->m_elapsedTimeHPC == 0)
	{
		selfPercent = 100.0f;
	}
	else
	{
		selfPercent = ((float)node->m_selfTimeHPC / (float)node->m_elapsedTimeHPC) * 100.0f;
	}
	prl.m_selfTimePercent_intPart = (int)selfPercent;
	prl.m_selfTimePercent_floatPart = selfPercent - (float)prl.m_selfTimePercent_intPart;

	// Self Time
	ParseTime((float)ConvertHPCtoSeconds(node->m_selfTimeHPC), prl.m_selfTime_intPart, prl.m_selfTime_floatPart, prl.m_selfTime_units);

	report.push_back(prl);

	for (auto iter = node->m_children.begin(); iter != node->m_children.end(); ++iter)
	{
		NestedReport_AddNodeToReport_Recursive(report, iter->second, treeRoot, level + 1);
	}
}


std::vector<PrintableReportLine> Profiler_GenerateNestedReportFromTree(ReportNode* tree)
{
	std::vector<PrintableReportLine> report;

	// Generate Report
	NestedReport_AddNodeToReport_Recursive(report, tree, tree);
	
	return report;
}



// Access old reports -----------------------------------------------------------------------------
int Profiler_GetMaxNumPreviousTrees()
{
	return MAX_NUM_OLD_TREES;
}



int Profiler_GetNumPreviousTrees()
{
	return (int)s_oldTrees.size();
}



ProfilerNode* Profiler_GetPreviousTree(unsigned int xTreesAgo)
{
	ProfilerNode* node = nullptr;

	if (s_oldTrees.size() > xTreesAgo)
	{
		node = s_oldTrees[s_oldTrees.size() - 1 - xTreesAgo];
	}

	return node;
}



std::vector<ProfilerNode*> Profiler_GetAllPreviousTrees()
{
	std::vector<ProfilerNode*> oldTrees;

	for (int i = 0; i < (int)s_oldTrees.size(); ++i)
	{
		oldTrees.push_back(s_oldTrees[i]);
	}

	return oldTrees;
}



bool Profiler_IsCompiledIn()
{
	return true;
}



#else
// ----------------------------------------------------------------------------------------------------------------
// Stubs ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
void Profiler_Initialize() {}
void Profiler_Destroy() {}

void Profiler_BeginFrame() {}
void Profiler_EndFrame() {}

void Profiler_Push(char const* name) {UNUSED(name);}
void Profiler_Pop() {}

bool Profiler_IsPaused() {return false;}
void Profiler_Pause() {}
void Profiler_Unpause() {}

unsigned int Profiler_GetFrameNumber() {return 0;}

ReportNode*						 Profiler_GenerateReportTree(ProfilerNode* tree) {UNUSED(tree); return nullptr;}
std::vector<PrintableReportLine> Profiler_GenerateFlatReportFromTree(ReportNode* tree, eFlatReportSortMode sortMode) {UNUSED(tree); UNUSED(sortMode); return std::vector<PrintableReportLine>();}
std::vector<PrintableReportLine> Profiler_GenerateNestedReportFromTree(ReportNode* tree) {UNUSED(tree); return std::vector<PrintableReportLine>();}

int							Profiler_GetMaxNumPreviousTrees() {return 0;}
int							Profiler_GetNumPreviousTrees() {return 0;}
ProfilerNode*				Profiler_GetPreviousTree(unsigned int xFramesAgo) {UNUSED(xFramesAgo); return nullptr;}
std::vector<ProfilerNode*>	Profiler_GetAllPreviousTrees() {return std::vector<ProfilerNode*>();}

bool						Profiler_IsCompiledIn() {return false;};
#endif
