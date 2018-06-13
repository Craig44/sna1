#pragma once

#include "Agents.h"
#include "Model.h"
/**
 * The population of `Fish`
 *
 * We don't attempt to model every single fish in the population. Instead,
 * the vector of `Fish` objects is intended to be a representative sample of the overall population.
 * The variable, `scalar` is then used to scale other variables, like biomass, to population levels.
 */


/**
 * Initialise parameters etc
 */
void Agents::initialise(void) {
  // Create a pointer to the environment class so we can get preference values by space and time.
}


void Agents::finalise(void) {
  cerr << "enter finalise Agents" << endl;
  boost::filesystem::create_directories("output/fishes");

  // print end point of fish that are alive
  std::ofstream agent_attributes("output/fishes/attributes.tsv");
  agent_attributes << "latitude longitude alive region length age\n";
  for (auto& agent : *this) {
    agent_attributes << agent.latitude_ << " " << agent.longitude_ << " " << agent.alive() << " " << agent.region_ << " " << agent.length_ << " " << agent.age() << endl;
  }


  std::ofstream values("output/fishes/values.tsv");
  values << "name\tvalue" << std::endl
         << "fishes_size\t" << size() << std::endl
         << "fish_bytes\t" << sizeof(Agent) << std::endl
         << "alive\t" << number(false) << std::endl
         << "scalar\t" << scalar_ << std::endl
         << "number\t" << number(true) << std::endl;

  // Generate some example growth trajectories for checking
  std::ofstream pars("output/fishes/growth_pars.tsv");
  pars << "fish\tintercept\tslope\tk\tL_inf\n";
  std::ofstream trajs("output/fishes/growth_trajs.tsv");
  trajs << "fish\ttime\tlength\tlength_new\n";
  for (int index = 0; index < 100; index++) {
      Agent agent;
      agent.born(HG);
      pars << index << "\t"
           << agent.growth_intercept_ << "\t"
           << agent.growth_slope_ << "\t"
           << -log(agent.growth_slope_ + 1) << "\t"
           << agent.growth_intercept_ / -agent.growth_slope_ << "\n";

      for (int time = 0; time < 50; time++) {
          trajs << index << "\t"
               << time << "\t"
               << agent.length_ << "\t";
          agent.growth();
          trajs << agent.length_ << "\n";
      }
  }
  cerr << "exit agents fishes" << endl;
}


/**
 * Seed the population with individuals that have attribute values
 * whose distributions approximate that of a pristine population
 *
 * This method is usually used in `Model::pristine` to reduce burn-in times
 * but is a separate method so that it can also be used in unit tests.
 */
void Agents::seed(unsigned int number) {
    clear();
    resize(number);
    for (auto& agent : *this) {
      agent.seed(model_->get_environment_ptr());
    }
}


void Agents::biomass_update(void) {
    biomass_ = 0.0;
    for (auto& agent : *this){
        if (agent.alive()) {
            biomass_ += agent.weight();
        }
    }
    biomass_ *= scalar_;
}

void Agents::biomass_spawners_update(void) {
    biomass_spawners_ = 0.0;
    for (auto& agent : *this){
        if (agent.alive() and agent.mature_) {
            biomass_spawners_(agent.region_) += agent.weight();
        }
    }
    biomass_spawners_ *= scalar_;
}



void Agents::recruitment_update(void) {
    auto y = year(now);
    for(auto region : regions) {
        if (recruitment_mode_ == 'p') {
            recruitment_(region) = recruitment_pristine_(region) / scalar_;
        } else {
            auto s = biomass_spawners_(region);
            auto r0 = recruitment_pristine_(region) / scalar_;
            auto s0 = parameters.fishes_b0(region);
            auto h = parameters.fishes_steepness;
            auto determ = 4*h*r0*s/((5*h-1)*s+s0*(1-h));

            double strength = parameters.fishes_rec_strengths(y, region);
            if(parameters.debug) {
               cerr << "YCS in year " << now << ": " << strength << endl;
            }
            if (strength < 0) {
                strength = Lognormal(1, parameters.fishes_rec_var).random();
            }

            recruitment_(region) = determ * strength;

        }
        recruitment_instances_(region) = round(recruitment_(region));
    }
}


/**
 * Calculate the number of fish in the population
 *
 * @param scale Scale up the number?
 */
double Agents::number(bool scale) {
    auto sum = 0.0;
    for (auto& agent : *this){
        if (agent.alive()) {
            sum++;
        }
    }
    return sum * (scale ? scalar_ : 1);
}

/**
 * Calculate the mean age of fish
 */
double Agents::age_mean(void) {
    Mean mean;
    for (auto& agent : *this){
        if (agent.alive())
            mean.append(agent.age_bin());
    }
    return mean;
}

/**
 * Calculate the mean length of fish
 */
double Agents::length_mean(void) {
    Mean mean;
    for (auto& agent : *this) {
        if (agent.alive())
           mean.append(agent.length_);
    }
    return mean;
}

/**
 * Enumerate the population (count number of fish etc)
 */
void Agents::enumerate(void) {
    counts_ = 0;
    for (auto& agent : *this) {
        if(agent.alive()){
            counts_(
                agent.region_,
                agent.sex_,
                agent.age_bin(),
                agent.length_bin()
            )++;
        }
    }
}

/**
 * Track the population by writing attributes and structure to files
 */
void Agents::track(void) {
    static std::ofstream* counts_file = nullptr;
    if(not counts_file) counts_file = new std::ofstream("output/fishes/counts.tsv");

    enumerate();

    for(auto region : regions){
        for(auto sex : sexes){
            for(auto age: ages){
                for(auto length : lengths){
                    (*counts_file)
                        <<now<<"\t"
                        <<region<<"\t"
                        <<sex<<"\t"
                        <<age<<"\t"
                        <<length<<"\t"
                        <<counts_(region,sex,age,length)<<"\n"
                    ;
                }
            }
        }
    }
    (*counts_file).flush();
}
