#include "Model/model.hpp"

// initialize static random engine to be shared by all instances of the Fish class

int main(int argc, char** argv) {
    std::setprecision(10);
    boost::random::random_device device;
    Engine engine(device);
    Model model(engine);
    model.initialise();

    try {
        std::string task;
        if (argc >= 2) {
            task = argv[1];
        }
        if (task == "run") {
            std::cout << "year\tfish alive(millions)\t size of population (total)\tstatus(%B0)\texprate\n";
            std::cout << std::setprecision(2);
            std::function<void()> callback([&](){
                std::cout
                    << now << "\t"
                    << model.fishes.number(false)/1e6 << "\t" << model.fishes.size()/1e6  << "\t"
                    << sum(model.fishes.biomass_spawners)/sum(parameters.fishes_b0) << "\t"
                    << sum(model.harvest.catch_taken)/sum(model.harvest.biomass_vulnerable) << std::endl;
            });
            model.run(1899, 2018, &callback);
        } else {
            std::cout << "No task (e.g. run) specified" <<std::endl;
        }
    } catch(std::exception& error) {
        std::cout << "************Error************\n"
                  << error.what() <<"\n"
                  << "******************************\n";
        return 1;
    } catch(...) {
        std::cout << "************Unknown error************\n";
        return 1;
    }

    model.finalise();

    return 0;
}
