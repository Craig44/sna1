#pragma once

#include "Requirements.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/split.hpp>

#include "Environment.h"

/**
 * Source file for the envirnoment
 */

/*
 *  An accessor that returns the preference to a zonal and meridional gradient relative to where they are, in a year
 */
vector<double> Environment::get_gradient(double lat ,double lon, unsigned year) {
  //cout << "lon = " << lon << " index = " << get_long_index(lon) << " lat = " << lat << " index = " <<  get_lat_index(lat) << endl;
  return {zonal_preference_by_year_[year][get_lat_index(lat)][get_long_index(lon)],meridional_preference_by_year_[year][get_lat_index(lat)][get_long_index(lon)]};
}


/*
 * This method will read in data and do preliminary calculations and checks.
 */
void Environment::initialise(void) {
  cout << "standard deviation for diffution = " << parameters.standard_dev_for_preference << endl;
  read_in_data();
  cout << "finished reading in data" << endl;
  calculate_preference_layer();
  cout << "finsihed calculating preference layer" << endl;
  calculate_gradient();
  cout << "finished calculating check for initial values -45 and 177" << endl;
  vector<double> val = get_gradient(-41.234, 187.232, 1900);
  cout << "lat index = " << get_lat_index(-41.234) << " long index = " <<  get_long_index(187.232) <<  " zonal (u) = " << val[0] <<  " meridional = " << val[1] << endl;

}

void Environment::finalise(void) {
  cerr << "finalise environ" << endl;
  // Generate input layer
  boost::filesystem::create_directories("output/environ");
  // print preference, zonal and meridional gradients
  boost::filesystem::create_directories("output/environ/preference");

  for (unsigned year : years_) {
    string temp_year = boost::lexical_cast<std::string>(year);
    string temp_dir = "output/environ/preference/" + temp_year + ".tsv";
    ofstream preference_by_year(temp_dir);
    preference_by_year << year << "\t";
    for (auto lon : lon_mids_) {
      preference_by_year << lon << "\t";
    }
    preference_by_year << "\n";
    for (unsigned i = 0; i < n_lats_; ++i) {
      preference_by_year << lat_mids_[i] << "\t";
      for (unsigned j = 0; j < n_lons_; ++j) {
        preference_by_year << preference_by_year_[year][i][j] << "\t";
      }
      preference_by_year << "\n";
    }
  }

  boost::filesystem::create_directories("output/environ/zonal");
  for (unsigned year : years_) {
    string temp_year = boost::lexical_cast<std::string>(year);
    string temp_dir = "output/environ/zonal/" + temp_year + ".tsv";
    ofstream preference_by_year(temp_dir);
    preference_by_year << year << "\t";
    for (auto lon : lon_mids_) {
      preference_by_year << lon << "\t";
    }
    preference_by_year << "\n";
    for (unsigned i = 0; i < n_lats_; ++i) {
      preference_by_year << lat_mids_[i] << "\t";
      for (unsigned j = 0; j < n_lons_; ++j) {
        preference_by_year << zonal_preference_by_year_[year][i][j] << "\t";
      }
      preference_by_year << "\n";
    }
  }
  boost::filesystem::create_directories("output/environ/meridional");
  for (unsigned year : years_) {
    string temp_year = boost::lexical_cast<std::string>(year);
    string temp_dir = "output/environ/meridional/" + temp_year + ".tsv";
    ofstream preference_by_year(temp_dir);
    preference_by_year << year << "\t";
    for (auto lon : lon_mids_) {
      preference_by_year << lon << "\t";
    }
    preference_by_year << "\n";
    for (unsigned i = 0; i < n_lats_; ++i) {
      preference_by_year << lat_mids_[i] << "\t";
      for (unsigned j = 0; j < n_lons_; ++j) {
        preference_by_year << meridional_preference_by_year_[year][i][j] << "\t";
      }
      preference_by_year << "\n";
    }
  }
  cerr << "exit finailise environ" <<endl;
}

/*
 *  Return the longitude bin index that this longitude is in.
 */
unsigned Environment::get_long_index(double lon) {
  unsigned index = 0;
  // skip the first bin as that is a lower bound and nothing should be less than that.
  for (unsigned this_lon = 1; this_lon <  lons_.size(); ++this_lon,++index) {
    if (lon <= lons_[this_lon]) {
      return index;
    }
  }
  return n_lons_;
}

/*
 *  Return the latitude bin index that this longitude is in.
 */
unsigned Environment::get_lat_index(double lat) {
  unsigned index = 0;
  // skip the first bin as that is a lower bound and nothing should be less than that.
  for (unsigned this_lat = 1; this_lat < lats_.size(); ++this_lat,++index) {
    if (lat <= lats_[this_lat]) {
      return index;
    }
  }
  return n_lats_;
}


/*
 * This method is responsible for reading in preference data.
*/
void Environment::calculate_gradient(void) {
  cout << "calculate gradient" << endl;
  // calcualte cell mid_points
  for (unsigned i = 0; i < n_lats_; ++i)
    lat_mids_.push_back((lats_[i] + lats_[i+1])/2.0);
  for (unsigned i = 0; i < n_lons_; ++i)
    lon_mids_.push_back((lons_[i] + lons_[i+1])/2.0);


  // Calculate gradient using forward, backwards, and central difference approximation
  // assume h = 1
  for (auto year : years_) {
    for (unsigned i = 0; i < n_lats_; ++i) {
      for (unsigned j = 0; j < n_lons_; ++j) {
        // calcualte meridional gradient
        if (i == 0) {
          meridional_preference_by_year_[year][i][j] = preference_by_year_[year][i + 1][j] - preference_by_year_[year][i][j];
        } else if (i == (n_lats_ - 1)) {
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
double Environment::pref_function(double x,double& mu,double& low_tol, double& upp_tol) {
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
void Environment::read_in_data(void) {
  // Define some variables that will be used throughout the code. temporary variables to be destroyed at the end of function call
  bool lat_file_found = false;
  bool long_file_found = false;
  string current_line;
  vector<string> current_line_parts;

  // So for I have only implemented sst and depth as descriptors of spatial distribution
  vector<string> dirs = {"input/environ/base", "input/environ/sst"};
  double optimum_preference = 0;
  double lower_preference = 0;
  double upper_preference = 0;

  for (unsigned dir = 0; dir < dirs.size(); ++dir) {

    if (dir == 0) {
      optimum_preference = parameters.depth_optimum;
      lower_preference = parameters.depth_lower;
      upper_preference = parameters.depth_upper;
      cout << "about to calculate depth preference optimum = " << optimum_preference << " lower = " << lower_preference << " upper = " << upper_preference << endl;

    } else if (dir == 1) {
      optimum_preference = parameters.sst_optimum;
      lower_preference = parameters.sst_lower;
      upper_preference = parameters.sst_upper;
      cout << "about to calculate sst preference optimum = " << optimum_preference << " lower = " << lower_preference << " upper = " << upper_preference << endl;

    }
    boost::filesystem::path current_dir(dirs[dir]); //

    // if it doesn't exist skip it
    if (!boost::filesystem::exists(current_dir)) {

      directories_.push_back(false);
      if (dir == 0) {
        cerr << "could not find directory " << dirs[dir] << endl;
        exit (EXIT_FAILURE);
      }

      continue;
    }
    boost::filesystem::recursive_directory_iterator end;

    for (boost::filesystem::recursive_directory_iterator iter(current_dir), end; iter != end;  ++iter) {
      string name = iter->path().filename().string();
      cerr << "name = " << name << " directory " << iter->path().string() << endl;
      vector<string> file_name_parts;

      // make exceptions for certain files, this could be a way allowing for multiple components such as having 0 areas such as islands that individuals cannot head to.
      if (name == "lats.txt") {
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
              exit (EXIT_FAILURE);
            }
            lats_.push_back(element);
          }
        }
        n_lats_ = lats_.size() - 1; // These are upper and lower bounds
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
              exit (EXIT_FAILURE);
            }
            lons_.push_back(element);
          }
        }
        n_lons_ = lons_.size() - 1; // These are upper and lower bounds
        continue;
      }
      if (name == "depths.txt") {
        cout << "found depths.txt" << endl;
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
        exit (EXIT_FAILURE);
      }
      years_.push_back(temp_year);

      // CHeck it isn't empty
      if (boost::filesystem::is_empty(iter->path())) {
        cerr << "file is empty something has gone wrong" << endl;
        exit (EXIT_FAILURE);
      }

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
        // Replace seperators
        boost::replace_all(current_line, "\t", " ");
        boost::replace_all(current_line, ",", " ");
        boost::replace_all(current_line, "  ", " ");

        // Break up columns
        boost::split(current_line_parts, current_line, boost::is_any_of(" "));

        if (current_line_parts.size() != n_lons_) {
          cerr << "found " << current_line_parts.size() << " columns in file '" << name << "', when there should be " << n_lons_ << endl;
          exit (EXIT_FAILURE);
        }
        // iterate over and store
        unsigned long_iter = 0;
        double element;
        for (auto col : current_line_parts) {
          if (!To<double>(col, element)) {
            cerr << "failed to convert " << col << " to double, either the code is shit or you have character at row '"<< lat_iter + 1 << "' and column '" << long_iter + 1 << "'" << endl;
            exit (EXIT_FAILURE);
          }
          row_vector.push_back(pref_function(element, optimum_preference, lower_preference, upper_preference));

          ++long_iter;
        }
        if (dir == 1) {
          sst_[temp_year].push_back(row_vector);
        }
        ++lat_iter;
      } // read line
      if (lat_iter != n_lats_) {
        cerr << "found " << (lat_iter) << " rows in file '" << name << "', when there should be " << n_lats_ <<  " please fix this " << endl;
        exit (EXIT_FAILURE);
      }
    } // Directory in Dirs
  } // Dirs

  if (!long_file_found) {
    cerr << "error: could not find longs.txt, this is an expected file please check it exists." << endl;
    exit (EXIT_FAILURE);
  }
  if (!lat_file_found) {
    cerr << "error: could not find lats.txt, this is an expected file please check it exists." << endl;
    exit (EXIT_FAILURE);
  }

  parameters.min_lat = lats_[0];
  parameters.max_lat = lats_[n_lats_];
  parameters.min_lon = lons_[0];
  parameters.max_lon = lons_[n_lons_];

  cerr << "min lat = " << parameters.min_lat << " max lat = " << parameters.max_lat << " min long = " << parameters.min_lon << " max long = " << parameters.max_lon << endl;
  // No time varying preference layer so create time-invarying preference for each year.
  if (years_.size() <= 0) {
    for(auto year = 1899; year <= 2019; ++year)
      years_.push_back(year);
  }
  //TODO
  // Check lats_ and longs are consistent, we don't want an issue where we are accessing out of memory elements in any given year
  // CHeck there is a preference for each year.
  // send an expression to the C++ error handler catch(),  currently just using the exit() call.
}

/*
 * This method is responsible for calculating the final preference which currently
 * is just made up of depth and sst.
*/

void Environment::calculate_preference_layer(void) {
  for (auto year : years_) {
    // Allocate memory
    preference_by_year_[year].resize(n_lats_);
    zonal_preference_by_year_[year].resize(n_lats_);
    meridional_preference_by_year_[year].resize(n_lats_);
    for (unsigned i = 0; i < n_lats_; ++i) {
      preference_by_year_[year][i].resize(n_lons_);
      zonal_preference_by_year_[year][i].resize(n_lons_);
      meridional_preference_by_year_[year][i].resize(n_lons_);
      for (unsigned j = 0; j < n_lons_; ++j) {
        preference_by_year_[year][i][j] = depths_[i][j];// pow(sst_[year][i][j] * depths_[i][j], 0.5); // 0.5 = 1/n where n = 2 TODO add back in
        //cout << "i = " << i + 1 << " j = " << j + 1 << " preference = " << preference_by_year_[year][i][j] << endl;
      }
    }
  }
}
