#pragma once // This is a call to the compiler to say that if included in a source file, only to parse it once <http://en.cppreference.com/w/cpp/preprocessor/impl>

#include "Agents.h"
#include "Environment.h"
#include "Harvest.h"
#include "Monitor.h"

#include "Random.h"
#include "Parameters.h"

// Will need to build the boost thread library
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>


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
      work_ctrl_ = new boost::asio::io_service::work(io_service_);
      //n_threads_ = boost::thread::hardware_concurrency() - 1;
      //if (n_threads_ <= 0)
      n_child_threads_ = 4;

      cout << "n threads = " << n_child_threads_ << endl;
      for (int i = 0; i < n_child_threads_; ++i) {
        threads_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
      }
    }

    ~Model() {
      delete environemnt_;
      environemnt_ = nullptr;
      io_service_.stop();
      threads_.join_all();
      delete work_ctrl_;
    }
    void run_individual_processes(Monitor& monitor,std::vector<Agent>& agents, unsigned first_element, unsigned last_element, bool burnin);
    void initialise(void);
    void finalise(void);
    void update(void);
    void pristine(Time time, function<void()>* callback = 0, bool called_after_seed = false);
    void run(Time start, Time finish, std::function<void()>* callback = 0,int initial = 0);
    //void set_seed(int seed) {parameters.seed = seed; }
    Environment*  get_environment_ptr(void) const {return environemnt_;}
  private:
    int defualt_value_ = 0;
    // Threading variables
    boost::asio::io_service io_service_;
    boost::thread_group threads_;
    boost::asio::io_service::work *work_ctrl_;
    unsigned n_child_threads_;
    int workCount_;
    boost::mutex mutex_;
    // Mutex for monitor class, they are mutually exclusive so we can probably deduce this to one mutex TODO
    boost::mutex mutex_monitor_;
    boost::mutex mutex_monitor_2_;
    boost::condition_variable condition_;
    int agents_per_thread_ = 0;

};
// end class Model
