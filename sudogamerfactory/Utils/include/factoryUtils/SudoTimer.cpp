#include "SudoTimer.h"

using namespace std::chrono;

SudoTimer::SudoTimer() noexcept
{
	last = steady_clock::now();
}

float SudoTimer::Mark() noexcept
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

float SudoTimer::Peek() const noexcept
{
	return duration<float>( steady_clock::now() - last ).count();
}
