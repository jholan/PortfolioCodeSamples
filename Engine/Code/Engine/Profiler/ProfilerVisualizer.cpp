#include "Engine/Profiler/ProfilerVisualizer.hpp"

#include "Engine/Profiler/Profiler.hpp"

#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Commands/Command.hpp"

#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"
#include "Engine/Rendering/Renderer.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Commands/DevConsole.hpp"

extern InputSystem*			g_theInput;
extern Window*				g_theWindow;
extern Renderer*			g_theRenderer;
extern ProfilerVisualizer*	g_theProfiler;
extern DevConsole*			g_theDevConsole;



void ProfilerVisualizer_ToggleVisibility_Command(Command& cmd)
{
	UNUSED(cmd);

	if (!Profiler_IsCompiledIn())
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Profiler is not compiled in");
	}
	else
	{
		g_theProfiler->ToggleVisibilityState();
		g_theDevConsole->PrintToLog(RGBA(0,255,0), "Profiler Toggled");
	}

}


void ProfilerVisualizer::Initialize()
{
	IntVector2 windowDim = g_theWindow->GetDimensions();

	// Camera
	m_camera = new Camera();
	m_camera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)windowDim));
	m_camera->SetColorTarget(g_theRenderer->GetDefaultColorTarget());
	m_camera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());

	// Font
	m_font = BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png");

	// Rendering
	m_backgroundTexture	= Texture::CreateOrGet("Data/BuiltIns/DEFAULT_TEXTURE.png", Texture::TEXTURE_FORMAT_RGBA8);

	// Graph
	float graphMinY = 694.0f;
	float graphHeight = 100.0f;
	float padding = 8.0f;
	m_graphRegion = AABB2D(padding, graphMinY, (float)windowDim.x - padding - padding, graphMinY + graphHeight);

	// Commands
	RegisterCommand("Profiler", ProfilerVisualizer_ToggleVisibility_Command);
}



void ProfilerVisualizer::Destroy()
{
	// Camera
	delete m_camera;
	m_camera = nullptr;

}



void ProfilerVisualizer::BeginFrame()
{
	if (IsVisible())
	{
		ProcessInput();
	}
}



void ProfilerVisualizer::EndFrame()
{

}



void ProfilerVisualizer::ProcessInput()
{
	// Tree view changes
	if (g_theInput->WasKeyJustPressed('V'))
	{
		ToggleTreeView();
	}
	if (g_theInput->WasKeyJustPressed('L'))
	{
		ToggleFlatView();
	}
	if (g_theInput->WasKeyJustPressed('M'))
	{
		ToggleMouseCaptureState();
	}


	// Handle Mouse things
	if (IsCapturingMouse())
	{
		Vector2 mousePosition = Vector2(g_theInput->GetMousePosition());

		// Tree Selection
		if (m_graphRegion.IsPointInside(mousePosition))
		{
			// If the mouse is inside the graph
			if (g_theInput->WasMouseButtonJustPressed(InputSystem::MOUSE_LEFT))
			{
				m_showingSpecificGraph = true;
				Profiler_Pause();
				float normalizedGraphPositon = RangeMapFloat(mousePosition.x, m_graphRegion.mins.x, m_graphRegion.maxs.x, 0.0f, 1.0f);
				int numOldTrees = Profiler_GetNumPreviousTrees();
				m_specificGraph = (int)(numOldTrees * normalizedGraphPositon);
			}
		}

		// Tree Release
		if (g_theInput->WasMouseButtonJustPressed(InputSystem::MOUSE_RIGHT) && m_showingSpecificGraph == true)
		{
			Profiler_Unpause();
			m_showingSpecificGraph = false;
		}
	}
}



void ProfilerVisualizer::Render()
{
	// Short Circuit
	if (!IsVisible())
	{
		return;
	}


	// Cached values
	IntVector2 windowDim = g_theWindow->GetDimensions();


	// Setup camera
	g_theRenderer->BindCamera(m_camera);
	g_theRenderer->ClearDepth();


	// Bind the shader directly as we will be using this shader for everything
	Shader* ds = Shader::Get("Default_2D");
	g_theRenderer->BindShader(ds);


	// Draw translucent background covering the whole screen
	g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_backgroundTexture);
	Mesh* bg = CreateAABB2D(AABB2D(Vector2(0.0f, 0.0f), (Vector2)windowDim), m_profilerBackgroundColor);
	g_theRenderer->DrawMesh(bg->GetSubMesh(0));
	delete bg;


	// Start Drawing Text
	float padding = 8.0f;
	Vector2 offset = Vector2(8.0f, (float)windowDim.y - padding);

	// Draw Title
	offset.y -= m_titleFontHeight;
	g_theRenderer->DrawText2D(offset, "PROFILER", m_titleFontHeight, RGBA(), m_fontAspect, m_font);

	offset.y -= padding * 2.0f;

	// Draw FPS 
	offset.y -= m_fontHeight;
	float frameTimeInSeconds = (float)ConvertHPCtoSeconds(Profiler_GetPreviousTree()->GetElapsedTimeHPC());
	g_theRenderer->DrawText2D(offset, Stringf("FPS: %i", (int)(1.0f / frameTimeInSeconds)), m_fontHeight, RGBA(), m_fontAspect, m_font);

	// Draw Frame time
	offset.y -= m_fontHeight;
	g_theRenderer->DrawText2D(offset, Stringf("Frame Time: %f ms", frameTimeInSeconds * 1000.0), m_fontHeight, RGBA(), m_fontAspect, m_font);

	// Graph Title
	offset.y -= padding * 2.0f;
	offset.y -= m_titleFontHeight;
	g_theRenderer->DrawText2D(offset, "Frame Time Graph - <M> to toggle mouse", m_titleFontHeight, RGBA(), m_fontAspect, m_font);

	// SPACE FOR THE GRAPH
	offset.y -= 100.0f;
	Mesh* meshBG = CreateAABB2D(m_graphRegion, m_graphBackgroundColor);
	g_theRenderer->BindTexture(Renderer::DIFFUSE_TEXTURE_BIND_POINT, m_backgroundTexture);
	g_theRenderer->DrawMesh(meshBG->GetSubMesh(0));
	delete meshBG;

	std::vector<ProfilerNode*> oldTrees = Profiler_GetAllPreviousTrees();
	float longestTimeInSeconds = 0.0f;
	for (int i = 0; i < (int)oldTrees.size(); ++i)
	{
		float treeTimeInSeconds = (float)ConvertHPCtoSeconds(oldTrees[i]->GetElapsedTimeHPC());
		if (treeTimeInSeconds > longestTimeInSeconds)
		{
			longestTimeInSeconds = treeTimeInSeconds;
		}
	}

	// Selected Report
	Mesh* specificSpikeMesh = nullptr;

	// Graph
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, false);

	float spikeWidth = (float)m_graphRegion.GetDimensions().x / (float)oldTrees.size();
	float spikeMaxHeight = (float)m_graphRegion.GetDimensions().y;

	// Add spikes to mesh
	Vector2 startPos = m_graphRegion.mins;
	for (int i = 0; i < (int)oldTrees.size(); ++i)
	{
		float treeElapsedTimeSeconds = (float)ConvertHPCtoSeconds(oldTrees[i]->GetElapsedTimeHPC());
		float normalizedHeight = treeElapsedTimeSeconds / longestTimeInSeconds;
		Vector2 maxs = Vector2(startPos.x + spikeWidth, startPos.y + (spikeMaxHeight * normalizedHeight));


		// Color code the spike
		RGBA color;
		if (treeElapsedTimeSeconds <= 1.0f / 60.0f)
		{
			color = m_colorGood;
		}
		else if (treeElapsedTimeSeconds <= 1.0f / 30.0f)
		{
			color = m_colorOk;
		}
		else
		{
			color = m_colorBad;
		}


		AABB2D spike = AABB2D(startPos, maxs);
		builder.AddBillboardedQuad(Vector3(spike.GetCenter(), -0.001f), Vector3::RIGHT, Vector3::UP, spike.GetDimensions() * 0.5f, Vector2::ZEROS, Vector2::ONES, color);

		if (m_showingSpecificGraph && i == m_specificGraph)
		{
			specificSpikeMesh = CreateAABB2D(spike, m_selectedReportColor);
		}
		
		startPos.x += spikeWidth;
	}

	builder.Finalize();
	Mesh* graphMesh = builder.CreateMesh();
	g_theRenderer->DrawMesh(graphMesh->GetSubMesh(0));
	delete graphMesh;


	if (specificSpikeMesh != nullptr)
	{
		g_theRenderer->DrawMesh(specificSpikeMesh->GetSubMesh(0));
		delete specificSpikeMesh;
		specificSpikeMesh = nullptr;
	}


	// Report Title
	bool isFlat = (m_viewMode == PROFILER_VISUALIZER_TREE_VIEW_MODE_FLAT ? true : false);
	bool sortByTotalTime = (m_flatViewMode == PROFILER_VISUALIZER_FLAT_VIEW_MODE_TOTAL_TIME ? true : false);
	
	offset.y -= m_titleFontHeight + (padding * 4.0f);
	std::string title = (isFlat ? std::string("FLAT REPORT") : std::string("NESTED REPORT"));
	std::string flatAppend = (isFlat ? ( (sortByTotalTime ? "<L>(TOTAL^)": "<L>(SELF^)") ) : "");
	std::string reportTitle = Stringf("<V> %s %s", title.c_str(), flatAppend.c_str() );
	g_theRenderer->DrawText2D(offset, reportTitle, m_titleFontHeight, RGBA(), m_fontAspect, m_font);
	
	offset.y -= padding;
	offset.x += padding;

	// Get the last report
	int previousTree = (m_showingSpecificGraph ? Profiler_GetMaxNumPreviousTrees() - m_specificGraph - 1 : 0);
	ReportNode* report = Profiler_GenerateReportTree(Profiler_GetPreviousTree(previousTree));
	std::vector<PrintableReportLine> printableReport;
	if (m_viewMode == PROFILER_VISUALIZER_TREE_VIEW_MODE_NESTED)
	{
		printableReport = Profiler_GenerateNestedReportFromTree(report);
	}
	else if (m_viewMode == PROFILER_VISUALIZER_TREE_VIEW_MODE_FLAT && m_flatViewMode == PROFILER_VISUALIZER_FLAT_VIEW_MODE_TOTAL_TIME)
	{
		printableReport = Profiler_GenerateFlatReportFromTree(report, FLAT_REPORT_SORT_MODE_TOTAL_TIME);
	}
	else if (m_viewMode == PROFILER_VISUALIZER_TREE_VIEW_MODE_FLAT && m_flatViewMode == PROFILER_VISUALIZER_FLAT_VIEW_MODE_SELF_TIME)
	{
		printableReport = Profiler_GenerateFlatReportFromTree(report, FLAT_REPORT_SORT_MODE_SELF_TIME);
	}


	// Report Helpers
	float fontWidth = (m_fontHeight * m_fontAspect);
	float nameX = offset.x;
	float callsX = offset.x + (fontWidth * NAME_SIZE); 
	float totalPercentX = callsX + (fontWidth * CALL_COUNT_SIZE);
	float totalTimeX = totalPercentX + (fontWidth * ELAPSED_PERCENT_SIZE);
	float selfPercentX = totalTimeX + (fontWidth * ELAPSED_TIME_SIZE);
	float selfTimeX = selfPercentX + (fontWidth * SELF_PERCENT_SIZE);


	offset.y -= padding;
	
	// Draw Report Headers
	g_theRenderer->DrawText2D(Vector2(nameX, offset.y), "FUNCTION NAME", m_fontHeight, m_nameFrontColor, m_fontAspect, m_font); 
	g_theRenderer->DrawText2D(Vector2(callsX, offset.y), "CALLS", m_fontHeight, m_nameFrontColor, m_fontAspect, m_font); 
	g_theRenderer->DrawText2D(Vector2(totalPercentX, offset.y), "TOTAL%", m_fontHeight, m_nameFrontColor, m_fontAspect, m_font); 
	g_theRenderer->DrawText2D(Vector2(totalTimeX, offset.y), "TOTAL TIME", m_fontHeight, m_nameFrontColor, m_fontAspect, m_font); 
	g_theRenderer->DrawText2D(Vector2(selfPercentX, offset.y), "SELF%", m_fontHeight, m_nameFrontColor, m_fontAspect, m_font); 
	g_theRenderer->DrawText2D(Vector2(selfTimeX, offset.y), "SELF TIME", m_fontHeight, m_nameFrontColor, m_fontAspect, m_font); 
	
	// Draw Report
	
	for (int i = 0; i < (int)printableReport.size(); ++i)
	{
		offset.y -= m_fontHeight + 1;


		PrintableReportLine& line = printableReport[i];

		// Name
		// Indent
		int spacesToIndent = line.m_indent * INDENT_SIZE;
		g_theRenderer->DrawText2D(Vector2(nameX + (fontWidth * spacesToIndent), offset.y), line.m_nameFront, m_fontHeight, m_nameFrontColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(nameX + ((spacesToIndent + line.m_nameFront.length()) * fontWidth), offset.y), line.m_nameBack, m_fontHeight, m_nameBackColor, m_fontAspect, m_font);

		// Calls
		g_theRenderer->DrawText2D(Vector2(callsX, offset.y), std::to_string(line.m_callCount), m_fontHeight, m_intColor, m_fontAspect, m_font); 

		// Total Percent
		std::string totalPercent_intPart = std::to_string(line.m_totalTimePercent_intPart);
		std::string totalPercent_floatPart = Stringf("%.2f", line.m_totalTimePercent_floatPart);
		totalPercent_floatPart = totalPercent_floatPart.substr(totalPercent_floatPart.find_first_of('.'));
		g_theRenderer->DrawText2D(Vector2(totalPercentX, offset.y), totalPercent_intPart, m_fontHeight, m_intColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(totalPercentX + (totalPercent_intPart.size() * fontWidth), offset.y), totalPercent_floatPart, m_fontHeight, m_floatColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(totalPercentX + ((totalPercent_intPart.size() + totalPercent_floatPart.size()) * fontWidth), offset.y), "%", m_fontHeight, m_intColor, m_fontAspect, m_font);

		// Total Time
		std::string totalTime_intPart = std::to_string(line.m_totalTime_intPart);
		std::string totalTime_floatPart = "";
		if (line.m_totalTime_floatPart != 0.0f)
		{
			totalTime_floatPart = Stringf("%.2f", line.m_totalTime_floatPart);
			totalTime_floatPart = totalTime_floatPart.substr(totalTime_floatPart.find_first_of('.'));
		}
		g_theRenderer->DrawText2D(Vector2(totalTimeX, offset.y), totalTime_intPart, m_fontHeight, m_intColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(totalTimeX + (totalTime_intPart.size() * fontWidth), offset.y), totalTime_floatPart, m_fontHeight, m_floatColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(totalTimeX + ((totalTime_intPart.size() + totalTime_floatPart.size()) * fontWidth), offset.y), line.m_totalTime_units, m_fontHeight, m_unitColor, m_fontAspect, m_font);

		
		// Self Percent
		std::string selfPercent_intPart = std::to_string(line.m_selfTimePercent_intPart);
		std::string selfPercent_floatPart = Stringf("%.2f", line.m_selfTimePercent_floatPart);
		selfPercent_floatPart = selfPercent_floatPart.substr(selfPercent_floatPart.find_first_of('.'));
		g_theRenderer->DrawText2D(Vector2(selfPercentX, offset.y), selfPercent_intPart, m_fontHeight, m_intColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(selfPercentX + ( selfPercent_intPart.size() * fontWidth), offset.y), selfPercent_floatPart, m_fontHeight, m_floatColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(selfPercentX + ((selfPercent_intPart.size() + selfPercent_floatPart.size()) * fontWidth), offset.y), "%", m_fontHeight, m_intColor, m_fontAspect, m_font);
		
		
		// Total Time
		std::string selfTime_intPart = std::to_string(line.m_selfTime_intPart);
		std::string selfTime_floatPart = "";
		if (line.m_selfTime_floatPart != 0.0f)
		{
			selfTime_floatPart = Stringf("%.2f", line.m_selfTime_floatPart);
			selfTime_floatPart = selfTime_floatPart.substr(selfTime_floatPart.find_first_of('.'));
		}
		g_theRenderer->DrawText2D(Vector2(selfTimeX, offset.y), selfTime_intPart, m_fontHeight, m_intColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(selfTimeX + ( selfTime_intPart.size() * fontWidth), offset.y), selfTime_floatPart, m_fontHeight, m_floatColor, m_fontAspect, m_font);
		g_theRenderer->DrawText2D(Vector2(selfTimeX + ((selfTime_intPart.size() + selfTime_floatPart.size()) * fontWidth), offset.y), line.m_selfTime_units, m_fontHeight, m_unitColor, m_fontAspect, m_font);
		
	}
	
	delete report;
}



bool ProfilerVisualizer::IsVisible() const
{
	return m_isVisible;
}



void ProfilerVisualizer::SetVisibilityState(bool isVisible)
{
	if (!Profiler_IsCompiledIn())
	{
		return;
	}

	m_isVisible = isVisible;

	SetMouseCaptureState(false);
}



void ProfilerVisualizer::ToggleVisibilityState()
{
	SetVisibilityState(!IsVisible());
}



void ProfilerVisualizer::ToggleTreeView()
{
	if (m_viewMode == PROFILER_VISUALIZER_TREE_VIEW_MODE_FLAT)
	{
		m_viewMode = PROFILER_VISUALIZER_TREE_VIEW_MODE_NESTED;
	}
	else
	{
		m_viewMode = PROFILER_VISUALIZER_TREE_VIEW_MODE_FLAT;
	}
}



void ProfilerVisualizer::ToggleFlatView()
{
	if (m_flatViewMode == PROFILER_VISUALIZER_FLAT_VIEW_MODE_TOTAL_TIME)
	{
		m_flatViewMode = PROFILER_VISUALIZER_FLAT_VIEW_MODE_SELF_TIME;
	}
	else
	{
		m_flatViewMode = PROFILER_VISUALIZER_FLAT_VIEW_MODE_TOTAL_TIME;
	}
}



bool ProfilerVisualizer::IsCapturingMouse() const
{
	bool isCapturing = false;

	if (IsVisible() && m_isCapturingMouse)
	{
		isCapturing = true;
	}

	return isCapturing;
}



void ProfilerVisualizer::SetMouseCaptureState(bool shouldCaptureMouse)
{
	if (m_isCapturingMouse == shouldCaptureMouse)
	{
		return;
	}

	m_isCapturingMouse = shouldCaptureMouse;

	if (IsCapturingMouse())
	{
		m_previousUsersMouseMode = g_theInput->GetMouseMode();
		m_previousUsersMouseVisibility = g_theInput->IsMouseVisible();
		g_theInput->SetMouseMode(MOUSE_MODE_ABSOLUTE);
		g_theInput->SetMouseVisiblity(true);
	}
	else
	{
		g_theInput->SetMouseMode(m_previousUsersMouseMode);
		g_theInput->SetMouseVisiblity(m_previousUsersMouseVisibility);
	}
}



void ProfilerVisualizer::ToggleMouseCaptureState()
{
	SetMouseCaptureState(!m_isCapturingMouse); // Dont think we want to account for visibility?
}
