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

    // Public objects.
    map<unsigned, vector<vector<float>>> preference_by_year_;
    vector<double> lats; // These are expected to be upper and lower bins for each cell e.g. 10 20 30 = 2 cells with midpoints |10.5|20.5|
    vector<double> lons;
    vector<unsigned> years_;

    /*
     *  An accessor that returns the preference for a cell that an individual is in, in a year
     */
    float get_preference(double lat ,double lon, unsigned year) {
      return preference_by_year_[year][get_lat_index(lat)][get_long_index(lon)];
    }

    /*
     *  Return the longitude bin index that this longitude is in.
     */
    unsigned get_long_index(double lon) {
      unsigned index = 0;
      // skip the first bin as that is a lower bound and nothing should be less than that.
      for (unsigned this_lon = 1; this_lon <  lons.size(); ++this_lon,++index) {
        if (lon < lons[this_lon]) {
          return index;
        }
      }
      return lons.size() - 1;
    }

    /*
     *  Return the latitude bin index that this longitude is in.
     */
    unsigned get_lat_index(double lat) {
      unsigned index = 0;
      // skip the first bin as that is a lower bound and nothing should be less than that.
      for (unsigned this_lat = 1; this_lat <  lats.size(); ++this_lat,++index) {
        if (lat > lats[this_lat]) {
          return index;
        }
      }
      return lats.size() - 1;
    }


  /*
   * This method will read in data and do preliminary calcualtions and checks.
   */
    void initialise(void) {
      read_in_data();
    }

    void read_in_data(void) {
      // Define some variables that will be used throughout the code. temporary variables to be destroyed at the end of function call
      bool lat_file_found = false;
      bool long_file_found = false;
      string current_line;
      vector<string> current_line_parts;
      //map<unsigned, vector<vector<double>>> preference_by_year;

      boost::filesystem::path current_dir("input/environ"); //
      boost::filesystem::recursive_directory_iterator end;
      for (boost::filesystem::recursive_directory_iterator iter(current_dir), end; iter != end;  ++iter) {
        string name = iter->path().filename().string();
        cerr << "name = " << name << " directory " << iter->path().string() << endl;
        vector<string> file_name_parts;

        // make exceptions for certain files, this could be a way allowing for multiple components such as having 0 areas such as islands that individuals cannot head to.
        if (name == "lats.txt") {
          cerr << "found lats.txt" << endl;
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
              lats.push_back(element);
            }
          }

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
              lons.push_back(element);
            }
          }
          continue;
        }
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

        //cerr << "read in file, resize preference " << parameters.number_lat_bins << endl;
        // Allocate memory for rows, then push_back cols preferences[year][lat_ndx][long_ndx]
        //preference_by_year_[temp_year].resize(7);

      /*
       * iterate over each line and change seperators, then splits rows into columns and stores into preference.
       */
        cerr << "about to start reading" << endl;

        unsigned lat_iter = 0;
        while(getline(file, current_line)){
          vector<float> row_vector;
          cerr << "entering line number = " << lat_iter + 1 << " where line = " << current_line << endl;
          // Replace seperators
          boost::replace_all(current_line, "\t", " ");
          boost::replace_all(current_line, ",", " ");
          boost::replace_all(current_line, "  ", " ");

          // Break up columns
          boost::split(current_line_parts, current_line, boost::is_any_of(" "));

          // iterate over and store
          unsigned long_iter = 0;
          float element;
          for (auto col : current_line_parts) {
            if (!To<float>(col, element)) {
              cerr << "failed to convert " << col << " to double, either the code is shit or you have character at row '"<< lat_iter + 1 << "' and column '" << long_iter + 1 << "'" << endl;
            }
            row_vector.push_back(element);

            ++long_iter;
          }
          preference_by_year_[temp_year].push_back(row_vector);
          ++lat_iter;
        }
      }

      if (!long_file_found) {
        cerr << "could not find longs.txt, this is an expected file please check." << endl;
      }
      if (!lat_file_found) {
        cerr << "could not find lats.txt, this is an expected file please check." << endl;
      }

      //TODO
      // Check lats and longs are consistent, we don't want an issue where we are accessing out of memory elements in any given year
      // CHeck there is a preference for each year.
      // send an expression to the C++ error handler catch()
    }


    void finalise(void) {
      // Generate input layer
    }

  private:

};  // class Environ*/
