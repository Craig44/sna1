#pragma once

#include "Dimensions.h"
#include "Parameters.h"
#include "Environment.h"
#include "Random.h"
#include "Agent.h"
/**
 * The population/Partition of `Agents`
 *
 * We don't attempt to model every single individual in the population. Instead,
 * the vector of `Agent` objects is intended to be a representative sample of the overall population.
 * The variable, `scalar` is then used to scale other variables, like biomass, to population levels.
 */
class Model;

class Agents : public std::vector<Agent> {
  public:
    // constructor that sets intial vector of fishes = size.
    Agents(int size, Engine* engine, Model* model) :
      std::vector<Agent>(size, Agent()),
      model_(model),
      engine_for_seeding_(engine)
    {}

    void                    initialise(void);
    void                    finalise(void);
    // member functions
    void                    seed(unsigned int number);
    void                    biomass_update(void);
    void                    biomass_spawners_update(void);
    // Accessors
    void                    set_scalar(double scalar) {scalar_ = scalar;};
    double                  get_scalar(void) const {return scalar_;};
    double                  get_biomass(void) const {return biomass_;};
    void                    set_biomass(double biomass) {biomass_ = biomass;};
    void                    recruitment_update(void);
    double                  number(bool scale = true);
    double                  age_mean(void);
    double                  length_mean(void);
    void                    enumerate(void);
    void                    track(void);

    // Public containers Sruggling to implement accessors for the stencilia objects I get cant'convert stencilia::Level<Region> -> Region
    Array<double, Regions>  biomass_spawners_;
    char                    recruitment_mode_ = 'n';
    Array<double, Regions>  recruitment_pristine_;
    Array<double, Regions>  recruitment_;
    Array<unsigned int, Regions> recruitment_instances_;

  private:
    double                  scalar_ = 1.0;    // Population scalar, Used to scale the things like biomass etc from the size of `fishes` to the
    double                  biomass_ = 1.0;         //Current total biomass (t)
    Array<unsigned int,Regions,Sexes,Ages,Lengths> counts_;
    Model*                  model_ = nullptr;
    Engine*                 engine_for_seeding_= nullptr;
};  // end class Agents



