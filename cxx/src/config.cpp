#include "config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <iterator>
#include <boost/preprocessor/repetition/enum.hpp>

#define TEXT(z, n, text) text[n]

using json = nlohmann::json;

Config initConfig() {
    return Config();
}

glm::vec3 jsonToVec(json::value_type json) {
    return {json["x"], json["y"], json["z"]};
}

Energy jsonToEnergy(json::value_type json) {
    return {
            BOOST_PP_ENUM(N_BANDS, TEXT, json)
    };
}

Config initConfig(boost::filesystem::path path) {

    std::cout << "Starting with config " << path << std::endl;

    path = boost::filesystem::current_path() / path;

    std::ifstream f(path.c_str());


    json configFile = json::parse(f);


    return {
            configFile["seed"],
            configFile["filename"],
            configFile["source"]["use_source_plane"],
            configFile["source"]["source_obj"],
            configFile["source"]["min_source_radius"],
            configFile["specular_energy"],
            configFile["diffuse_energy"],
            jsonToVec(configFile["sender_location"]),
            jsonToVec(configFile["receiver_location"]),
            configFile["receiver_radius"],
            configFile["rays_cast"],
            configFile["max_hit_level"],
            jsonToEnergy(configFile["scattering_coefficient"]),
            jsonToEnergy(configFile["absorption_coefficient"]),
            configFile["importance_sampling"],
            configFile["auto_run"],
            configFile["quit_after_auto_run"],
            configFile["auto_importance_sampling_steps"],
            configFile["output_location"],
            configFile.get<PROJECTION_METHODS>(),
                    configFile["volume"]
    };
}


