/**
 * A header file to include all external dependencies
 */

#pragma once

// C/C++ standard library
#include <fstream>
#include <vector>
#include <thread>
#include <map>
#include <iostream>
#include <string>
#include <cstdlib>

// Boost library
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

// Stencila structure, array and query classes
#include <stencila/dimension.hpp>
using Stencila::Dimension;
#include <stencila/array-static.hpp>
using Stencila::Array;
#include <stencila/query.hpp>
using namespace Stencila::Queries;
using namespace std;
using Stencila::StandardDeviation;
#include <stencila/structure.hpp>
using Stencila::Structure;


/*
*
 * This method converts a string into a lowercase version of it
 *
 * @param Copy of the string to lowercase
 * @return a lowercase version of the string
*/

inline string ToLowercase(const ::std::string arg) {
  string value = arg;
  for (char &c : value)
    c = tolower(c);

  return value;
};
/*

*
 * This is a shortened version of the method used
 * to call boost::lexical_cast<Target>(source);.
*/

template<typename Target>
bool To(const ::std::string arg, Target &result) {
  try {
    result = boost::lexical_cast<Target>(arg);
  } catch (...) {
    return false;
  }
  return true;
};

/**
 * This method will iterate through a vector and attempt to convert every
 * element in to the target type. Any invalid conversions will be returned
 * in a vector*/

template<typename Target>
vector<string> To(const vector<string>& source, vector<Target>& result) {
  vector<string> invalids;

  for (string value : source) {
    try {
      result.push_back(boost::lexical_cast<Target>(value));
    } catch (...) {
      invalids.push_back(value);
    }
  }

  return invalids;
}


/*
*
 * This is a specialisation for handling unsigned ints that are put in as negative
*/

template <>
inline bool To(const ::std::string arg, unsigned &result) {
  try {
    int temp = boost::lexical_cast<int>(arg);
    if (temp < 0)
      return false;
    result = (unsigned)temp;
  } catch (...) {
    return false;
  }

  return true;
}

/*
*
 * This is one of our specializations that handles
 * boolean types
 *
 * @param arg The arguement to check for valid boolean type
 * return true/false. Exception on failure
*/

template<>
inline bool To(const ::std::string arg, bool &result) {

  try {
    result = boost::lexical_cast<bool>(arg);
    return true;

  } catch (...) {
    vector<string> true_values  = { "t", "true", "yes", "y" };
    vector<string> false_values = { "f", "false", "no", "n" };

    string value = ToLowercase(arg);

    if (std::find(true_values.begin(), true_values.end(), value) != true_values.end()) {
      result = true;
      return true;
    }

    if (std::find(false_values.begin(), false_values.end(), value) != false_values.end()) {
      result = false;
      return true;
    }
  }

  return false;
}

/*
*
 * This is a method of converting from a known type to another
 * type. This method differs from above because we don't have
 * a hard-coded string source
*/

template<typename Source, typename Target>
bool To(const Source& arg, Target& result) {
  try {
    result = boost::lexical_cast<Target>(arg);
  } catch (...) {
    return false;
  }
  return true;
}

template<typename Source, typename Target>
Target ToInline(const Source arg) {
  Target result = Target();
  try {
    result = boost::lexical_cast<Target>(arg);
  } catch (...) {
    cerr << "Failed to do an inline conversion from " << arg;
  }
  return result;
}


