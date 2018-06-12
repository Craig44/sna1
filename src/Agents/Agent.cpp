#pragma once

#include "Dimensions.h"
#include "Parameters.h"
#include "Random.h"

#include "Agent.h"
#include "Environment.cpp"

/*
 * Source code for the Agent class
*/
/*************************************************************
 * Attributes
 ************************************************************/
float Agent::age(void) const {
    return year(now) - year(birth_);
}

Region Agent::get_region(void) const {
  return region_;
}

/**
 * Is this fish alive?
 */
bool Agent::alive(void) const {
    return death_ == 0;
}

/**
 * Get the age bin of this fish
 */
int Agent::age_bin(void) const {
    return ::age_bin(age());
}

/**
 * Get the length bin of this fish
 */
int Agent::length_bin(void) const {
    return ::length_bin(length_);
}
/**
 * Get the weight of this fish
 *
 * Currently, all fish have the same condition factor so weight is
 * simply a function of length
 */
double Agent::weight(void) const {
    return parameters.fishes_a*std::pow(length_, parameters.fishes_b);
}

/*************************************************************
 * Processes
 ************************************************************/

/**
 * Create a seed fish
 *
 * Needed for intial seeding of the population prior to burning it in. In other circumstances
 * `birth()` should used be used. Currently, a number of approximations are used so that the
 * seed population is closer to an equilibrium population:
 *
 *  - exponential distribution of ages
 *  - seed fish are distributed evenly across areas
 *  - maturity is approximated by maturation schedule
 */
void Agent::seed(Environment* environment, Engine* engine) {
    environemnt_ptr_ = environment;
    engine_ptr_ = engine;
    home_ = Region(int(parameters.fishes_seed_region_dist.random()));
    region_ = home_;

    latitude_ = -41.234;
    longitude_ = 187.232;

    auto seed_age = std::max(1.,std::min(parameters.fishes_seed_age_dist.random(),100.));
    birth_ = now-seed_age;
    death_ = 0;

    sex_ = (chance()<parameters.fishes_males) ? male : female;

    growth_init(seed_age);

    // This an approximation
    mature_ = chance()<parameters.fishes_maturation(seed_age);

    tag_ = 0;

    method_last_ = -1;

    //age = seed_age;
}


void Agent::seed() {
    home_ = Region(int(parameters.fishes_seed_region_dist.random()));
    region_ = home_;

    latitude_ = -41.234;
    longitude_ = 187.232;

    auto seed_age = std::max(1.,std::min(parameters.fishes_seed_age_dist.random(),100.));
    birth_ = now-seed_age;
    death_ = 0;

    sex_ = (chance()<parameters.fishes_males) ? male : female;

    growth_init(seed_age);

    // This an approximation
    mature_ = chance()<parameters.fishes_maturation(seed_age);

    tag_ = 0;

    method_last_ = -1;

    //age = seed_age;
}

/**
 * Birth this fish
 *
 * Initialises attributes as though this fish is close
 * to age 0
 */
void Agent::born(Region region, Environment* environment, Engine* engine) {
    environemnt_ptr_ = environment;
    engine_ptr_ = engine;

    latitude_ = -41.234;
    longitude_ = 187.232;

    home_ = region;
    region_ = home_;

    birth_ = now;
    death_ = 0;

    sex_ = (chance()<parameters.fishes_males) ? male : female;

    growth_init(0);

    mature_ = false;

    tag_ = 0;

    method_last_ = -1;

    //age = 0;
}


void Agent::born(Region region) {

    latitude_ = -41.234;
    longitude_ = 187.232;

    home_ = region;
    region_ = home_;

    birth_ = now;
    death_ = 0;

    sex_ = (chance()<parameters.fishes_males) ? male : female;

    growth_init(0);

    mature_ = false;

    tag_ = 0;

    method_last_ = -1;

    //age = 0;
}
/**
 * Initialises growth parameters and length for this fish
 *
 * Note that event if this is an exponential growth model that
 * we are parameterize if using `k` and `linf`
 */
void Agent::growth_init(int age) {
    // Get von Bert growth parameters from their distributions
    double k;
    double linf;
    if (parameters.fishes_growth_variation == 't') {
        // All individual fish have the same growth parameters
        // but there is temporal variation in increments
        k = parameters.fishes_k_mean;
        linf = parameters.fishes_linf_mean;
    } else {
        // Each individual fish gets it's own growth parameters
        k = parameters.fishes_k_dist.random();
        linf = parameters.fishes_linf_dist.random();
    }
    // Convert `k` and `linf` to `growth_intercept` and `growth_slope`
    growth_slope_ = std::exp(-k)-1;
    growth_intercept_ = -growth_slope_ * linf;
    // Calculate expected length at age assuming von Bert parameters
    // Note that this is an aproximation only. It does not allow for
    // temporal variation in growth or for the exponential growth model.
    length_ = linf*(1-std::exp(-k*age));
}

/**
 * Kill this fish
 *
 * This method is separate from `survive()` because it
 * is also used by `Fleets` to kill a fish from harvest or
 * incidental mortality
 */
void Agent::dies(void) {
    death_ = now;
}

/**
 * Does this fish survive half natural mortality this time step?
 */
bool Agent::survival(void) {
    auto survives = chance() > parameters.fishes_m_rate;
    if (not survives)
      dies();
    return survives;
}

/**
 * Does this fish survive this time step?
 */
bool Agent::half_survival(void) {
    auto survives = chance() > parameters.fishes_half_m_rate;
    if (not survives)
      dies();
    return survives;
}

/**
 * Increase the length of this fish
 */
void Agent::growth(void) {
    //age++;
    // Calculate growth increment
    double incr;
    if (parameters.fishes_growth_model == 'l') {
        // Linear increment v length
        incr = growth_intercept_ + growth_slope_ * length_;
    } else if (parameters.fishes_growth_model == 'e') {
        // Exponential increment v length
        const double length_alpha = 25;
        const double length_beta = 50;
        double growth_alpha = growth_intercept_ + growth_slope_ * length_alpha;
        double growth_beta = growth_intercept_ + growth_slope_ * length_beta;
        double lamda = 1/(length_beta-length_alpha)*std::log(growth_alpha/growth_beta);
        double kappa = std::pow(growth_alpha*(growth_alpha/growth_beta), length_alpha/(length_beta-length_alpha));
        incr = 1/lamda*log(1+ (lamda*kappa*exp(-lamda*length_)));
    } else {
        // Move this to initialise() TODO
        throw std::runtime_error("Unknown growth model: " + parameters.fishes_growth_model);
    }
    // Apply temporal variation in growth if needed
    if (parameters.fishes_growth_variation == 't' or parameters.fishes_growth_variation == 'm') {
        int sd = std::max(parameters.fishes_growth_temporal_sdmin, incr * parameters.fishes_growth_temporal_cv);
        incr += standard_normal_rand() * sd;
        if (incr < parameters.fishes_growth_temporal_incrmin)
            incr = parameters.fishes_growth_temporal_incrmin;
    }
    // Add increment but ensure fish size does not go below zero
    length_ += incr;
    if (length_ < 0)
        length_ = 0;
}

/**
 * Change the maturation status of this fish
 */
void Agent::maturation(void) {
    if (not mature_) {
        if (chance()<parameters.fishes_maturation(age_bin())) {
            mature_ = true;
        }
    }
}



/**
 * apply the longiutidinal jump
 */
void Agent::zonal_jump(void) {
    //cout << "about to use preference movement to move fish lat = " << latitude << " longitude " << lon << " year " << year(now) << " or year = "<< now << endl;
    // pull gradients zonal and meridinal
    double velocity = environemnt_ptr_->get_gradient(latitude_, longitude_, year(now))[0];
    boost::normal_distribution<> dist {velocity, parameters.standard_dev_for_preference};

    double zonal_jump = dist(*engine_ptr_);

    longitude_ += zonal_jump;
    if (longitude_ > parameters.max_lon)
      longitude_ -= zonal_jump;
    else if (longitude_ < parameters.min_lon)
      longitude_ -= zonal_jump;
    lon_memory_[year(now)] = longitude_;
    //cout << setprecision(10)  << "calculating preference movement,  long = " << longitude_ - zonal_jump << " gradient long direction = " << velocity << " long jump = " << zonal_jump << endl;

}


/*
 * Apply lat jump
*/
void Agent::meridional_jump(void) {
  double velocity = environemnt_ptr_->get_gradient(latitude_, longitude_, year(now))[1];
  boost::normal_distribution<> dist {velocity, parameters.standard_dev_for_preference};
  double meridional_jump = dist(*engine_ptr_);
  //cout << "jump = " << meridional_jump << endl;
  latitude_ += meridional_jump;
  if (latitude_ > parameters.max_lat)
    latitude_ -= meridional_jump;
  else if (latitude_ < parameters.min_lat)
    latitude_ -= meridional_jump;
  lat_memory_[year(now)] = latitude_;
  //cout << "calculating preference movement, lat = " << latitude_ - meridional_jump << " gradient lat direction = " << velocity << " lat jump = " << meridional_jump << endl;
}

/*
 * Apply the actual movement
*/
void Agent::preference_movement(void) {
  // Only apply preference movement in first year TODO think about equilibrium spatial distributions
  if (now >= 1900) {
    meridional_jump();
    zonal_jump();
  }
}
/**
 * Move this fish
 */
void Agent::movement(void) {
    // If no movement, don't do anything
    if (parameters.fishes_movement_type == 'n')
      return;
    // Instantaneous movement between regions is eith Markovian (based on where fish is)
    // or home fidelity (based on fish's home)
    Region basis = region_;
    switch (parameters.fishes_movement_type) {
        case 'm':
            basis = region_;
        break;
        case 'h':
            basis = home_;
        break;
    };
    // Randomly move a region (note that rows of the movement matrix sum to 1)
    auto region_to = Region(regions.select(chance()).index());
    if (chance() < parameters.fishes_movement(basis, region_to)) {
        region_ = region_to;
    }
}

/**
 * Fish was released by a method
 */
void Agent::released(Method method) {
    method_last_ = method;
}

/**
 * Does this fish shed it's tag (if any)?
 */
void Agent::shedding(void) {
    if (tag_) {
        if (chance() < parameters.tagging_shedding) {
            tag_ = 0;
        }
    }
}
