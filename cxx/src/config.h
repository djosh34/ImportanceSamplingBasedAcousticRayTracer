

#include <boost/filesystem/path.hpp>
#include <glm/vec3.hpp>
#include <rays/Energy.h>
#include "settings.h"

#pragma once
struct Config {

    const int SEED = 0;
    const boost::filesystem::path filename = "/objects/cg_obj/simple_room.obj";

    // source plane
    const bool USE_SOURCE_PLANE = false;
    const boost::filesystem::path source_obj = "";
    const float MIN_SOURCE_RADIUS = 1.0f;

    // energy saving
    const bool SPECULAR_ENERGY = true;
    const bool DIFFUSE_ENERGY = true;

    // rays
    glm::vec3 SENDER_LOCATION{4.0f, -1.0f, 0.0f}; //
    glm::vec3 RECEIVER_LOCATION{-4.0f, -1.0f, 0.0f}; //
    const float RECEIVER_RADIUS = 0.5f; //
    const int RAYS_CAST = 10 * 1000;
    const int MAX_HIT_LEVEL = 5;

    // audio
    Energy SCATTERING_COEFFICIENT;
    Energy ABSORPTION_COEFFICIENT;

    // IS
    const bool IMPORTANCE_SAMPLING = true;
    const bool AUTO_RUN = false;
    const bool QUIT_AFTER_AUTO_RUN = true;
    const int AUTO_IMPORTANCE_SAMPLING_STEPS = 2;
    const boost::filesystem::path output_location = "histograms";

    const int PROJECTION_METHOD = EQUI_RECT;

    const float VOLUME = 0.0;
};



extern Config* global_config;
extern std::chrono::high_resolution_clock::time_point start_time;


Config initConfig();
Config initConfig(boost::filesystem::path path);