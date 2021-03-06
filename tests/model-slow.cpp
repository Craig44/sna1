#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "../model.hpp"


BOOST_AUTO_TEST_SUITE(model)

// Temporarily skip tagging tests which are currently broken
#if 0

/**
 * A simple tagging "analysis"
 *
 * Uses a very simple Petersen estimate done in an external R script
 */
BOOST_AUTO_TEST_CASE(tagging_simple){
	Model model;
	model.initialise();

	// No movement
	parameters.fishes_movement_type = 'n';

	// Set up tagging program
	auto& monitor = model.monitor;
	auto& tagging = model.monitor.tagging;

	// Tag releases are not affected by the size selectivity of the gear
	tagging.release_length_selective = false;
	
	// Release schedule
	tagging.release_targets = 0;
	tagging.release_targets(2000, EN, LL) = 100000;
	tagging.release_targets(2000, HG, LL) = 100000;
	tagging.release_targets(2000, BP, LL) = 100000;

	tagging.release_years = false;
	tagging.release_years(2000) = true;

	// Recovery schedule
	tagging.recovery_years = false;
	tagging.recovery_years(2000) = true;
	tagging.recovery_years(2001) = true;
	tagging.recovery_years(2002) = true;
	tagging.recovery_years(2003) = true;
	tagging.recovery_years(2004) = true;

	// Record population size in each year
	Array<int, Years, Regions> pop = 0;
	std::function<void()> callback([&](){
		if (year(now) >= 2000) {
			for (const auto& fish : model.fishes) {
				if (fish.alive() and (fish.length > tagging.release_length_min)) {
					pop(year(now), fish.region)++;
				}
			}
			std::cout 
				<< year(now) << "\t" 
				<< model.fishes.number(false) << "\t" 
				<< pop(year(now), EN) << "\t" 
				<< pop(year(now), HG) << "\t" 
				<< pop(year(now), BP) << std::endl;
		}
	});

	// Run the model
	model.run(2000, 2005, &callback);

	// Do checks
	BOOST_CHECK(tagging.number > 0);
	BOOST_CHECK(tagging.tags.size() > 0);

	// Output files for R script
	tagging.write("tests/tagging/simple");
	pop.write("tests/tagging/simple/population.tsv");

	// Run analysis script
	auto ok = std::system("cd tests/tagging/simple && Rscript analysis.R");
	BOOST_CHECK(ok==0);

	// Read in mean error
	std::ifstream file("tests/tagging/simple/error.txt");
	double error;
	file >> error;

	// Check error <5%
	BOOST_CHECK(error < 0.05);
}


/**
 * A more complicated tagging test using an external
 * estimator.
 */
BOOST_AUTO_TEST_CASE(tagging_estimate){
	Model model;
	model.initialise();

	// Exponential growth model with no temporal variation
	// and limited amount of individual variation
	parameters.fishes_growth_model = 'e';
	parameters.fishes_growth_variation = 'i';
    parameters.fishes_k_mean = 0.1;
    parameters.fishes_k_sd = 0.02;
    parameters.fishes_linf_mean = 60;
    parameters.fishes_linf_sd = 10;

	// No movement
	parameters.fishes_movement_type = 'n';

	// Set up tagging program
	auto& monitor = model.monitor;
	auto& tagging = model.monitor.tagging;

	// Tag releases are not affected by the size selectivity of the gear
	tagging.release_length_selective = false;
	
	// Release schedule
	tagging.release_targets = 0;
	tagging.release_targets(2000, EN, LL) = 100000;
	tagging.release_targets(2000, HG, LL) = 100000;
	tagging.release_targets(2000, BP, LL) = 100000;

	tagging.release_years = false;
	tagging.release_years(2000) = true;

	// Recovery schedule
	tagging.recovery_years = false;
	tagging.recovery_years(2000) = true;
	tagging.recovery_years(2001) = true;
	tagging.recovery_years(2002) = true;
	tagging.recovery_years(2003) = true;
	tagging.recovery_years(2004) = true;

	// Record population size in each year
	Array<int, Years, Regions> pop = 0;
	std::function<void()> callback([&](){
		if (year(now) >= 2000) {
			for (const auto& fish : model.fishes) {
				if (fish.alive() and (fish.length > tagging.release_length_min)) {
					pop(year(now), fish.region)++;
				}
			}
			std::cout 
				<< year(now) << "\t" 
				<< model.fishes.number(false) << "\t" 
				<< pop(year(now), EN) << "\t" 
				<< pop(year(now), HG) << "\t" 
				<< pop(year(now), BP) << std::endl;
		}
	});

	// Run the model
	model.run(2000, 2005, &callback);

	// Do checks
	BOOST_CHECK(tagging.number > 0);
	BOOST_CHECK(tagging.tags.size() > 0);

	// Output files for R script
	tagging.write("tests/tagging/simple");
	pop.write("tests/tagging/simple/population.tsv");

	// Run analysis script
	auto ok = std::system("cd tests/tagging/simple && Rscript analysis.R");
	BOOST_CHECK(ok==0);

	// Read in mean error
	std::ifstream file("tests/tagging/simple/error.txt");
	double error;
	file >> error;

	// Check error <5%
	BOOST_CHECK(error < 0.05);
}

#endif
