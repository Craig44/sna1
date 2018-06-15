#include <boost/test/unit_test.hpp>

#include "Agents/Agents.cpp"
#include "Agents/Agent.cpp"
#include "Model/Model.h"

BOOST_AUTO_TEST_SUITE(agent)

BOOST_AUTO_TEST_CASE(birth){
	Agent agent;
	agent.born(HG);

	BOOST_CHECK(agent.alive());

	BOOST_CHECK_EQUAL(agent.get_region(), HG);

	BOOST_CHECK_EQUAL(agent.age(), 0);
	BOOST_CHECK_EQUAL(agent.age_bin(), 0);

	BOOST_CHECK_EQUAL(agent.length_, 0);
	BOOST_CHECK_EQUAL(agent.length_bin(), 0);
}

BOOST_AUTO_TEST_CASE(seed){
	Agent agent;
	agent.seed();

	BOOST_CHECK(agent.alive());

	BOOST_CHECK(agent.age() > 0);
	BOOST_CHECK(agent.length_ > 0);
}

// Runs fish movement over many time steps and many 
// fish and calculates the resulting distribution of fish 
// across regions for each home region
Array<double, Regions, RegionTos> movement_run(void) {
  Model* model = nullptr;
	Agents agents(5000,model);

	int count = 0;
	for (auto& fish : agents) {
		fish.born(Region(count++ % 3));
	}

	for (int t=0; t<100; t++) {
		for (auto& fish : agents) {
			fish.movement();
		}
	}

	Array<double, Regions, RegionTos> dist;
	for (auto& fish : agents) {
		dist(fish.home_, fish.region_)++;
	}
	dist /= agents.size()/3;

	return dist;
}

BOOST_AUTO_TEST_CASE(movement_none){
	parameters.fishes_movement_type = 'n';
	parameters.fishes_movement = {};

	auto dist = movement_run();

	BOOST_CHECK_CLOSE(dist(EN, EN), 1.0, 1);
	BOOST_CHECK(dist(EN, HG) < 0.00001);
	BOOST_CHECK(dist(EN, BP) < 0.00001);

	BOOST_CHECK(dist(HG, EN) < 0.00001);
	BOOST_CHECK_CLOSE(dist(HG, HG), 1.0, 1);
	BOOST_CHECK(dist(HG, BP) < 0.00001);

	BOOST_CHECK(dist(BP, EN) < 0.00001);
	BOOST_CHECK(dist(BP, HG) < 0.00001);
	BOOST_CHECK_CLOSE(dist(BP, BP), 1.0, 1);
	
	parameters.initialise();
}

BOOST_AUTO_TEST_CASE(movement_markov){
	parameters.fishes_movement_type = 'm';
	parameters.fishes_movement = {
		0.8, 0.1, 0.1,
		0.1, 0.8, 0.1,
		0.1, 0.1, 0.8
	};

	auto dist = movement_run();

	BOOST_CHECK_SMALL(dist(EN, EN) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(EN, HG) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(EN, BP) - 0.333, 0.05);

	BOOST_CHECK_SMALL(dist(HG, EN) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(HG, HG) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(HG, BP) - 0.333, 0.05);

	BOOST_CHECK_SMALL(dist(BP, EN) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(BP, HG) - 0.333, 0.05);
	BOOST_CHECK_SMALL(dist(BP, BP) - 0.333, 0.05);

	parameters.initialise();
}

BOOST_AUTO_TEST_CASE(movement_home){
	parameters.fishes_movement_type = 'h';
	parameters.fishes_movement = {
		0.8, 0.1, 0.1,
		0.1, 0.7, 0.2,
		0.1, 0.3, 0.6
	};

	auto dist = movement_run();

	BOOST_CHECK_SMALL(dist(EN, EN) - 0.8, 0.05);
	BOOST_CHECK_SMALL(dist(EN, HG) - 0.1, 0.05);
	BOOST_CHECK_SMALL(dist(EN, BP) - 0.1, 0.05);

	BOOST_CHECK_SMALL(dist(HG, EN) - 0.1, 0.05);
	BOOST_CHECK_SMALL(dist(HG, HG) - 0.7, 0.05);
	BOOST_CHECK_SMALL(dist(HG, BP) - 0.2, 0.05);

	BOOST_CHECK_SMALL(dist(BP, EN) - 0.1, 0.05);
	BOOST_CHECK_SMALL(dist(BP, HG) - 0.3, 0.05);
	BOOST_CHECK_SMALL(dist(BP, BP) - 0.6, 0.05);

	parameters.initialise();
}

BOOST_AUTO_TEST_SUITE_END()
