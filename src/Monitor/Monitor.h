#pragma once

#include "Agents.h"
#include "Harvest.h"
#include "Tagging.h"


class Monitor {
 public:

    void initialise(void);
    void finalise(std::string directory = "output/monitor");
    void reset();
    void population(const Agent& agent);
    void catch_sample(Region region, Method method, const Agent& agent);
    void update_initial_partition(const Agents& agents);
    void update(const Agents& agents, const Harvest& harvest);

    Tagging tagging_;
    MonitoringComponents                        components_;              // An optimization to store the current year's monitoring components
    Array<int, Years, Regions>                  population_numbers_;       // Population numbers by year and region
    Array<double, Regions, Lengths>             population_lengths_sample_; //Population length distribution by region for current year
    Array<double, Years, Regions>               biomass_spawners_;
    Array<unsigned int, Years, Regions>         recruits_;
    Array<unsigned int, Ages, Regions>          initial_numbers_at_age_;
    Array<double, Years, Regions, Methods>      catches_;
    Array<double, Regions, Methods>             cpue_;
    Array<double, Years, Regions, Methods>      cpues_;
    Array<double, Regions, Methods, Ages>       age_sample_;
    Array<double, Years, Regions, Methods, Ages> age_samples_;
    Array<double, Years, Regions, Ages>         numbers_at_age_;
    Array<double, Regions, Methods, Lengths>    length_sample_;
    Array<double, Years, Regions, Methods, Lengths> length_samples_;
};  // class Monitor
