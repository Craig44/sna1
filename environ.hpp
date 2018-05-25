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


    void finalise(void) {
    	// Generate input layer
        boost::filesystem::create_directories("output/environ");
    }
};  // class Environ
