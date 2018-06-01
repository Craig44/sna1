#pragma once // This is a call to the compiler to say that if included in a source file, only to parse it once <http://en.cppreference.com/w/cpp/preprocessor/impl>

#include "random.hpp"

#include "parameters.hpp"

#include "environ.hpp"
#include "fishes.hpp"
#include "harvest.hpp"
#include "monitor.hpp"

using namespace std;
/**
 * The model
 *
 * Links together the sub-models e.g `Environ`, 'Fishes` and `Harvest`
 */
class Model {
 public:

    Environ this_environ; // DO NOT call this environ, that is a macro in stdlib.h on windows which caused me many hours of suffering trying to find.
    Fishes fishes;
    Harvest harvest;
    Monitor monitor;

    void initialise(void) {
        parameters.initialise();
        this_environ.initialise();
        fishes.initialise();
        harvest.initialise();
        monitor.initialise();
    }

    void finalise(void) {
        parameters.finalise();
        this_environ.finalise();
        fishes.finalise();
        harvest.finalise();
        monitor.finalise();
    }

    /**
     * The main update function for the model
     * called at each time step? every year
     *
     * This is optimised to reduce the number of loops through
     * the population of fish
     */
    void update(void) {
        auto y = year(now);
        bool burnin = (y < Years_min);

        if (y == Years_min)
            monitor.update_initial_partition(fishes);
        // Reset the monitoring counts
        if (not burnin) {
        	monitor.reset();
        }
        /*****************************************************************
         * Spawning and recruitment
         ****************************************************************/
        
        // Update spawning biomass
        fishes.biomass_spawners_update();
      #ifdef DEBUG
      cerr << "complete spawning" << endl;
      #endif
        // Update recruitment
        fishes.recruitment_update();
        #ifdef DEBUG
        cerr << "calculate recruits" << endl;
        #endif
        // Create and insert each recruit into the population
        unsigned int slot = 0;
        for (auto region : regions) {
            for (unsigned int index = 0; index < fishes.recruitment_instances(region); index++){
                Fish recruit;
                recruit.born(Region(region.index()), this_environ);

                // Find a "slot" in population to insert this recruit
                // An empty 'slot' is created when previous fish in the partition die.
                // If no empty slot found add to end of fish population
                while (slot < fishes.size()) {
                    if (not fishes[slot].alive()) {
                        fishes[slot] = recruit;
                        break;
                    } else {
                        slot++;
                    }
                }
                if (slot == fishes.size()) {
                    fishes.push_back(recruit);
                }
            }
        }
        #ifdef DEBUG
        cerr << "finished recruitss" << endl;
        #endif
        /*****************************************************************
         * Fish population dynamics
         ****************************************************************/
        for (Fish& fish : fishes) {
            if (fish.alive()) {
                // This is only half M, I am going to repeat this
                if (fish.survival()) {
                    fish.growth();
                    fish.maturation();
                    //fish.movement();
                    fish.preference_movement();

                    fish.shedding();

                    if (not burnin) {
                        monitor.population(fish);
                    }

                }
            }
        }
        // Don't go further if in burn in
        if (burnin) {
        	return;
        }


        /*****************************************************************
         * Monitoring (independent of harvesting e.g. tag release)
         *
         * Done before harvesting to allow simulation of tags recaptured
         * in the same time step as release
         ****************************************************************/

        int releases_targetted = 0;
        for(auto region : regions) {
            for(auto method : methods) {
                releases_targetted += parameters.tagging_releases(y, region, method);
            }
        }
        int releases_done = 0;

        unsigned int trials = 0;
        while(releases_done < releases_targetted) {
            // Randomly choose a fish
            Fish& fish = fishes[chance()*fishes.size()];
            // If the fish is alive, and not yet tagged then...
            if (fish.alive() and not fish.tag and fish.length >= monitor.tagging.release_length_min) {
                // Randomly choose a fishing method in the region the fish currently resides
                auto method = Method(methods.select(chance()).index());
                auto region = fish.region;
                // If the tag releases for the method in the region is not yet acheived...
                if (monitor.tagging.released(y, region, method) < parameters.tagging_releases(y, region, method)) {
                    // Is this fish caught by this method?
                    auto selectivity = harvest.selectivity_at_length(method, fish.length_bin());
                    if ((!monitor.tagging.release_length_selective) || (chance() < selectivity)) {
                        // Tag and release the fish
                        monitor.tagging.release(fish, method);
                        fish.released(method);
                        // Increment the number of releases
                        releases_done++;
                        // Apply tagging mortality
                        if (chance() < parameters.tagging_mortality) fish.dies();
                    }
                }
            }
            // Escape if too many trials
            if (trials++ > fishes.size() * 100) {
                cerr << trials << " " << releases_done << " " << releases_targetted << endl;
                throw runtime_error("Too many attempts to tag fish. Something is probably wrong.");
            }
        }

        /*****************************************************************
         * Harvesting and harvest related monitoring (e.g. CPUE, tag recoveries)
         ****************************************************************/

        // Update the current catches by region/method
        // from the catch history
        harvest.catch_observed_update();

        // Reset the harvesting accounting
        harvest.attempts = 0;
        harvest.catch_taken = 0;
        // Keep track of total catch taken and quit when it is >= observed
        double catch_taken = 0;
        double catch_observed = sum(harvest.catch_observed);

        vector<double> test_sample_at_age(31);
        vector<double> test_numbers_at_age(31);

        if (parameters.debug) {
            cerr << "Entering mortality process: yes" << endl;
        }
        if (parameters.length_based_selectivity) {
            // If there was observed catch then randomly draw fish and "assign" them with varying probabilities
            // to a particular region/method catch
            while(catch_observed > 0) {

                // Randomly choose a fish
                Fish& fish = fishes[chance()*fishes.size()];
                // If the fish is alive, then...
                if (fish.alive()) {
                    auto region = fish.region;

                    // Randomly choose a fishing method in the region the fish currently resides
                    auto method = Method(methods.select(chance()).index());
                    // If the catch for the method in the region is not yet caught...
                    if (harvest.catch_taken(region, method) < harvest.catch_observed(region, method)) {
                        // Is this fish caught by this method?
                        auto selectivity = harvest.selectivity_at_length(method, fish.length_bin());
                        auto boldness = (method == fish.method_last) ? (1 - parameters.fishes_shyness(method)) : 1;
                        if (chance() < selectivity * boldness) {
                            // An additional step of fish we keep only above mls
                            if (fish.length >= parameters.harvest_mls(method)) {
                                // Kill the fish
                                fish.dies();

                                // Add to catch taken for region/method
                                double fish_biomass = fish.weight() * fishes.scalar;
                                harvest.catch_taken(region, method) += fish_biomass;
                                harvest.catch_taken_by_year(region, method, y) += fish_biomass;

                                // Catch sampling, currently 100% sampling of catch
                                monitor.catch_sample(region, method, fish);

                                // Update total catch and quit if all taken
                                catch_taken += fish_biomass;
                                if (catch_taken >= catch_observed) {
                                    break;
                                }

                                // Is this fish scanned for a tag?
                                if (chance() < parameters.tagging_scanning(y, region, method)) {
                                    monitor.tagging.scan(fish, method);
                                }
                            } else {
                                // Does this fish die after released?
                                if (chance() < parameters.harvest_handling_mortality) {
                                    fish.dies();
                                } else {
                                    fish.released(method);
                                }
                            }
                        }
                    }
                    harvest.attempts++;
                    if (harvest.attempts > fishes.size() * 100) {
                        cerr << y << endl
                                  << "Catch taken so far:\n" << harvest.catch_taken << endl
                                  << "Catch observed:\n" << harvest.catch_observed << endl;
                        throw runtime_error("Too many attempts to take catch. Something is probably wrong.");
                    };
                }
            }
        } else {
            // If there was observed catch then randomly draw fish and "assign" them with varying probabilities
            // to a particular region/method catch
            while(catch_observed > 0) {

                // Randomly choose a fish
                Fish& fish = fishes[chance()*fishes.size()];
                // If the fish is alive, then...
                if (fish.alive()) {
                    auto region = fish.region;

                    // Randomly choose a fishing method in the region the fish currently resides
                    auto method = Method(methods.select(chance()).index());
                    // If the catch for the method in the region is not yet caught...
                    if (harvest.catch_taken(region, method) < harvest.catch_observed(region, method)) {
                        // Is this fish caught by this method?
                        auto selectivity = harvest.selectivity_at_age(method, fish.age_bin());
                        if (parameters.debug)
                            cerr << " method = " << method <<  "age_bin " << fish.age_bin() << " " <<  selectivity <<" fish actual age " << fish.age() <<  endl;

                        auto boldness = (method == fish.method_last) ? (1 - parameters.fishes_shyness(method)) : 1;
                        if (chance() <= selectivity * boldness) {
                            // An additional step of fish we keep only above mls
                            //if (fish.length >= parameters.harvest_mls(method)) {
                                // Kill the fish
                                fish.dies();
                                if ((method == 0) & (y == 1980))
                                    test_sample_at_age[fish.age_bin()] ++;

                                // Add to catch taken for region/method
                                double fish_biomass = fish.weight() * fishes.scalar;
                                harvest.catch_taken(region, method) += fish_biomass;
                                harvest.catch_taken_by_year(region, method, y) += fish_biomass;

                                // Catch sampling, currently 100% sampling of catch
                                monitor.catch_sample(region, method, fish);

                                // Update total catch and quit if all taken
                                catch_taken += fish_biomass;
                                if (catch_taken >= catch_observed)
                                    break;

                                // Is this fish scanned for a tag?
                                if (chance() < parameters.tagging_scanning(y, region, method)) {
                                    monitor.tagging.scan(fish, method);
                                }
                            //} else {
                                // Does this fish die after released?
                            //    if (chance() < parameters.harvest_handling_mortality) {
                                    fish.dies();
                            //    } else {
                            //        fish.released(method);
                            //    }
                            //}
                        }
                    }
                    harvest.attempts++;
                    if (harvest.attempts > fishes.size() * 100) {
                        cerr << y << endl
                                  << "Catch taken so far:\n" << harvest.catch_taken << endl
                                  << "Catch observed:\n" << harvest.catch_observed << endl;
                        throw runtime_error("Too many attempts to take catch. Something is probably wrong.");
                    };
                }
            }
        }




        // Update harvest.biomass_vulnerable for use in monioring
        harvest.biomass_vulnerable_update(fishes);

        // Update monitoring
        monitor.update(fishes, harvest);

    }

    /**
     * Take the population to pristine equilibium
     *
     * This method simply calls `update()` and then sets population level attributes
     * like `biomass_spawners_pristine` and `scalar`
     */
    void pristine(Time time, function<void()>* callback = 0, bool called_after_seed = false){

        if (parameters.debug) {
            cerr << "entering pristine: " << "yes" << endl;
        }


        // Set `now` to some arbitrary time (but high enough that fish
        // will have a birth time (unsigned int) greater than 0)
        now = 1;
        // Keep recruitment fixed at a constant level that will produce the
        // wanted `seed_number` of individuals in equilibrium
        fishes.recruitment_mode = 'p';
        double number = 0;
        for (int age = 0; age < 200; age++)
        	number += std::exp(-parameters.fishes_m*age);

        if (parameters.debug) {
            cerr << "number: " << number << endl;
        }

        for (auto region : regions) {
            fishes.recruitment_pristine(region) = 
                parameters.fishes_seed_number/number *
                parameters.fishes_b0(region)/sum(parameters.fishes_b0);
            if (parameters.debug) {
                cerr << "Pristine recruits " << Region(region.index()) << ": " << fishes.recruitment_pristine(region) << endl;
            }
        }
        // start by seeding fishes scalar = 1, find out where this gets updated through out the code
        fishes.scalar = 1;
        fishes.seed(parameters.fishes_seed_number, this_environ);
        // Burn in
        // TODO Currently just burns in for an arbitarty number of iterations
        // Should instead exit when stability in population characteristics
        // Stability is defined as total biomass
        std::vector<unsigned> steps_to_check = {50,100,150,200};
        double equilibrium_tolerance = 0.01; // TODO might want to play with this number and the one above
        steps_to_check.size();
        double initial_biomass = 0;
        int steps = 0;

        int step_limit = 150;
        if (called_after_seed)
        	step_limit = 50;

        while (steps < step_limit) {
        	fishes.biomass_update();
        	initial_biomass = fishes.biomass;
        	if (parameters.debug) {
        	    cerr << "initial_biomass in step " << steps << ": " << initial_biomass << "\n";
        	}
        	// run the time step
            update();
        	if (find(steps_to_check.begin(),steps_to_check.end(),steps) != steps_to_check.end()) {
        		// Check convergence tolerance
        		//std::cerr << "checking convergence at iteration " << steps << "\n";
            	fishes.biomass_update();
            	if (parameters.debug) {
            	    cerr << "current biomass: " << fishes.biomass << endl;
            	    cerr << "diff: " << fabs(fishes.biomass - initial_biomass) << "\n";
            	}

            	if (fabs(fishes.biomass - initial_biomass) < equilibrium_tolerance)
            		break;
        	}
            if (callback)
            	(*callback)();
            steps++;
            now++;
        }
        // Re-calibrate the fishes birth to current time
        // The fish have arbitrary `birth` times so we need to "re-birth"
        // them so that the population is in equilbrium AND "current"
        auto diff = time-now;
        for (auto& fish : fishes) {
            fish.birth += diff;
        }
        now = time;
        // Set scalar so that the current spawner biomass 
        // matches the intended value
        fishes.scalar = sum(parameters.fishes_b0)/sum(fishes.biomass_spawners);
        if (parameters.debug) {
            cerr << "update scalar: " << fishes.scalar << endl;
        }
        // Adjust accordingly
        fishes.biomass_spawners *= fishes.scalar;
        fishes.recruitment_pristine *= fishes.scalar;
        // Go to "normal" recruitment
        fishes.recruitment_mode = 'n';
    }

    /**
     * @brief      Run the model over a time period, starting in pristine conditions
     *
     * @param[in]  start     The start
     * @param[in]  finish    The finish
     * @param      callback  The callback function (can be used to output)
     */
    void run(Time start, Time finish, std::function<void()>* callback = 0, int initial = 0) {
        // Create initial population of fish or add a million fish, not sure if that else is ever called though
    	// the

        // I am going to pipe out my debug report to the log file
        #ifdef DEBUG
        cerr << "message: " << "value" << endl;
        cerr << "initial: " << initial << endl;
        #endif

        if (initial == 0) {
        	pristine(start, callback, false);
        } else {
            // Currently as this code stands this will never get executed perhaps ask someone the purpose of this
        	fishes.seed(1e6,this_environ);
        	pristine(start, callback, true);

        }

        // Have a look at
        // Iterate over years
        now = start;
        while (now <= finish) {
            if (parameters.debug) {
                cerr << "entering year: " << now << endl;
            }
            update();
            if (callback) (*callback)();
            now++;
        }
    }

};  // end class Model
