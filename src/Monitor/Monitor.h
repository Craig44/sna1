#pragma once

//#include "Tagging.h"
#include "Agents.h"
#include "Harvest.h"


class Monitor {
 public:

    void initialise(void);
    void finalise(std::string directory = "output/monitor");
    void reset();
    void population(const Agent& agent);
    void catch_sample(Region region, Method method, const Agent& agent);
    void update_initial_partition(const Agents& agents);
    void update(const Agents& agents, const Harvest& harvest);

    MonitoringComponents                        components;              // An optimization to store the current year's monitoring components
    Array<int, Years, Regions>                  population_numbers;       // Population numbers by year and region
    Array<double, Regions, Lengths>             population_lengths_sample; //Population length distribution by region for current year
    Array<double, Years, Regions>               biomass_spawners;
    Array<unsigned int, Years, Regions>         recruits;
    Array<unsigned int, Ages, Regions>          initial_numbers_at_age;
    Array<double, Years, Regions, Methods>      catches;
    Array<double, Regions, Methods>             cpue;
    Array<double, Years, Regions, Methods>      cpues;
    Array<double, Regions, Methods, Ages>       age_sample;
    Array<double, Years, Regions, Methods, Ages> age_samples;
    Array<double, Years, Regions, Ages>         numbers_at_age;
    Array<double, Regions, Methods, Lengths>    length_sample;
    Array<double, Years, Regions, Methods, Lengths> length_samples;
};  // class Monitor
