#pragma once

#include "Harvest.h"

void Harvest::initialise(void) {
  for (auto method : methods) {
    if (parameters.length_based_selectivity) {
      for (auto length_bin : lengths) {
        auto steep1 = parameters.harvest_sel_steep1(method);
        auto mode = parameters.harvest_sel_mode(method);
        auto steep2 = parameters.harvest_sel_steep2(method);
        auto length = length_mid(length_bin);
        double selectivity;
        if (length <= mode)
          selectivity = std::pow(2, -std::pow((length - mode) / steep1, 2));
        else
          selectivity = std::pow(2, -std::pow((length - mode) / steep2, 2));
        selectivity_at_length_(method, length_bin) = selectivity;
      }
    } else {
      bool double_normal = false;
      double selectivity = 0;
      auto steep1 = parameters.harvest_sel_steep1(method);
      auto mode = parameters.harvest_sel_mode(method);
      auto steep2 = parameters.harvest_sel_steep2(method);
      for (auto age : ages) {
        /*		            if (age <= 4) selectivity = 0;
         else if (age == 5)
         selectivity = 0.5;
         else
         selectivity = 1;*/

        // Case it to a double for use in std functions
        double d_age = (double) age.index();
        if (d_age == 0) {
          selectivity = 0.0;
        } else {
          // if double normal
          if (double_normal) {

            if (d_age < mode)
              selectivity = std::pow(2, -std::pow((d_age - mode) / steep1, 2));
            else
              selectivity = std::pow(2, -std::pow((d_age - mode) / steep2, 2));
          } else {
            selectivity = 1.0 / (1.0 + pow(19.0, (mode - d_age) / steep1));
          }
        }

        selectivity_at_age_(method, age) = selectivity;

        if (parameters.debug) {
          cerr << "Method = " << method << " checky d_age = " << d_age
              << " age = " << age_bin(age.index()) << ": "
              << selectivity_at_age_(method, age) << endl;
        }

      }
    }
  }
}



void Harvest::finalise(void) {
  // Report harvest info
  // Selectivities
  boost::filesystem::create_directories("output/harvest");
  if (parameters.length_based_selectivity)
    selectivity_at_length_.write("output/harvest/selectivity_at_length.tsv");
  else
    selectivity_at_age_.write("output/harvest/selectivity_at_age.tsv");
  // Actual removals
  catch_taken_by_year_.write("output/harvest/actual_catch.tsv");
}

void Harvest::biomass_vulnerable_update(const Agents& agents) {
  biomass_vulnerable_ = 0;
  if (parameters.length_based_selectivity) {
    for (const Agent& agent : agents) {
      if (agent.alive()) {
        auto weight = agent.weight();
        auto length_bin = agent.length_bin();
        for (auto method : methods) {
          biomass_vulnerable_(agent.get_region(), method) += weight
              * selectivity_at_length_(method, length_bin);
        }
      }
    }
  } else {
    for (const Agent& agent : agents) {
      if (agent.alive()) {
        auto weight = agent.weight();
        auto age_bin = agent.age_bin();
        for (auto method : methods) {
          biomass_vulnerable_(agent.get_region(), method) += weight * selectivity_at_age_(method, age_bin);
        }
      }
    }
  }
  biomass_vulnerable_ *= agents.get_scalar();
}

void Harvest::catch_observed_update(void) {
  auto y = year(now);
  if (y >= Years_min and y <= Years_max) {
    for (auto region : regions) {
      for (auto method : methods) {
        auto catches = parameters.harvest_catch_history(y, region, method);
        catch_observed_(region, method) = catches;
      }
    }
  }
}


