#pragma once

#include "Tagging.h"

void Tagging::initialise(void) {
    population_numbers_ = 0;
    released_ = 0;
    //scanned_ = 0;
}

void Tagging::finalise(void) {
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
void Tagging::population(const Agent& agent) {
    auto y = year(now);
    // Add fish to numbers by Year and Region
    if (agent.length_ >= release_length_min_)
      population_numbers_(y, agent.get_region())++;
}

/**
 * A mark and release of a fish.
 */
void Tagging::release(Agent& agent, Method method) {
    // Increment the tag number
    number_++;
    // Apply the tag to the fish
    agent.set_tag(number_);
    // Record the fish in the database
    tags_[number_].first = Event(agent, now, method);
    // Add to released
    released_(year(now), agent.get_region(), method)++;
}

void Tagging::scan(const Agent& agent, Method method) {
    //scanned(year(now), fish.region, method, fish.length_bin())++;
    if (agent.get_tag() and chance() < parameters.tagging_detection)
      recover(agent, method);
}

/**
 * A recovery of a tagged fish.
 *
 * Note that this method does not actually kill the 
 * fish (done elsewhere) it just records it
 */
void Tagging::recover(const Agent& agent, Method method) {
    // Record the fish in the database
    tags_[agent.get_tag()].second = Event(agent, now, method);
}

void Tagging::read(void) {
}

void Tagging::write(std::string directory) {
    boost::filesystem::create_directories(directory);
    
    population_numbers_.write(directory + "/population_numbers.tsv");
    released_.write(directory + "/released.tsv");
    //scanned.write(directory + "/scanned.tsv");

    std::ofstream releases_file(directory + "/releases.tsv");
    releases_file << "tag\ttime_rel\tregion_rel\tmethod_rel\tlength_rel\n";
    for(const auto& iter : tags_){
        auto number = iter.first;
        auto release = iter.second.first;
        releases_file<< number_ << "\t"
            << release.time_ << "\t"
            << region_code(release.region_) << "\t"
            << method_code(release.method_) << "\t"
            << release.length_ << "\n";
    }

    std::ofstream recaptures_file(directory + "/recaptures.tsv");
    recaptures_file << "tag\ttime_rel\ttime_rec\tregion_rel\tregion_rec\tmethod_rel\tmethod_rec\tlength_rel\tlength_rec\n";
    for(const auto& iter : tags_){
        auto number = iter.first;
        auto release = iter.second.first;
        auto recapture = iter.second.second;
        if(recapture.time_){
            recaptures_file<< number << "\t"
                << release.time_ << "\t" << recapture.time_ << "\t"
                << region_code(release.region_) << "\t" << region_code(recapture.region_) << "\t"
                << method_code(release.method_) << "\t" << method_code(recapture.method_) << "\t"
                << release.length_ << "\t" << recapture.length_ << "\n";
        }
    }
}
