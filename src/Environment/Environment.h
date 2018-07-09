#pragma once

#include "Requirements.h"


/**
 * The "environment"
 * Reads in the environmental data, and calculates preference values.
 */
// TODO think about how best to deal with domain bounds currently need to do a check which is a tax..

class Environment {
  public:
    Environment() {};
    ~Environment() {};

    void initialise(void);
    void finalise(void);
    /*
     *  An accessor that returns the preference gradient to a zonal and meridional gradient relative to where they are, in a year
     */
    vector<double> get_gradient(const double & lat ,const double & lon,const  unsigned & year);
    // So that an agent can query the environment what region they are in
    Region get_region(const double & lat ,const double & lon) {return region_[get_lat_index(lat)][get_long_index(lon)];};

    double recruit_latitude(Region region) {return recruitment_lats_[region][chance() * recruitment_lats_[region].size()];};
    double recruit_longitude(Region region) {return recruitment_lons_[region][chance() * recruitment_lons_[region].size()];};

  private:
    // protected  objects.
    map<unsigned, vector<vector<double>>> preference_by_year_;
    map<unsigned, vector<vector<double>>> sst_;
    map<unsigned, vector<vector<double>>> catch_;
    vector<vector<double>>                depths_; // does not vary by year
    vector<vector<unsigned>>              region_index_; // does not vary by year
    vector<vector<unsigned>>              recruitment_index_;
    vector<vector<Region>>                region_;
    map<unsigned, vector<vector<double>>> zonal_preference_by_year_;
    map<unsigned, vector<vector<double>>> meridional_preference_by_year_;


    vector<double>                        lats_; // These are expected to be upper and lower bins for each cell e.g. 10 20 30 = 2 cells with midpoints |10.5|20.5|
    vector<double>                        lons_;
    // Midpoints to plant recruits in, can put in vector form so we can randomly assign recruits in.
    map<Region, vector<double>>           recruitment_lats_;
    map<Region, vector<double>>           recruitment_lons_;
    vector<double>                        lon_mids_; // mid points for reporting
    vector<double>                        lat_mids_;
    vector<unsigned>                      years_;
    unsigned                              n_lats_ = 1;
    unsigned                              n_lons_ = 1;
    vector<bool>                          directories_;
    double                                optimum_preference_ = 0;
    double                                lower_preference_ = 0;
    double                                upper_preference_ = 0;

    /*
     * Some in house utility functions used within the class
     */
    unsigned                              get_long_index(double lon);
    unsigned                              get_lat_index(double lat);
    void                                  calculate_gradient(void);
    vector<double>                        pref_function(vector<double> values,double& mu,double& low_tol, double& upp_tol);
    void                                  calculate_preference_layer(void);
    void                                  read_in_data(void);
    void                                  convert_region_object(void);
    template<typename type>
    void                                  process_line(string current_line, vector<type>& final_line, string& file_name);
};  // class Environment*/
