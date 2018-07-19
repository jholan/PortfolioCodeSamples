#pragma once

#include "Engine/Core/RGBA.hpp"



class Sampler
{
public:
	Sampler();
	~Sampler();

	bool Initialize(bool isLinear = false, bool shouldUseMipmaps = false, const unsigned int wrapMode = WRAP_MODE_REPEAT, const RGBA& borderColor = RGBA());
	void Destroy();

	unsigned int GetSamplerHandle() const;


	static const Sampler* GetLinearSampler();
	static const Sampler* GetPointSampler();

	static const unsigned int WRAP_MODE_REPEAT;
	static const unsigned int WRAP_MODE_CLAMP_TO_EDGE;
	static const unsigned int WRAP_MODE_CLAMP_TO_BORDER;

private:
	unsigned int m_samplerHandle;


	static const Sampler* LINEAR_SAMPLER;
	static const Sampler* POINT_SAMPLER;
};