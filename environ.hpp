#pragma once

#include "requirements.hpp"
//#include "dimensions.hpp"
//#include "parameters.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/split.hpp>

/**
 * The "environment"
 *
 * Currently just a placeholder
 */
// TODO think about how best to deal with bounds..

class Environ {
  public:
    /*
     *  An accessor that returns the preference to a zonal and meridional gradient relative to where they are, in a year
     */
    vector<double> get_gradient(double lat ,double lon, unsigned year) {
      return {zonal_preference_by_year_[year][get_lat_index(lat)][get_long_index(lon)],meridional_preference_by_year_[year][get_lat_index(lat)][get_long_index(lon)]};
    }
    /*
     * This method will read in data and do preliminary calculations and checks.
     */
      void initialise(void) {
        read_in_data();
        calculate_gradient();
      }

      void finalise(void) {
        // Generate input layer
        boost::filesystem::create_directories("output/environ");
        // print preference, zonal and meridional gradients
        boost::filesystem::create_directories("output/environ/preference");

        boost::filesystem::create_directories("output/environ/zonal");

        boost::filesystem::create_directories("output/environ/meridional");

      }

  protected:
    // protected  objects.
    map<unsigned, vector<vector<double>>> preference_by_year_;
    map<unsigned, vector<vector<double>>> sst_;
    vector<vector<double>>                depths_; // does not vary by year

    map<unsigned, vector<vector<double>>> zonal_preference_by_year_;
    map<unsigned, vector<vector<double>>> meridional_preference_by_year_;

    vector<double>                        lats_; // These are expected to be upper and lower bins for each cell e.g. 10 20 30 = 2 cells with midpoints |10.5|20.5|
    vector<double>                        lons_;
    vector<double>                        lon_mids_;
    vector<double>                        lat_mids_;
    vector<unsigned>                      years_;
    unsigned                              n_lats_;
    unsigned                              n_lons_;

    /*
     *  Return the longitude bin index that this longitude is in.
     */
    unsigned get_long_index(double lon) {
      unsigned index = 0;
      // skip the first bin as that is a lower bound and nothing should be less than that.
      for (unsigned this_lon = 1; this_lon <  lons_.size(); ++this_lon,++index) {
        if (lon < lons_[this_lon]) {
          return index;
        }
      }
      return lons_.size() - 1;
    }

    /*
     *  Return the latitude bin index that this longitude is in.
     */
    unsigned get_lat_index(double lat) {
      unsigned index = 0;
      // skip the first bin as that is a lower bound and nothing should be less than that.
      for (unsigned this_lat = 1; this_lat <  lats_.size(); ++this_lat,++index) {
        if (lat > lats_[this_lat]) {
          return index;
        }
      }
      return lats_.size() - 1;
    }




    /*
     * This method is responsible for reading in preference data.
    */
    void calculate_gradient(void) {
      // calcualte cell mid_points
      for (unsigned i = 0; i < (n_lats_ - 1); ++i)
        lat_mids_.push_back((lats_[i] + lats_[i+1])/2.0);
      for (unsigned i = 0; i < (n_lons_ - 1); ++i)
        lon_mids_.push_back((lons_[i] + lons_[i+1])/2.0);


      // Calculate gradient using forward, backwards, and central difference approximation
      // assume h = 1
      for (auto year : years_) {
        for (unsigned i = 0; i < n_lats_; ++i) {
          for (unsigned j = 0; j < n_lons_; ++j) {
            // calcualte meridional gradient
            if (i == 0) {
              meridional_preference_by_year_[year][i][j] = preference_by_year_[year][i + 1][j] - preference_by_year_[year][i][j];
            } else if (i == (n_lons_ - 1)) {
              meridional_preference_by_year_[year][i][j] = preference_by_year_[year][i][j] - preference_by_year_[year][i - 1][j];
            } else {
              meridional_preference_by_year_[year][i][j] = (preference_by_year_[year][i + 1][j] - preference_by_year_[year][i - 1][j]) / 2.0;
            }
            // calcualte zonal gradient
            if (j == 0) {
              zonal_preference_by_year_[year][i][j] = preference_by_year_[year][i][j + 1] - preference_by_year_[year][i][j];
            } else if (j == (n_lons_ - 1)) {
              zonal_preference_by_year_[year][i][j] = preference_by_year_[year][i][j] - preference_by_year_[year][i][j - 1];
            } else {
              zonal_preference_by_year_[year][i][j] = (preference_by_year_[year][i][j + 1] - preference_by_year_[year][i][j - 1]) / 2.0;
            }
          }
        }
      }
    }

    /*
     * @pref_function method is responsible for calculating the preference to an attribute.
     * @param x value of attribute
     * @param mu optimal value
     * @param low_tol lower value
     * @param upp_tol upper value
     *
     * @return the preference value
     *
    */
    double pref_function(double x,double& mu,double& low_tol, double& upp_tol) {
      if (x <= mu) {
        return exp(log(0.1) * pow(x - mu,2) / pow(low_tol - mu,2));
      }
      return exp(log(0.1) * pow(x - mu,2) / pow(upp_tol - mu,2));
    }

    /*
     * This method is responsible for reading in preference data.
     * This assumes there is two directories 'sst' and 'base'
     * in sst there are files labelled by year e.g. 1990.txt defining SST in a matrix
     * in base we are currently looking for depth.txt lats.txt and longs.txt
     * this can be expanded at a later date.
    */
    void read_in_data(void) {
      // Define some variables that will be used throughout the code. temporary variables to be destroyed at the end of function call
      bool lat_file_found = false;
      bool long_file_found = false;
      string current_line;
      vector<string> current_line_parts;
      //map<unsigned, vector<vector<double>>> preference_by_year;
      vector<string> dirs = {"input/environ/base", "input/environ/sst"};
      double optimum_preference = 0;
      double lower_preference = 0;
      double upper_preference = 0;

      for (unsigned dir = 0; dir < dirs.size(); ++dir) {

        if (dir == 0) {
          optimum_preference = parameters.depth_optimum;
          lower_preference = parameters.depth_lower;
          upper_preference = parameters.depth_upper;

        } else if (dir == 1) {
          optimum_preference = parameters.sst_optimum;
          lower_preference = parameters.sst_lower;
          upper_preference = parameters.sst_upper;
        }
        boost::filesystem::path current_dir(dirs[dir]); //

        // if it doesn't exist skip it
        if (!boost::filesystem::exists(current_dir))
          continue;

        boost::filesystem::recursive_directory_iterator end;


        for (boost::filesystem::recursive_directory_iterator iter(current_dir), end; iter != end;  ++iter) {
          string name = iter->path().filename().string();
          cerr << "name = " << name << " directory " << iter->path().string() << endl;
          vector<string> file_name_parts;

          // make exceptions for certain files, this could be a way allowing for multiple components such as having 0 areas such as islands that individuals cannot head to.
          if (name == "lats_.txt") {
            cerr << "found lats_.txt" << endl;
            lat_file_found = true;
            boost::filesystem::ifstream file{iter->path()};
            while(getline(file, current_line)){
              boost::replace_all(current_line, "\t", " ");
              boost::replace_all(current_line, ",", " ");
              boost::replace_all(current_line, "  ", " ");

              // Break up columns
              boost::split(current_line_parts, current_line, boost::is_any_of(" "));
              double element;
              for (auto col : current_line_parts) {
                if (!To<double>(col, element)) {
                  cerr << "failed to convert " << col << " to double, either the code is shit or you have character in the lats.txt file" << endl;
                }
                cerr << "loading values " << element << endl;
                lats_.push_back(element);
              }
            }
            n_lats_ = lats_.size();
            continue;
          }

          if (name == "longs.txt") {
            long_file_found = true;
            boost::filesystem::ifstream file{iter->path()};
            while(getline(file, current_line)){
              boost::replace_all(current_line, "\t", " ");
              boost::replace_all(current_line, ",", " ");
              boost::replace_all(current_line, "  ", " ");

              // Break up columns
              boost::split(current_line_parts, current_line, boost::is_any_of(" "));
              double element;
              for (auto col : current_line_parts) {
                if (!To<double>(col, element)) {
                  cerr << "failed to convert " << col << " to double, either the code is shit or you have character in the lons.txt file" << endl;
                }
                lons_.push_back(element);
              }
            }
            n_lons_ = lons_.size();
            continue;
          }
          if (name == "depths.txt") {
            boost::filesystem::ifstream file{iter->path()};
            while(getline(file, current_line)){
              vector<double> temp_depths;
              boost::replace_all(current_line, "\t", " ");
              boost::replace_all(current_line, ",", " ");
              boost::replace_all(current_line, "  ", " ");

              // Break up columns
              boost::split(current_line_parts, current_line, boost::is_any_of(" "));
              double element;
              for (auto col : current_line_parts) {
                if (!To<double>(col, element)) {
                  cout << "failed to convert " << col << " to double, either the code is shit or you have character in the depths.txt file" << endl;
                  exit (EXIT_FAILURE);
                }
                temp_depths.push_back(pref_function(element, parameters.depth_optimum, parameters.depth_lower, parameters.depth_upper));
              }
              depths_.push_back(temp_depths);
            }
            continue;
          }

          //
          // We should only be going pass this point for year specific characteristics.
          // Pull out the year and store it to do some checks later on.
          boost::split(file_name_parts, name, boost::is_any_of("."));
          cerr << "Year " << file_name_parts[0] << endl;
          unsigned temp_year;
          if (!To<unsigned>(file_name_parts[0], temp_year)) {
            cerr << "failed to convert " << file_name_parts[0] << " to unsigned int, either the code is shit or you have characters or decimals in the file name" << endl;
          }
          years_.push_back(temp_year);
          cerr << "converted = " << temp_year << endl;

          // CHeck it isn't empty
          if (boost::filesystem::is_empty(iter->path()))
            cerr << "file is empty something has gone wrong" << endl;

          // Now read in the file and save it into preferences array, I am going to make assumptions here these are
          // rows = lat 30
          // cols = long 30
          // I will allow the following seperators ',' '\t' ' ' '  ' but no others.
          boost::filesystem::ifstream file{iter->path()};


        /*
         * iterate over each line and change seperators, then splits rows into columns and stores into preference.
         */
          unsigned lat_iter = 0;
          while(getline(file, current_line)){
            vector<double> row_vector;
            cerr << "entering line number = " << lat_iter + 1 << " where line = " << current_line << endl;
            // Replace seperators
            boost::replace_all(current_line, "\t", " ");
            boost::replace_all(current_line, ",", " ");
            boost::replace_all(current_line, "  ", " ");

            // Break up columns
            boost::split(current_line_parts, current_line, boost::is_any_of(" "));

            // iterate over and store
            unsigned long_iter = 0;
            double element;
            for (auto col : current_line_parts) {
              if (!To<double>(col, element)) {
                cerr << "failed to convert " << col << " to double, either the code is shit or you have character at row '"<< lat_iter + 1 << "' and column '" << long_iter + 1 << "'" << endl;
              }
              row_vector.push_back(pref_function(element, optimum_preference, lower_preference, upper_preference));

              ++long_iter;
            }
            if (dir == 1) {
              sst_[temp_year].push_back(row_vector);
            }
            ++lat_iter;
          } // read line
        } // Directory in Dirs
      } // Dirs

      if (!long_file_found) {
        cout << "could not find longs.txt, this is an expected file please check." << endl;
        exit (EXIT_FAILURE);
      }
      if (!lat_file_found) {
        cout << "could not find lats_.txt, this is an expected file please check." << endl;
        exit (EXIT_FAILURE);
      }

      //TODO
      // Check lats_ and longs are consistent, we don't want an issue where we are accessing out of memory elements in any given year
      // CHeck there is a preference for each year.
      // send an expression to the C++ error handler catch()


    }

};  // class Environ*/
