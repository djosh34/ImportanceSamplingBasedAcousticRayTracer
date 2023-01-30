#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <nlohmann/json.hpp>

#pragma once

// histogram saving
#define AUTO_OUTPUT true

const boost::filesystem::path AUTO_OUTPUT_FILE_CSV = boost::filesystem::current_path() / "output" / "histogram.csv";
const boost::filesystem::path AUTO_OUTPUT_FILE_JSON = boost::filesystem::current_path() / "output" / "histogram.json";
const std::string SPECIAL_TYPE;

#define SET_CAMERA setCamera({4.79033, -5.49633, 0.487207},{0.49026, 1.53968, 0}, 17.85f);

// stochastic ray tracing
#define HISTOGRAM_SECONDS_M 4.0
const float HISTOGRAM_SECONDS = HISTOGRAM_SECONDS_M;
const int HISTOGRAM_SAMPLES_PER_SECOND = 44100;
const float HISTOGRAM_SAMPLING_FREQUENCY =( 1.0f / ((float) HISTOGRAM_SAMPLES_PER_SECOND));
const int HISTOGRAM_SAMPLES = (HISTOGRAM_SECONDS_M * HISTOGRAM_SAMPLES_PER_SECOND);

// Material
#define N_BANDS 8 // should be the same amount as these array!!!
const float BANDS [] {20.00000, 47.42747, 112.46827, 266.70429, 632.45553, 1499.78842, 3556.55882, 8433.93007, 20000.00000};
#define OneEnergyTemplate {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}

const glm::vec3 SENDER_COLOR{0.0f, 0.0f, 1.0f};
const glm::vec3 RECEIVER_COLOR{1.0f, 0.0f, 0.0f};
const float PARTY_OPACITY = 0.4f;

// math/physics constants
const float infT = std::numeric_limits<float>::max();
const float SPEED_OF_SOUND = 343.0f;

// ray tracing settings
const float EPSILON = 1e-05;
const int RANDOM_SEED = 42;
const int NUM_THREADS = 8;
const bool RANDOM_REFLECTION_RAYS = false;

// importance sampling
const bool ADJUST_ENERGY_WITH_PROBABILITY = true;
const double MAX_PROBABILITY_FACTOR = 1;

// projection methods
enum PROJECTION_METHODS {
    ZET_THETA,
    EQUI_RECT
};

NLOHMANN_JSON_SERIALIZE_ENUM( PROJECTION_METHODS, {
    {ZET_THETA, "ZET_THETA"},
    {EQUI_RECT, "EQUI_RECT"},
})

// drawing settings
const float RAY_TRANSPARENCY = 0.7;
const float STANDARD_TRANSPARENCY = 0.0;
const float RAY_RADIUS = 0.05f;
const int MAX_AMOUNT_RAYS = 500;
const float RAY_LINE_WIDTH = 3.0f;
const float SOURCE_RADIUS = 0.5f; //

// draw colors
const glm::vec4 RAY_HIT_COLOR{1.0f, 1.0f, 1.0f, 1.0f};
const glm::vec4 RAY_MISS_COLOR{1.0f, .2f, .2f, 1.0f};
#define SOURCE_PLANE_COLOR 0.0, 1.0, 0.5, 0.3

// draw modes
const bool DRAW_RAY_LINES = true;
const bool DRAW_RAY_POINTS = true;
const bool DRAW_ONLY_INTERSECTIONS = false;
const bool ENABLE_VIEW_FILTERING = true;
const bool DRAW_INDEX_1 = true;

const bool DRAW_RAY_ONLY_HITS = true;
const bool DRAW_DIFFUSE_RAYS_ONLY = false;


// animation settings
const float ANIMATION_T_STEP = 0.04f;
