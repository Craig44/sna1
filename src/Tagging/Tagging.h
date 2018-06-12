#pragma once

#include "Requirements.h"

/**
 * Simulation of a tagging programme
 *
 * Creates a "database" of tag release and recapture pairs which can be
 * analysed by various methods.
 */
class Tagging {
public:

    /**
     * The minimum size of release
     */
    double release_length_min = 25;

    /**
     * Size selective releases?
     *
     * Used for simplicity in some tests
     */
    bool release_length_selective = true;

    /**
     * Population (above `release_length_min`) numbers by year and region
     */
    Array<int, Years, Regions> population_numbers;

    /**
     * The number of actual releases by year, region, and method 
     * to compare to `release_targets`
     */
    Array<int, Years, Regions, Methods> released;

    /**
     * The number of fish scanned by year, region, method and length
     */
    //Array<int, Years, Regions, Methods, Lengths> scanned;

    /**
     * The current tag number
     *
     * Is incremented in the `release()` method and applied
     * to each fish.
     */
    unsigned int number = 0;

    /**
     * A tagging event
     *
     * Copies the fish and adds a time stamp
     */
/*    class Event : public Agent {
     public:
        Time time;
        Method method;

        *
         * Constructor used for both release and
         * recapture events

        Event(const Agent& agent, Time time, Method method):
          Agent(agent),
          time(time),
          method(method){
        }

        *
         * Default constructer used for the recapture
         * event when an entry is made in database. 
         * Signified as empty by `time==0`

        Event(void):
            time(0){
        }
    };*/

    /**
     * A database of tagged fish
     *
     * A fish instance is stored at the time of release and recovery
     */
    //map<int, pair<Event, Event> > tags_;


    void initialise(void) {
        population_numbers = 0;
        released = 0;
        //scanned = 0;
    }

    void finalise(void) {
        write();
    }

    /**
     * Monitor the fish population
     *
     * In reality, we can never sample the true underlying population of fish. 
     * This method just allows us to capture some true population statistics for things
     * like examining the precision and bias of our estimates.
     * 
     * @param fish   A fish
     */
    void population(const Agent& agent) {
/*        auto y = year(now);
        // Add fish to numbers by Year and Region
        if (agent.length() >= release_length_min)
          population_numbers(y, agent.get_region())++;*/
    }

    /**
     * A mark and release of a fish.
     */
    void release(Agent& agent, Method method) {
/*        // Increment the tag number
        number++;
        // Apply the tag to the fish
        agent.set_tag(number);
        // Record the fish in the database
        tags_[number].first = Event(agent, now, method);
        // Add to released
        released(year(now), agent.get_region(), method)++;*/
    }

    void scan(const Agent& agent, Method method) {
/*        //scanned(year(now), fish.region, method, fish.length_bin())++;
        if (agent.get_tag() and chance() < parameters.tagging_detection)
          recover(agent, method);*/
    }

    /**
     * A recovery of a tagged fish.
     *
     * Note that this method does not actually kill the 
     * fish (done elsewhere) it just records it
     */
    void recover(const Agent& agent, Method method) {
        // Record the fish in the database
      //tags_[agent.get_tag()].second = Event(agent, now, method);
    }

    void read(void) {
    }

    void write(std::string directory = "output/monitor/tagging") {
/*
        boost::filesystem::create_directories(directory);
        
        population_numbers.write(directory + "/population_numbers.tsv");
        released.write(directory + "/released.tsv");
        //scanned.write(directory + "/scanned.tsv");

        std::ofstream releases_file(directory + "/releases.tsv");
        releases_file << "tag\ttime_rel\tregion_rel\tmethod_rel\tlength_rel\n";
        for(const auto& iter : tags_){
            auto number = iter.first;
            auto release = iter.second.first;
            releases_file<< number << "\t"
                << release.time << "\t"
                << region_code(release.get_region()) << "\t"
                << method_code(release.get_method()) << "\t"
                << release.get_length() << "\n";
        }

        std::ofstream recaptures_file(directory + "/recaptures.tsv");
        recaptures_file << "tag\ttime_rel\ttime_rec\tregion_rel\tregion_rec\tmethod_rel\tmethod_rec\tlength_rel\tlength_rec\n";
        for(const auto& iter : tags_){
            auto number = iter.first;
            auto release = iter.second.first;
            auto recapture = iter.second.second;
            if(recapture.time){
                recaptures_file<< number << "\t"
                    << release.time << "\t" << recapture.time << "\t"
                    << region_code(release.region) << "\t" << region_code(recapture.region) << "\t"
                    << method_code(release.method) << "\t" << method_code(recapture.method) << "\t"
                    << release.length << "\t" << recapture.length << "\n";
            }
        }

    }
*/

};  // class Tagging
