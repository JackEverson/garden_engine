#include <print>

#include "Approacher.hpp"


Approacher::Approacher(float distance, float speed_per_second) :
	m_distanceAway(distance),
	m_approachSpeed(speed_per_second)
{
	last_cycle_time = std::chrono::steady_clock::now();
}

Approacher::~Approacher(){

}

void Approacher::Step(){
	if (m_distanceAway == 0 || stopped) {
		return;
	}

	auto now = std::chrono::steady_clock::now();
	auto duration = now - last_cycle_time;
	long long ms =
		std::chrono::duration_cast<std::chrono::milliseconds>(duration)
		.count();

	float dist = ((float)ms / 1000) * m_approachSpeed;

	float new_dist = m_distanceAway - dist;

	if (new_dist < 0) m_distanceAway = 0;
	else m_distanceAway = new_dist;
	
	last_cycle_time = now;

}
