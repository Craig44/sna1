#pragma once

#include "Requirements.h"


/**
 * The "environment"
 * Reads in the environmental data, and calculates preference values.
 */
// TODO think about how best to deal with bounds..

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

  private:
    // protected  objects.
    map<unsigned, vector<vector<double>>> preference_by_year_;
    map<unsigned, vector<vector<double>>> sst_;
    vector<vector<double>>                depths_; // does not vary by year
    vector<vector<unsigned>>              region_index_; // does not vary by year
    vector<vector<Region>>                 region_;
    map<unsigned, vector<vector<double>>> zonal_preference_by_year_;
    map<unsigned, vector<vector<double>>> meridional_preference_by_year_;

    vector<double>                        lats_; // These are expected to be upper and lower bins for each cell e.g. 10 20 30 = 2 cells with midpoints |10.5|20.5|
    vector<double>                        lons_;
    vector<double>                        lon_mids_; // mid points for reporting
    vector<double>                        lat_mids_;
    vector<unsigned>                      years_;
    unsigned                              n_lats_;
    unsigned                              n_lons_;
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
    void                                  process_line(string current_line, vector<type>& final_line);
};  // class Environment*/
