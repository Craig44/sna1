#pragma once

#include "requirements.hpp"

/**
 * Fishing activities
 */
class Harvest {
 public:

    /**
     * Selectivity by method for each length bin
     */
    Array<double, Methods, Lengths> selectivity_at_length;

    /**
     * Selectivity by method for each age bin
     */
    Array<double, Methods, Ages> selectivity_at_age;

    /**
     * Current vulnerable biomass by method
     */
    Array<double, Regions, Methods> biomass_vulnerable;

    Array<double, Regions, Methods> catch_observed;

    Array<double, Regions, Methods> catch_taken;

    // A container just for reporting purposes
    Array<double, Regions, Methods, Years> catch_taken_by_year;


    uint attempts;



    void initialise(void){
        for (auto method : methods) {
        	if (parameters.length_based_selectivity) {
				for (auto length_bin : lengths) {
					auto steep1 = parameters.harvest_sel_steep1(method);
					auto mode = parameters.harvest_sel_mode(method);
					auto steep2 = parameters.harvest_sel_steep2(method);
					auto length = length_mid(length_bin);
					double selectivity;
					if(length<=mode)
						selectivity = std::pow(2,-std::pow((length-mode)/steep1,2));
					else
						selectivity = std::pow(2,-std::pow((length-mode)/steep2,2));
					selectivity_at_length(method,length_bin) = selectivity;
				}
        	} else {
				for (auto age : ages) {

                    double p = 0;
                    if (age <= 4) p = 0;
                    else if (age == 5)
                        p = 0.5;
                    else
                        p = 1;
                    selectivity_at_age(method,age) = p;


/*

					auto steep1 = parameters.harvest_sel_steep1(method);
					auto mode = parameters.harvest_sel_mode(method);
					auto steep2 = parameters.harvest_sel_steep2(method);
					double selectivity;
					if(age<=mode)
						selectivity = std::pow(2,-std::pow((age.index()-mode)/steep1,2));
					else
						selectivity = std::pow(2,-std::pow((age.index()-mode)/steep2,2));
					selectivity_at_age(method,age.index()) = selectivity;
*/

					if (parameters.debug) {
					    cerr << "Method = " << method << " age = " <<  age.index() << ": " << selectivity_at_age(method,age.index()) << endl;
					}

				}
        	}
        }
    }

    void biomass_vulnerable_update(const Fishes& fishes) {
        biomass_vulnerable = 0;
        if (parameters.length_based_selectivity) {
			for (const Fish& fish : fishes) {
				if (fish.alive()) {
					auto weight = fish.weight();
					auto length_bin = fish.length_bin();
					for (auto method : methods) {
						biomass_vulnerable(fish.region,method) += weight * selectivity_at_length(method,length_bin);
					}
				}
			}
        } else {
			for (const Fish& fish : fishes) {
				if (fish.alive()) {
					auto weight = fish.weight();
					auto age_bin = fish.age_bin();
					for (auto method : methods) {
						biomass_vulnerable(fish.region,method) += weight * selectivity_at_age(method,age_bin);
					}
				}
			}
        }
        biomass_vulnerable *= fishes.scalar;
    }

    void catch_observed_update(void) {
        auto y = year(now);
        if (y >= Years_min and y <= Years_max) {
            for (auto region : regions) {
                for(auto method : methods) {
                    auto catches = parameters.harvest_catch_history(y,region,method);
                    catch_observed(region,method) = catches;
                }
            }
        }
    }

    void finalise(void) {
        // Report harvest info
        // Selectivities
        boost::filesystem::create_directories("output/harvest");
        if (parameters.length_based_selectivity)
        	selectivity_at_length.write("output/harvest/selectivity_at_length.tsv");
        else
        	selectivity_at_age.write("output/harvest/selectivity_at_age.tsv");
        // Actual removals
        catch_taken_by_year.write("output/harvest/actual_catch.tsv");
    }

};  // class Harvest
