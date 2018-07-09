#include "Model.cpp"

// initialize static random engine to be shared by all instances of the Fish class
using namespace std;

int main(int argc, char* argv[]) {
#ifdef DEBUG
  cerr << "Enter main" << endl;
#endif
  // This is to give every Agent it's own unique random number generator implemented for the preference movement functionality
  int start_s=clock();
  Model model;
  model.initialise();

  try {
    string task;
    if (argc >= 2) {
      task = argv[1];
      if (argc == 3) {
        int seed;
        //seed = atoi(argv[2]);
        //model.set_seed(seed);
        //cout << "setting seed = " << seed << endl;
      }
    }

    if (task == "run") {
      cout << "year\tfish alive(millions)\t size of population (total)\tstatus(%B0)\texprate\n";
      cout << setprecision(6);
      function < void() > callback([&]() {
        cout
        << now << "\t"
        << model.agents_.number(false)/1e6 << "\t" << model.agents_.partition_.size()/1e6 << "\t"
        << sum(model.agents_.biomass_spawners_)/sum(parameters.fishes_b0) << "\t"
        << sum(model.harvest_.catch_taken_)/sum(model.harvest_.biomass_vulnerable_) << endl;
      });
      model.run(1899, 2018, &callback);
    } else {
      cout << "No task (e.g. run) specified" << endl;
    }
  } catch (exception& error) {
    cout << "************Error************\n" << error.what() << "\n" << "******************************\n";
    return 1;
  } catch (...) {
    cout << "************Unknown error************\n";
    return 1;
  }

  model.finalise();
  // the code you wish to time goes here
  int stop_s=clock();
  cout << setprecision(7);
  cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC) << endl;

  return 0;
}
