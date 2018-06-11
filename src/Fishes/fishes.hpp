#pragma once

#include "requirements.hpp"
#include "dimensions.hpp"
#include "parameters.hpp"
#include "Environment/Environment.hpp"
#include "random.hpp"


class Environ; // fwd declare
Normal individual_normal_generator;

/**
 * A fish
 */
class Fish {
 public:
    Fish();
    Fish(Engine& engine, Environ* this_environ) :
      engine_(engine),
      the_environ(this_environ)
    { }

    Engine& engine_;
    Environ* the_environ;

    /**
     * Home region for this fish
     */
    Region home;

    /**
     * A pointer to the environment
     */
    /**
     * Time of birth of this fish
     */
    Time birth;

    /**
     * Time of death of this fish
     */
    Time death;

    /**
     * Sex of this fish
     */
    Sex sex;

    /**
     * latitude of this fish
     */
    double latitude;

    /**
     * lonigtude of this fish
     */
    double longitude;



    map<unsigned, double> lat_memory;
    map<unsigned, double> lon_memory;

    /**
     * Intercept of the length increment to length relaion
     */
    float growth_intercept;

    /**
     * Slope of the length increment to length relaion
     */
    float growth_slope;

    /**
     * Current length (cm) of this fish
     */
    float length;

    /**
     * Current age of this fish
     */
    //float age;

    float age(void) const {
        return year(now) - year(birth);
    }


    /**
     * Is this fish mature?
     */
    bool mature;

    /**
     * Current region of this fish
     */
    Region region;

    /**
     * Tag number for fish
     */
    unsigned int tag;

    /**
     * The method that this fish was last caught by (and released,
     * because undersized or tag-release)
     */
    short method_last;


    /*************************************************************
     * Attributes
     ************************************************************/

    /**
     * Is this fish alive?
     */
    bool alive(void) const {
        return death == 0;
    }



    /**
     * Get the age bin of this fish
     */
    int age_bin(void) const {
        return ::age_bin(age());
    }

    /**
     * Get the length bin of this fish
     */
    int length_bin(void) const {
        return ::length_bin(length);
    }

    /**
     * Birthday
     */
    //void birthday(void) {
    //    age++;
    //}
    /**
     * Get the weight of this fish
     *
     * Currently, all fish have the same condition factor so weight is
     * simply a function of length
     */
    double weight(void) const {
        return parameters.fishes_a*std::pow(length, parameters.fishes_b);
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
    void seed() {
      cout << "seed a fish" << endl;


        home = Region(int(parameters.fishes_seed_region_dist.random()));
        region = home;

        latitude = -41.234;
        longitude = 187.232;

        auto seed_age = std::max(1.,std::min(parameters.fishes_seed_age_dist.random(),100.));
        birth = now-seed_age;
        death = 0;

        sex = (chance()<parameters.fishes_males)?male:female;

        growth_init(seed_age);

        // This an approximation
        mature = chance()<parameters.fishes_maturation(seed_age);

        tag = 0;

        method_last = -1;

        //age = seed_age;
    }

    /**
     * Birth this fish
     *
     * Initialises attributes as though this fish is close
     * to age 0
     */
    void born(Region region_) {
        cout << "initialise a fish" << endl;

        latitude = -41.234;
        longitude = 187.232;

        home = region_;
        region = home;

        birth = now;
        death = 0;
        
        sex = (chance()<parameters.fishes_males)?male:female;

        growth_init(0);

        mature = false;

        tag = 0;

        method_last = -1;

        //age = 0;
    }

    /**
     * Initialises growth parameters and length for this fish
     *
     * Note that event if this is an exponential growth model that
     * we are parameterize if using `k` and `linf`
     */
    void growth_init(int age) {
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
        growth_slope = std::exp(-k)-1;
        growth_intercept = -growth_slope * linf;
        // Calculate expected length at age assuming von Bert parameters
        // Note that this is an aproximation only. It does not allow for 
        // temporal variation in growth or for the exponential growth model.
        length = linf*(1-std::exp(-k*age));
    }

    /**
     * Kill this fish
     *
     * This method is separate from `survive()` because it
     * is also used by `Fleets` to kill a fish from harvest or
     * incidental mortality
     */
    void dies(void) {
        death = now;
    }

    /**
     * Does this fish survive half natural mortality this time step?
     */
    bool survival(void) {
        auto survives = chance() > parameters.fishes_m_rate;
        if (not survives) dies();
        return survives;
    }

    /**
     * Does this fish survive this time step?
     */
    bool half_survival(void) {
        auto survives = chance() > parameters.fishes_half_m_rate;
        if (not survives) dies();
        return survives;
    }

    /**
     * Increase the length of this fish
     */
    void growth(void) {
        //age++;
        // Calculate growth increment
        double incr;
        if (parameters.fishes_growth_model == 'l') {
            // Linear increment v length
            incr = growth_intercept + growth_slope * length;
        } else if (parameters.fishes_growth_model == 'e') {
            // Exponential increment v length
            const double length_alpha = 25;
            const double length_beta = 50;
            double growth_alpha = growth_intercept + growth_slope * length_alpha;
            double growth_beta = growth_intercept + growth_slope * length_beta;
            double lamda = 1/(length_beta-length_alpha)*std::log(growth_alpha/growth_beta);
            double kappa = std::pow(growth_alpha*(growth_alpha/growth_beta), length_alpha/(length_beta-length_alpha));
            incr = 1/lamda*log(1+ (lamda*kappa*exp(-lamda*length)));
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
        length += incr;
        if (length < 0)
            length = 0;
    }

    /**
     * Change the maturation status of this fish
     */
    void maturation(void) {
        if (not mature) {
            if (chance()<parameters.fishes_maturation(age_bin())) {
                mature = true;
            }
        }
    }



    /**
     * Move this fish
     */
    void preference_movement(void) {
      if (now == 1899) {
        lat_memory[now] = latitude;
        lon_memory[now] = longitude;
      }

      if (now >= 1900) {
        //cout << "about to use preference movement to move fish lat = " << latitude << " longitude " << lon << " year " << year(now) << " or year = "<< now << endl;
        // pull gradients zonal and meridinal
        vector<double> gradient = the_environ->get_gradient(latitude, longitude, year(now));
        individual_normal_generator = {gradient[0], parameters.standard_dev_for_preference};

        double zonal_jump = individual_normal_generator.random();

        longitude += zonal_jump;
        if (longitude > parameters.max_lon)
          longitude -= zonal_jump;
        else if (longitude < parameters.min_lon)
          longitude -= zonal_jump;
        lon_memory[year(now)] = longitude;
        individual_normal_generator = {gradient[1], parameters.standard_dev_for_preference};

        double meridional_jump = individual_normal_generator.random();
        //cout << "jump = " << meridional_jump << endl;
        latitude += meridional_jump;
        if (latitude > parameters.max_lat)
          latitude -= meridional_jump;
        else if (latitude < parameters.min_lat)
          latitude -= meridional_jump;
        lat_memory[year(now)] = latitude;
        cout << setprecision(10)  << "calculating preference movement, lat = " << latitude - meridional_jump << " long = " << longitude - zonal_jump << " gradient long direction = " << gradient[0] << " long jump = " << zonal_jump << " gradient lat direction = " << gradient[1] << " lat jump = " << meridional_jump << endl;

      }
    }
    /**
     * Move this fish
     */
    void movement(void) {
        // If no movement, don't do anything
        if (parameters.fishes_movement_type == 'n')
          return;
        // Instantaneous movement between regions is eith Markovian (based on where fish is)
        // or home fidelity (based on fish's home)
        Region basis = region;
        switch (parameters.fishes_movement_type) {
            case 'm':
                basis = region;
            break;
            case 'h':
                basis = home;
            break; 
        };
        // Randomly move a region (note that rows of the movement matrix sum to 1)
        auto region_to = Region(regions.select(chance()).index());
        if (chance() < parameters.fishes_movement(basis, region_to)) {
            region = region_to;
        }
    }

    /**
     * Fish was released by a method
     */
    void released(Method method) {
        method_last = method;
    }

    /**
     * Does this fish shed it's tag (if any)?
     */
    void shedding(void) {
        if (tag) {
            if (chance() < parameters.tagging_shedding) {
                tag = 0;
            }
        }
    }

};  // end class Fish


/**
 * The population of `Fish`
 * 
 * We don't attempt to model every single fish in the population. Instead,
 * the vector of `Fish` objects is intended to be a representative sample of the overall population.
 * The variable, `scalar` is then used to scale other variables, like biomass, to population levels.
 */


class Fishes : public vector<Fish> {
 public:

  vector<Fish> partition_;
    // constructor that sets intial vector of fishes = size.
    Fishes(int size = 0, Engine& engine, Environ* this_environ) :
      partition_(size, Fish(engine, this_environ))
      {}

    /**
     * Population scalar
     *
     * Used to scale the things like biomass etc from the size of `fishes` to the 
     * total population size
     */
    double scalar = 1.0;





    /**
     * Seed the population with individuals that have attribute values 
     * whose distributions approximate that of a pristine population
     * 
     * This method is usually used in `Model::pristine` to reduce burn-in times
     * but is a separate method so that it can also be used in unit tests. 
     */
    void seed(unsigned int number) {
        clear();
        resize(number);
        for (auto& fish : *this) {
            fish.seed();
        }
    }

    /**
     * Aggregate properties that get calculated at various times
     */
    
    /**
     * Current total biomass (t)
     */
    double biomass;

    void biomass_update(void) {
        biomass = 0.0;
        for (auto& fish : *this) {
            if (fish.alive()) {
                biomass += fish.weight();
            }
        }
        biomass *= scalar;
    }

    /**
     * Current spawner biomass (t)
     */
    Array<double, Regions> biomass_spawners;

    void biomass_spawners_update(void) {
        biomass_spawners = 0.0;
        for (auto& fish : *this) {
            if (fish.alive() and fish.mature) {
                biomass_spawners(fish.region) += fish.weight();
            }
        }
        biomass_spawners *= scalar;
    }




    char recruitment_mode = 'n';

    /**
     * Recruitment for pristine population (see `Model::pristine()`)
     */
    Array<double, Regions> recruitment_pristine;

    /**
     * Current recruitment (no.)
     */
    Array<double, Regions> recruitment;

    /**
     * Current recruitment (instances)
     */
    Array<unsigned int, Regions> recruitment_instances;


    void recruitment_update(void) {
        auto y = year(now);
        for(auto region : regions) {
            if (recruitment_mode == 'p') {
                recruitment(region) = recruitment_pristine(region)/scalar;
            } else {
                auto s = biomass_spawners(region);
                auto r0 = recruitment_pristine(region)/scalar;
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

                recruitment(region) = determ * strength;

            }
            recruitment_instances(region) = std::round(recruitment(region));
        }
    }


    /**
     * Counts of fish by model dimensions
     */
    Array<unsigned int,Regions,Sexes,Ages,Lengths> counts;


    /**
     * Initialise parameters etc
     */
    void initialise(void){
      // Create a pointer to the environment class so we can get preference values by space and time.


    }

    /**
     * Finalise (e.g. write values to file)
     */
    void finalise(void){
        cerr << "enter finalise fishes" << endl;
        boost::filesystem::create_directories("output/fishes");

        std::ofstream movement_file("output/fishes/movement.tsv");

        unsigned fish_number = 1;
        for (auto& fish : *this) {
          movement_file << "fish_" << fish_number << "\t";
          for (auto lat_mem : fish.lat_memory) {
            if (lat_mem.second < 0)
              movement_file << lat_mem.second << "\t";
          }
          movement_file << "\n";
          movement_file << "fish_" << fish_number << "\t";
          for (auto lon_mem : fish.lon_memory) {
            if (lon_mem.second > 0)
              movement_file << lon_mem.second << "\t";
          }

          movement_file << "\n";
          ++fish_number;
        }
        std::ofstream values("output/fishes/values.tsv");
        values << "name\tvalue" << std::endl
               << "fishes_size\t" << size() << std::endl
               << "fish_bytes\t" << sizeof(Fish) << std::endl
               << "alive\t" << number(false) << std::endl
               << "scalar\t" << scalar << std::endl
               << "number\t" << number(true) << std::endl;

        // Generate some example growth trajectories for checking
        std::ofstream pars("output/fishes/growth_pars.tsv");
        pars << "fish\tintercept\tslope\tk\tL_inf\n";
        std::ofstream trajs("output/fishes/growth_trajs.tsv");
        trajs << "fish\ttime\tlength\tlength_new\n";
        for (int index = 0; index < 100; index++) {
            Fish fish;
            fish.born(HG);
            pars << index << "\t"
                 << fish.growth_intercept << "\t" 
                 << fish.growth_slope << "\t"
                 << -log(fish.growth_slope + 1) << "\t"
                 << fish.growth_intercept / -fish.growth_slope << "\n";

            for (int time = 0; time < 50; time++) {
                trajs << index << "\t"
                     << time << "\t"
                     << fish.length << "\t";
                fish.growth();
                trajs << fish.length << "\n";
            }
        }
        cerr << "exit finalise fishes" << endl;

    }


    /**
     * Calculate the number of fish in the population
     *
     * @param scale Scale up the number?
     */
    double number(bool scale = true) {
        auto sum = 0.0;
        for (auto fish : *this){
            if (fish.alive()) {
                sum++;
            }
        }
        return sum * (scale?scalar:1);
    }

    /**
     * Calculate the mean age of fish
     */
    double age_mean(void) {
        Mean mean;
        for (auto fish : *this) {
            if (fish.alive())
                mean.append(fish.age_bin());
        }
        return mean;
    }

    /**
     * Calculate the mean length of fish
     */
    double length_mean(void) {
        Mean mean;
        for (auto fish : *this) {
            if (fish.alive())
               mean.append(fish.length);
        }
        return mean;
    }

    /**
     * Enumerate the population (count number of fish etc)
     */
    void enumerate(void) {
        counts = 0;
        for (auto fish : *this) {
            if(fish.alive()){
                counts(
                    fish.region,
                    fish.sex,
                    fish.age_bin(),
                    fish.length_bin()
                )++;
            }
        }
    }

    /**
     * Track the population by writing attributes and structure to files
     */
    void track(void){ 
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
                            <<counts(region,sex,age,length)<<"\n"
                        ;
                    }
                }
            }
        }
        (*counts_file).flush();
    }

};  // end class Fishes
