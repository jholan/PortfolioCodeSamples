#pragma once

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Core/RGBA.hpp"

#include "Engine/Math/AABB2D.hpp"

class Camera;
class BitmapFont;
class Texture;




enum ProfilerVisualizerTreeViewMode
{
	PROFILER_VISUALIZER_TREE_VIEW_MODE_INVALID = -1,

	PROFILER_VISUALIZER_TREE_VIEW_MODE_NESTED = 0,
	PROFILER_VISUALIZER_TREE_VIEW_MODE_FLAT,

	PROFILER_VISUALIZER_TREE_VIEW_MODE_COUNT
};

enum ProfilerVisualizerFlatViewMode
{
	PROFILER_VISUALIZER_FLAT_VIEW_MODE_INVALID = -1,

	PROFILER_VISUALIZER_FLAT_VIEW_MODE_TOTAL_TIME = 0,
	PROFILER_VISUALIZER_FLAT_VIEW_MODE_SELF_TIME,

	PROFILER_VISUALIZER_FLAT_VIEW_MODE_COUNT
};



class ProfilerVisualizer
{
public:
	// Composition
	ProfilerVisualizer() {};
	~ProfilerVisualizer() {};

	void Initialize();
	void Destroy();

	// Updating
	void BeginFrame();
	void EndFrame();

	// Input
	void ProcessInput();

	// Rendering
	void Render();

	// Visibility
	bool IsVisible() const;
	void SetVisibilityState(bool isVisible);
	void ToggleVisibilityState();

	// Views
	void ToggleTreeView();
	void ToggleFlatView();

	// Mouse
	bool IsCapturingMouse() const;
	void SetMouseCaptureState(bool shouldCaptureMouse);
	void ToggleMouseCaptureState(); 



private:
	bool							m_isVisible = false;

	// Tree view
	ProfilerVisualizerTreeViewMode	m_viewMode		= PROFILER_VISUALIZER_TREE_VIEW_MODE_NESTED;			// Flat vs Nested
	ProfilerVisualizerFlatViewMode	m_flatViewMode	= PROFILER_VISUALIZER_FLAT_VIEW_MODE_TOTAL_TIME;	// Self vs Total

	// Input
	bool							m_isCapturingMouse = false;
	bool							m_previousUsersMouseVisibility;
	MouseMode						m_previousUsersMouseMode;

	// Graph
	AABB2D							m_graphRegion;
	bool							m_showingSpecificGraph = false;
	int								m_specificGraph;
	
	// Colors
	RGBA							m_profilerBackgroundColor	= RGBA(48,63,159, 192);
	RGBA							m_graphBackgroundColor		= RGBA(144,202,249);
	
	RGBA							m_colorGood					= RGBA(139,195,74);
	RGBA							m_colorOk					= RGBA(255,235,59);
	RGBA							m_colorBad					= RGBA(244,67,54);
	RGBA							m_selectedReportColor		= RGBA(33,150,243);

	RGBA							m_nameFrontColor = RGBA(119, 199, 99);
	RGBA							m_nameBackColor = RGBA();
	RGBA							m_intColor = RGBA();
	RGBA							m_floatColor = RGBA(160, 130, 189);
	RGBA							m_unitColor = RGBA(119, 199, 99);

	// Offsets
	const int						INDENT_SIZE = 1;
	const int						NAME_SIZE = 72;
	const int						CALL_COUNT_SIZE = 8;
	const int						ELAPSED_PERCENT_SIZE = 10;
	const int						ELAPSED_TIME_SIZE = 14;
	const int						SELF_PERCENT_SIZE = 10;
	const int						SELF_TIME_SIZE = 14;


	// Rendering
	Camera*							m_camera		= nullptr;

	const BitmapFont*				m_font			= nullptr;
	float							m_fontHeight	= 11.0f;
	float							m_titleFontHeight = 16.0f;
	float							m_fontAspect	= 1.0f;

	const Texture*					m_backgroundTexture;
};