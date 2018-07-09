#pragma once

#include "Environment.h"

class Environemnt;
/**
 * An agents attributes, this class is also responsible for processes that control the fate of individual whether it dies, where it moves etc
 * Most attributes are public because the agents class actually inherits the vector class so to give it access to this class everything is public
 */
class Agent {
  public:
    Agent() {};
    ~Agent() {};
    /*************************************************************
     * Attributes
     ************************************************************/
    float age(void) const;
    bool alive(void) const;
    int age_bin(void) const;
    void set_birth(Time birth) {
      birth_ = birth;
    }
    Time get_birth(void) {
      return birth_;
    }
    int length_bin(void) const;
    int get_length(void) const {
      return length_;
    }
    double weight(void) const;
    Region get_region(void) const;
    unsigned int get_tag(void) const {
      return tag_;
    }
    ;
    void set_tag(unsigned int new_tag) {
      tag_ = new_tag;
    }
    short get_method_last(void) const {
      return method_last_;
    }

    void                    set_scalar(double scalar) {scalar_ = scalar;};
    double                  get_scalar(void) const {return scalar_;};
    /*************************************************************
     * Processes
     ************************************************************/
    void seed(); // overload for reporting fish attributes but not run time evaluations
    void seed(Environment* environment);
    void born(Region region); // overload for reporting fish attributes but not run time evaluations
    void born(Region region, Environment* environment);
    void growth_init(int age);
    void dies(void);
    bool survival(void);
    bool half_survival(void); // TODO Remove this
    void growth(void);
    void maturation(void);
    void preference_movement(void);
    void movement(void);
    void released(Method method);
    void shedding(void);

    Region home_;                        // Home region for this fish
    Time birth_;                       // Time of birth of this fish
    Time death_;                       // Time of death of this fish
    Sex sex_;                         // Sex of this fish
    float latitude_;                    // latitude of this fish
    float longitude_;                   // longitude of this fish
    float growth_intercept_; // Intercept of the length increment to length relaion
    float growth_slope_;      // Slope of the length increment to length relaion
    float length_;                      // Current length (cm) of this fish
    bool mature_;                      // Is this fish mature?
    Region region_;                      // Current region of this fish
    unsigned int tag_;                         // Tag number for fish
    short method_last_; // The method that this fish was last caught by (and released,because undersized or tag-release)
    double                  scalar_ = 1.0;    // Population scalar, Used to scale the things like biomass etc from the size of `fishes` to the

  protected:
    Environment* environemnt_ptr_ = nullptr;                // Give each fish a pointer to the environemnt for movement

};
// end class Agent


