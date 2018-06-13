#include "Model.cpp"

// initialize static random engine to be shared by all instances of the Fish class
using namespace std;


int main(int argc, char** argv) {
    cout << "enter main" << endl;
    // This is to give every Agent it's own unique random number generator implemented for the preference movement functionality
    Model model;
    model.initialise();

    try {
        string task;
        if (argc >= 2) {
            task = argv[1];
        }
        if (task == "run") {
            cout << "year\tfish alive(millions)\t size of population (total)\tstatus(%B0)\texprate\n";
            cout << setprecision(2);
            function<void()> callback([&](){
                cout
                    << now << "\t"
                    << model.agents.number(false)/1e6 << "\t" << model.agents.size()/1e6  << "\t"
                    << sum(model.agents.biomass_spawners_)/sum(parameters.fishes_b0) << "\t"
                    << sum(model.harvest.catch_taken_)/sum(model.harvest.biomass_vulnerable_) << endl;
            });
            model.run(1899, 2018, &callback);
        } else {
            cout << "No task (e.g. run) specified" << endl;
        }
    } catch(exception& error) {
        cout << "************Error************\n"
                  << error.what() <<"\n"
                  << "******************************\n";
        return 1;
    } catch(...) {
        cout << "************Unknown error************\n";
        return 1;
    }

    model.finalise();

    return 0;
}
