#pragma once

#include "Agents.cpp"


/**
 * Fishing activities
 */
class Harvest {
 public:
    Harvest() {};
    ~Harvest() {};
    // Member functions
    void                                    initialise(void);
    void                                    finalise(void);
    void                                    biomass_vulnerable_update(const Agents& agents);
    void                                    catch_observed_update(void);

    // Members containers
    Array<double, Methods, Lengths>         selectivity_at_length_;
    Array<double, Methods, Ages>            selectivity_at_age_;
    Array<double, Regions, Methods>         biomass_vulnerable_;
    Array<double, Regions, Methods>         catch_observed_;
    Array<double, Regions, Methods>         catch_taken_;
    Array<double, Regions, Methods, Years>  catch_taken_by_year_;
    unsigned int                            attempts_;
 private:



};  // class Harvest
