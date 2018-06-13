#pragma once // This is a call to the compiler to say that if included in a source file, only to parse it once <http://en.cppreference.com/w/cpp/preprocessor/impl>

#include "Agents.cpp"
#include "Environment.cpp"
#include "Harvest.cpp"
#include "Monitor.cpp"
#include "Agent.cpp"

#include "Random.h"
#include "Parameters.h"

#include "Model.h"

/**
 * The model
 *
 * Links together the sub-models e.g `Environment`, 'Agents` and `Harvest`
 */



void Model::initialise(void) {
  parameters.initialise();
  environemnt->initialise();
  agents.initialise();
  harvest.initialise();
  monitor.initialise();
}

void Model::finalise(void) {
  parameters.finalise();
  //environemnt->finalise();
  //agents.finalise();
  harvest.finalise();
  monitor.finalise();
}

/**
* The main update function for the model
* called at each time step? every year
*
* This is optimised to reduce the number of loops through
* the population of agent
 */
void Model::update(void) {
  auto y = year(now);
  bool burnin = (y < Years_min);

  if (y == Years_min)
    monitor.update_initial_partition(agents);
// Reset the monitoring counts
  if (not burnin) {
    monitor.reset();
  }
  /*****************************************************************
   * Spawning and recruitment
   ****************************************************************/

// Update spawning biomass
  agents.biomass_spawners_update();
#ifdef DEBUG
  cerr << "complete spawning" << endl;
#endif
// Update recruitment
  agents.recruitment_update();
#ifdef DEBUG
  cerr << "calculate recruits" << endl;
#endif
// Create and insert each recruit into the population
  unsigned int slot = 0;
  for (auto region : regions) {

    for (unsigned int index = 0; index < agents.recruitment_instances_(region); index++) {
      Agent recruit;
      recruit.born(Region(region.index()), environemnt);

      // Find a "slot" in population to insert this recruit
      // An empty 'slot' is created when previous agent in the partition die.
      // If no empty slot found add to end of agent population
      while (slot < agents.size()) {
        if (not agents[slot].alive()) {
          agents[slot] = recruit;
          break;
        } else {
          slot++;
        }
      }
      if (slot == agents.size()) {
        agents.push_back(recruit);
      }
    }
  }
#ifdef DEBUG
  cerr << "finished recruits" << endl;
#endif
  /*****************************************************************
   * Agent population dynamics
   ****************************************************************/
  unsigned counter = 0;
  for (Agent& agent : agents) {
    if (agent.alive()) {
      // This is only half M, I am going to repeat this
      if (agent.survival()) {
        agent.growth();
        agent.maturation();
        //agent.movement();
        agent.preference_movement();
        agent.shedding();

        if (not burnin) {
          monitor.population(agent);
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
  /*

   int releases_targetted = 0;
   for (auto region : regions) {
   for (auto method : methods) {
   releases_targetted += parameters.tagging_releases(y, region, method);
   }
   }
   int releases_done = 0;

   unsigned int trials = 0;

   while (releases_done < releases_targetted) {
   // Randomly choose a agent
   Agent& agent = agents[chance() * agents.size()];
   // If the agent is alive, and not yet tagged then...
   if (agent.alive() and not agent.get_tag()
   and agent.get_length() >= monitor.tagging.release_length_min) {
   // Randomly choose a fishing method in the region the agent currently resides
   auto method = Method(methods.select(chance()).index());
   auto region = agent.get_region();
   // If the tag releases for the method in the region is not yet acheived...
   if (monitor.tagging.released(y, region, method)
   < parameters.tagging_releases(y, region, method)) {
   // Is this agent caught by this method?
   auto selectivity = harvest.selectivity_at_length(method,
   agent.length_bin());
   if ((!monitor.tagging.release_length_selective)
   || (chance() < selectivity)) {
   // Tag and release the agent
   monitor.tagging.release(agent, method);
   agent.released(method);
   // Increment the number of releases
   releases_done++;
   // Apply tagging mortality
   if (chance() < parameters.tagging_mortality)
   agent.dies();
   }
   }
   }
   // Escape if too many trials
   if (trials++ > agents.size() * 100) {
   cerr << trials << " " << releases_done << " " << releases_targetted
   << endl;
   throw runtime_error(
   "Too many attempts to tag agent. Something is probably wrong.");
   }
   }

   */
  /*****************************************************************
   * Harvesting and harvest related monitoring (e.g. CPUE, tag recoveries)
   ****************************************************************/

  // Update the current catches by region/method
  // from the catch history
  harvest.catch_observed_update();

  // Reset the harvesting accounting
  harvest.attempts_ = 0;
  harvest.catch_taken_ = 0;
  // Keep track of total catch taken and quit when it is >= observed
  double catch_taken = 0;
  double catch_observed = sum(harvest.catch_observed_);

  if (parameters.debug) {
    cerr << "Entering mortality process: yes" << endl;
  }
  if (parameters.length_based_selectivity) {
    // If there was observed catch then randomly draw agent and "assign" them with varying probabilities
    // to a particular region/method catch
    while (catch_observed > 0) {

      // Randomly choose a agent
      Agent& agent = agents[chance() * agents.size()];
      // If the agent is alive, then...
      if (agent.alive()) {
        auto region = agent.get_region();

        // Randomly choose a fishing method in the region the agent currently resides
        auto method = Method(methods.select(chance()).index());
        // If the catch for the method in the region is not yet caught...
        if (harvest.catch_taken_(region, method) < harvest.catch_observed_(region, method)) {
          // Is this agent caught by this method?
          auto selectivity = harvest.selectivity_at_length_(method, agent.length_bin());
          auto boldness =
              (method == agent.get_method_last()) ? (1 - parameters.fishes_shyness(method)) : 1;
          if (chance() < selectivity * boldness) {
            // An additional step of agent we keep only above mls
            if (agent.get_length() >= parameters.harvest_mls(method)) {
              // Kill the agent
              agent.dies();

              // Add to catch taken for region/method
              double fish_biomass = agent.weight() * agents.get_scalar();
              harvest.catch_taken_(region, method) += fish_biomass;
              harvest.catch_taken_by_year_(region, method, y) += fish_biomass;

              // Catch sampling, currently 100% sampling of catch
              monitor.catch_sample(region, method, agent);

              // Update total catch and quit if all taken
              catch_taken += fish_biomass;
              if (catch_taken >= catch_observed) {
                break;
              }

              // Is this agent scanned for a tag?
              /*                  if (chance() < parameters.tagging_scanning(y, region, method)) {
               monitor.tagging.scan(agent, method);
               }*/
            } else {
              // Does this agent die after released?
              if (chance() < parameters.harvest_handling_mortality) {
                agent.dies();
              } else {
                agent.released(method);
              }
            }
          }
        }
        harvest.attempts_++;
        if (harvest.attempts_ > agents.size() * 100) {
          cerr << y << endl << "Catch taken so far:\n" << harvest.catch_taken_ << endl << "Catch observed:\n" << harvest.catch_observed_ << endl;
          throw runtime_error("Too many attempts to take catch. Something is probably wrong.");
        };
      }
    }
  } else {
    // If there was observed catch then randomly draw agent and "assign" them with varying probabilities
    // to a particular region/method catch
    while (catch_observed > 0) {

      // Randomly choose a agent
      Agent& agent = agents[chance() * agents.size()];
      // If the agent is alive, then...
      if (agent.alive()) {
        auto region = agent.get_region();

        // Randomly choose a fishing method in the region the agent currently resides
        auto method = Method(methods.select(chance()).index());
        // If the catch for the method in the region is not yet caught...
        if (harvest.catch_taken_(region, method) < harvest.catch_observed_(region, method)) {
          // Is this agent caught by this method?
          auto selectivity = harvest.selectivity_at_age_(method, agent.age_bin());
          if (parameters.debug)
            cerr << " method = " << method << "age_bin " << agent.age_bin()
                << " " << selectivity << " agent actual age " << agent.age()
                << endl;

          auto boldness =
              (method == agent.get_method_last()) ? (1 - parameters.fishes_shyness(method)) : 1;
          if (chance() <= selectivity * boldness) {
            // An additional step of agent we keep only above mls
            //if (agent.length >= parameters.harvest_mls(method)) {
            // Kill the agent
            agent.dies();

            // Add to catch taken for region/method
            double fish_biomass = agent.weight() * agents.get_scalar();
            harvest.catch_taken_(region, method) += fish_biomass;
            harvest.catch_taken_by_year_(region, method, y) += fish_biomass;

            // Catch sampling, currently 100% sampling of catch
            monitor.catch_sample(region, method, agent);

            // Update total catch and quit if all taken
            catch_taken += fish_biomass;
            if (catch_taken >= catch_observed)
              break;

            // Is this agent scanned for a tag?
            /*                if (chance() < parameters.tagging_scanning(y, region, method)) {
             monitor.tagging.scan(agent, method);
             }*/
            //} else {
            // Does this agent die after released?
            //    if (chance() < parameters.harvest_handling_mortality) {
            agent.dies();
            //    } else {
            //        agent.released(method);
            //    }
            //}
          }
        }
        harvest.attempts_++;
        if (harvest.attempts_ > agents.size() * 100) {
          cerr << y << endl << "Catch taken so far:\n" << harvest.catch_taken_ << endl << "Catch observed:\n" << harvest.catch_observed_ << endl;
          throw runtime_error("Too many attempts to take catch. Something is probably wrong.");
        };
      }
    }
  }

  // Update harvest.biomass_vulnerable for use in monioring
  harvest.biomass_vulnerable_update(agents);

  // Update monitoring
  monitor.update(agents, harvest);
}

/**
* Take the population to pristine equilibium
*
* This method simply calls `update()` and then sets population level attributes
* like `biomass_spawners_pristine` and `scalar`
*/
void Model::pristine(Time time, function<void()>* callback, bool called_after_seed) {

  cout << "size of an agent = " << sizeof(agents[0]) << endl;
// Set `now` to some arbitrary time (but high enough that agent
// will have a birth time (unsigned int) greater than 0)
  now = 1;
// Keep recruitment fixed at a constant level that will produce the
// wanted `seed_number` of individuals in equilibrium
  agents.recruitment_mode_ = 'p';
  double number = 0;
  for (int age = 0; age < 200; age++)
    number += std::exp(-parameters.fishes_m * age);

  if (parameters.debug) {
    cerr << "number: " << number << endl;
  }

  for (auto region : regions) {
    agents.recruitment_pristine_(region) = parameters.fishes_seed_number
        / number * parameters.fishes_b0(region) / sum(parameters.fishes_b0);
  }
// start by seeding agents scalar = 1, find out where this gets updated through out the code
  agents.set_scalar(1.0);
  agents.seed(parameters.fishes_seed_number);
// Burn in
// TODO Currently just burns in for an arbitarty number of iterations
// Should instead exit when stability in population characteristics
// Stability is defined as total biomass
  std::vector<unsigned> steps_to_check = { 50, 100, 150, 200 };
  double equilibrium_tolerance = 0.0001; // TODO might want to play with this number and the one above
  steps_to_check.size();
  double initial_biomass = 0;
  int steps = 0;

  int step_limit = 150;
  if (called_after_seed)
    step_limit = 50;

  while (steps < step_limit) {
    agents.biomass_update();
    initial_biomass = agents.get_biomass();
    if (parameters.debug) {
      cerr << "initial_biomass in step " << steps << ": " << initial_biomass
          << "\n";
    }
    // run the time step
    update();
    if (find(steps_to_check.begin(), steps_to_check.end(), steps)
        != steps_to_check.end()) {
      // Check convergence tolerance
      //std::cerr << "checking convergence at iteration " << steps << "\n";
      agents.biomass_update();
      if (parameters.debug) {
        cerr << "current biomass: " << agents.get_biomass() << endl;
        cerr << "diff: " << fabs(agents.get_biomass() - initial_biomass)
            << "\n";
      }

      if (fabs(agents.get_biomass() - initial_biomass) < equilibrium_tolerance)
        break;
    }
    if (callback)
      (*callback)();
    steps++;
    now++;
  }
// Re-calibrate the agents birth to current time
// The agent have arbitrary `birth` times so we need to "re-birth"
// them so that the population is in equilbrium AND "current"
  auto diff = time - now;
  for (auto& agent : agents) {
    agent.set_birth(agent.get_birth() + diff);
  }
  now = time;
// Set scalar so that the current spawner biomass
// matches the intended value
  agents.set_scalar(
      sum(parameters.fishes_b0) / sum(agents.biomass_spawners_));
  if (parameters.debug) {
    cerr << "update scalar: " << agents.get_scalar() << endl;
  }
// Adjust accordingly
  agents.biomass_spawners_ *= agents.get_scalar();
  agents.recruitment_pristine_ *= agents.get_scalar();
// Go to "normal" recruitment
  agents.recruitment_mode_ = 'n';
}

/**
* @brief      Run the model over a time period, starting in pristine conditions
*
* @param[in]  start     The start
* @param[in]  finish    The finish
* @param      callback  The callback function (can be used to output)
*/
void Model::run(Time start, Time finish, std::function<void()>* callback, int initial ) {
  // Create initial population of agent or add a million agent, not sure if that else is ever called though
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
    agents.seed(1e6);
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
    if (callback)
      (*callback)();
    now++;
  }
  cerr << "finished annual cycle" << endl;
}

// end class Model
