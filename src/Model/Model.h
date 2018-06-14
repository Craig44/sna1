#pragma once // This is a call to the compiler to say that if included in a source file, only to parse it once <http://en.cppreference.com/w/cpp/preprocessor/impl>

#include "Agents.h"
#include "Environment.h"
#include "Harvest.h"
#include "Monitor.h"

#include "Random.h"
#include "Parameters.h"


/**
 * The model
 *
 * Links together the sub-models e.g `Environment`, 'Agents` and `Harvest`
 */
class Model {
  public:
    Environment* environemnt_ = nullptr; // DO NOT call this environ, that is a macro in stdlib.h on windows which caused me many hours of suffering trying to find.
    Agents agents_;
    Harvest harvest_;
    Monitor monitor_;

    Model() :
      agents_(defualt_value_, this)
    {
      environemnt_ = new Environment;
    }

    ~Model() {
      delete environemnt_;
      environemnt_ = nullptr;
    }

    void initialise(void);
    void finalise(void);
    void update(void);
    void pristine(Time time, function<void()>* callback = 0, bool called_after_seed = false);
    void run(Time start, Time finish, std::function<void()>* callback = 0,int initial = 0);
    Environment*  get_environment_ptr(void) const {return environemnt_;}
  private:
    int defualt_value_ = 0;
};
// end class Model
