//-----------------------------------------------------------------------------------------------
// Time.hpp
//
#pragma once
#include <stdint.h>


//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds();

uint64_t GetCurrentTimeInHPC();

double ConvertHPCtoSeconds(uint64_t timeInHPC);
uint64_t ConvertSecondsToHPC(double timeInSeconds);