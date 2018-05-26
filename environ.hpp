#pragma once

#include "requirements.hpp"

/**
 * The "environment"
 *
 * Currently just a placeholder
 */

using std::ifstream;
using std::string;

class Environ {
	public:
  // Public containers
  //map<unsigned, vector<vector<double>>> preferences;
  //vector<double> lats;
  //vector<double> lons;



  // For now I am just going to read in data and store it the initialise class.
  void initialise(void) {
/*
    cerr << "entering initialsie" << endl;

    // Define some variables that will be used throughout the code. temporary variables to be destroyed at the end of function call
    bool lat_file_found = false;
    bool long_file_found = false;
    string current_line;
    vector<string> current_line_parts;

    vector<unsigned> years_;
    path current_dir("input/environ"); //
    for (recursive_directory_iterator iter(current_dir), end; iter != end;  ++iter) {
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
            }
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

      // Now read in the file and save it into preferences array, I am going to make assumptions here these are
      // rows = lat 30
      // cols = long 30
      // I will allow the following seperators ',' '\t' ' ' '  ' but no others.
      boost::filesystem::ifstream file{iter->path()};

      // Allocate memory for rows, then push_back cols preferences[year][lat][long]
      preferences[temp_year].resize(parameters.number_lat_bins);



        * iterate over each line and change seperators, then splits rows into columns and stores into preference.

      unsigned lat_iter = 0;
      while(getline(file, current_line)){
        cerr << "entering line number = " << lat_iter + 1 << endl;
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
          cerr << "value = " << element << " ";
          preferences[temp_year][lat_iter].push_back(element);

          ++long_iter;
        }
        cerr << endl;
        ++lat_iter;
      }

    }

    if (long_file_found) {
      cerr << "could not find longs.txt, this is an expected file please check." << endl;
    }
    if (lat_file_found) {
      cerr << "could not find lats.txt, this is an expected file please check." << endl;
    }
*/
    // Check that there is .txt file for each year of the model e.g. 1990.txt 1991.txt
    boost::filesystem::create_directories("output/environ");
  }

    void finalise(void) {
    	// Generate input layer
        boost::filesystem::create_directories("output/environ");
    }
};  // class Environ
