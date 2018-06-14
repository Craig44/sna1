#pragma once

#include "Tagging.cpp"
#include "Monitor.h"



void Monitor::initialise(void) {
  population_numbers_ = 0;
}

/**
 * Reset things at the start of each time step
 */
void Monitor::reset() {
  auto y = year(now);
  components_ = parameters.monitoring_programme(y);
  population_lengths_sample_ = 0;
  cpue_ = 0;
  age_sample_ = 0;
  length_sample_ = 0;
}

/**
 * Monitor the fish population
 *
 * In reality, we can never sample the true underlying population of fish.
 * This method just allows us to capture some true population statistics for things
 * like examining the precision and bias of our estimates.
 *
 * @param fish   A fish
 */
void Monitor::population(const Agent& agent) {
  auto y = year(now);
  // Add fish to numbers by Year and Region
  population_numbers_(y, agent.get_region())++;
  // Add fish to numbers by Region and Length for current year
  population_lengths_sample_(agent.get_region(), agent.length_bin())++;
  // Tagging specific population monitoring
  //tagging.population(fish);
  }

void Monitor::catch_sample(Region region, Method method, const Agent& agent) {
  if (components_.A)
    age_sample_(region, method, agent.age_bin())++;if
(  components_.L)
  length_sample_(region, method, agent.length_bin())++;
}

/**
 * Update teh numbers at the end of the initial time step
 * Passed Fishes by reference and
 */
void Monitor::update_initial_partition(const Agents& agents) {
  for (auto agent : agents) {
    if (agent.alive()) {
      initial_numbers_at_age_(agent.age_bin(), agent.get_region())++;}
    }
  }
  /**
   * Update things at the end of each time start
   */
void Monitor::update(const Agents& agents, const Harvest& harvest) {
  auto y = year(now);
  for (Agent agent : agents) {
    if (agent.alive())
      numbers_at_age_(y, agent.get_region(), agent.age_bin())++;}

    // Record spawning biomass
  for (auto region : regions) {
    biomass_spawners_(y, region) = agents.biomass_spawners_(region);
  }

  // Record number of recruits
  for (auto region : regions) {
    recruits_(y, region) = agents.recruitment_instances_(region);
  }

  // Record catches
  for (auto region : regions) {
    for (auto method : methods) {
      catches_(y, region, method) = harvest.catch_taken_(region, method);
    }
  }

  // Calculate current CPUE by region and method and store it
  if (components_.C) {
    for (auto region : regions) {
      for (auto method : methods) {
        cpue_(region, method) = harvest.biomass_vulnerable_(region, method);
        cpues_(y, region, method) = cpue_(region, method);
      }
    }
  }

  // Store current age sample
  if (components_.A) {
    for (auto region : regions) {
      for (auto method : methods) {
        for (auto age : ages) {

          age_samples_(y, region, method, age) = age_sample_(region, method, age);
        }
      }
    }
  }

  // Store current length sample
  if (components_.L) {
    for (auto region : regions) {
      for (auto method : methods) {
        for (auto length : lengths)
          length_samples_(y, region, method, length) = length_sample_(region,
              method, length);
      }
    }
  }
}

void Monitor::finalise(std::string directory) {
  boost::filesystem::create_directories(directory);

  //tagging.finalise();

  population_numbers_.write(directory + "/population_numbers.tsv");

  cpues_.write(directory + "/cpues.tsv");
  age_samples_.write(directory + "/age_samples.tsv");
  length_samples_.write(directory + "/length_samples.tsv");

  parameters.monitoring_programme.write(directory + "/programme.tsv", { "cpue","lengths", "ages" }, [](std::ostream& stream, const MonitoringComponents& components) {
    stream << components.C << "\t" << components.L << "\t" << components.A;
  });

  // Files for CASAL
  auto casal_directory = directory + "/casal";
  boost::filesystem::create_directories(casal_directory);

  std::ofstream catch_file(casal_directory + "/catch.tsv");
  catch_file << "year\tregion\tmethod\tcatch\n";

  std::ofstream biomass_file(casal_directory + "/biomass.tsv");
  biomass_file << "year\tregion\tbiomass\n";

  std::ofstream recruit_file(casal_directory + "/recruits.tsv");
  recruit_file << "year\tregion\trecruits\n";

  std::ofstream initial_numbers_at_age_file(
      casal_directory + "/initial_numbers_at_age.tsv");
  initial_numbers_at_age_file << "age\tregion\tnumber_of_agents\n";

  std::ofstream numbers_at_age_file(casal_directory + "/numbers_at_age.tsv");
  numbers_at_age_file << "year\tregion\t";
  for (auto this_age : ages)
    numbers_at_age_file << "age" << this_age << "\t";
  numbers_at_age_file << "\n";

  std::ofstream cpue_file(casal_directory + "/cpue.tsv");
  cpue_file << "year\tregion\tmethod\tcpue\n";

  std::ofstream age_file(casal_directory + "/age.tsv");
  age_file << "year\tregion\tmethod\t";
  for (auto age : ages)
    age_file << "age" << age << "\t";
  age_file << "\n";

  std::ofstream length_file(casal_directory + "/length.tsv");
  length_file << "year\tregion\tmethod\t";
  for (auto length : lengths)
    length_file << "length" << length << "\t";
  length_file << "\n";

  // Override of `method_code` method to output `REC`
  auto method_code = [](Stencila::Level<Methods> method) {
    if (method == RE)
    return std::string("REC");
    else
    return ::method_code(method);
  };

  // Initialisation quantities
  for (auto region : regions) {
    for (auto age : ages)
      initial_numbers_at_age_file << age.index() << "\t" << region << "\t"
          << initial_numbers_at_age_(age.index(), region) << "\n";
  }

  // Year Specific quantities
  for (auto year : years) {
    auto components = parameters.monitoring_programme(year);

    for (auto region : regions) {

      biomass_file << year << "\t" << region_code(region) << "\t"
          << biomass_spawners_(year, region) << "\n";

      recruit_file << year << "\t" << region_code(region) << "\t"
          << recruits_(year, region) << "\n";

      numbers_at_age_file << year << "\t" << region_code(region) << "\t";
      for (auto age : ages)
        numbers_at_age_file << numbers_at_age_(year, region, age) << "\t";
      numbers_at_age_file << "\n";

      for (auto method : methods) {
        catch_file << year << "\t" << region_code(region) << "\t"
            << method_code(method) << "\t" << catches_(year, region, method)
            << "\n";

        if (components.C) {
          cpue_file << year << "\t" << region_code(region) << "\t"
              << method_code(method) << "\t" << cpues_(year, region, method)
              << "\n";
        }

        if (components.A) {
          age_file << year << "\t" << region_code(region) << "\t"
              << method_code(method) << "\t";
          for (auto age : ages)
            age_file << age_samples_(year, region, method, age) << "\t";
          age_file << "\n";
        }

        if (components.L) {
          length_file << year << "\t" << region_code(region) << "\t"
              << method_code(method) << "\t";
          for (auto length : lengths)
            length_file << length_samples_(year, region, method, length) << "\t";
          length_file << "\n";
        }
      }
    }

  }

  // Output parameters to be inserted in 'population.csl'
  std::ofstream population_file(casal_directory + "/parameters.tsv");
  population_file << "par\tvalue\n";

  // Growth parameters
  double growth_20;
  double growth_50;
  double growth_cv;
  double growth_sdmin = parameters.fishes_growth_temporal_sdmin;
  if (parameters.fishes_growth_variation == 't') {
    // Parameters calculated from mean of k and linf
    auto growth_slope = std::exp(-parameters.fishes_k_mean) - 1;
    auto growth_intercept = -growth_slope * parameters.fishes_linf_mean;
    growth_20 = growth_intercept + 20 * growth_slope;
    growth_50 = growth_intercept + 50 * growth_slope;
    growth_cv = parameters.fishes_growth_temporal_cv;
  } else {
    // Parameters calculated by generating 1000 fish and
    // calculating mean and cv of growth parameters
    Mean growth_intercept_mean;
    StandardDeviation growth_intercept_sd;
    Mean growth_slope_mean;
    for (int index = 0; index < 100; index++) {
     Agent agent;
     agent.born(EN);
     growth_intercept_mean.append(agent.growth_intercept_);
     growth_intercept_sd.append(agent.growth_intercept_);
     growth_slope_mean.append(agent.growth_slope_);
   }
    growth_20 = growth_intercept_mean + 20 * growth_slope_mean;
    growth_50 = growth_intercept_mean + 50 * growth_slope_mean;
    growth_cv = growth_intercept_sd / growth_intercept_mean;
  }
  population_file << "growth_20\t" << growth_20 << "\n" << "growth_50\t"
      << growth_50 << "\n" << "growth_cv\t" << growth_cv << "\n"
      << "growth_sdmin\t" << growth_sdmin << "\n";
  population_file.close();

}
