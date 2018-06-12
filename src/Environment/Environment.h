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
    vector<double> get_gradient(double lat ,double lon, unsigned year);

  private:
    // protected  objects.
    map<unsigned, vector<vector<double>>> preference_by_year_;
    map<unsigned, vector<vector<double>>> sst_;
    vector<vector<double>>                depths_; // does not vary by year

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


    /*
     * Some in house utility functions used within the class
     */
    unsigned                              get_long_index(double lon);
    unsigned                              get_lat_index(double lat);
    void                                  calculate_gradient(void);
    double                                pref_function(double x,double& mu,double& low_tol, double& upp_tol);
    void                                  calculate_preference_layer(void);
    void                                  read_in_data(void);

};  // class Environment*/