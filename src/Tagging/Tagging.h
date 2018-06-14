#pragma once

#include "Requirements.h"
#include "Event.h"
/**
 * Simulation of a tagging programme
 *
 * Creates a "database" of tag release and recapture pairs which can be
 * analysed by various methods.
 */
class Event;

class Tagging {
public:

    Tagging() {};
    ~Tagging() {};
    // Methods
    void initialise(void);
    void finalise(void);
    void population(const Agent& agent);
    void release(Agent& agent, Method method);
    void scan(const Agent& agent, Method method);
    void recover(const Agent& agent, Method method);
    void read(void);
    void write(std::string directory = "output/monitor/tagging");

    // Members
    double release_length_min_ = 25; // The minimum size of release
    bool release_length_selective_ = true; // Size selective releases?  Used for simplicity in some tests
    Array<int, Years, Regions> population_numbers_; // Population (above `release_length_min`) numbers by year and region
    Array<int, Years, Regions, Methods> released_; // The number of actual releases by year, region, and method to compare to `release_targets`
    //Array<int, Years, Regions, Methods, Lengths> scanned_; //The number of fish scanned by year, region, method and length
    unsigned int number_ = 0;  //The current tag number Is incremented in the `release()` method and applied to each agent.
    map<int, pair<Event, Event> > tags_;  //A database of tagged fish A fish instance is stored at the time of release and recovery

};  // class Tagging
