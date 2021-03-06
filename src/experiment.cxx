#include "ana/experiment.h"

#include <exception>

ana::counter::experiment::experiment(double norm) :
	selection::cutflow(),
	m_norm(norm)
{}

void ana::counter::experiment::add(ana::counter& cnt)
{
	m_counters.push_back(&cnt);
}

void ana::counter::experiment::clear_counters()
{
	m_counters.clear();
}