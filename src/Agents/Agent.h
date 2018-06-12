#pragma once

#include "Requirements.h"
#include "Dimensions.h"
#include "Parameters.h"
#include "Model.h"
#include "Random.h"


Normal individual_normal_generator;
class Model;
/**
 * A fish
 */
class Agent {
 public:
    Agent() {};
    ~Agent() {};

    // Accessors
    /*************************************************************
     * Attributes
     ************************************************************/
    float           age(void) const;
    bool            alive(void) const;
    int             age_bin(void) const;
    void            set_birth(Time birth) {birth_ = birth;}
    Time            get_birth(void) {return birth_;}
    int             length_bin(void) const;
    int             get_length(void) const {return length_;}
    double          weight(void) const;
    Region          get_region(void) const;
    unsigned int    get_tag(void) const {return tag_;};
    void            set_tag(unsigned int new_tag)  {tag_ = new_tag;}
    short           get_method_last(void) const {return method_last_;}

    /*************************************************************
     * Processes
     ************************************************************/
    void            seed();
    void            born(Region region);
    void            growth_init(int age);
    void            dies(void);
    bool            survival(void);
    bool            half_survival(void); // TODO Remove this
    void            growth(void);
    void            maturation(void);
    void            preference_movement(void);
    void            movement(void);
    void            released(Method method);
    void            shedding(void);
 //protected:
    //Engine&         engine_;                      // boost engine for random number generation
    Model*          model_;                       // Model pointer TODO look if it is quicker just to have an environment pointer
    Region          home_;                        // Home region for this fish
    Time            birth_;                       // Time of birth of this fish
    Time            death_;                       // Time of death of this fish
    Sex             sex_;                         // Sex of this fish
    float           latitude_;                    // latitude of this fish
    float           longitude_;                   // longitude of this fish
    map<unsigned, float> lat_memory_;             // TODO removed when satisfied the preference movement is doing what it is suppose to.
    map<unsigned, float> lon_memory_;             // TODO removed when satisfied the preference movement is doing what it is suppose to.
    float           growth_intercept_;            // Intercept of the length increment to length relaion
    float           growth_slope_;                // Slope of the length increment to length relaion
    float           length_;                      // Current length (cm) of this fish
    bool            mature_;                      // Is this fish mature?
    Region          region_;                      // Current region of this fish
    unsigned int    tag_;                         // Tag number for fish
    short           method_last_;                 // The method that this fish was last caught by (and released,because undersized or tag-release)


};  // end class Agent


