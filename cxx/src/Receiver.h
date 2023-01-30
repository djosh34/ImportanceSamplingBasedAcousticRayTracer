
#include <vector>
#include <glm/vec3.hpp>
#include <rays/Ray.h>
#include "settings.h"

struct Receiver {

    void saveToFile(boost::filesystem::path path);

    void saveSettings(boost::filesystem::path path);

    float receiver_radius{};

    void addEnergyToHistogram(std::vector<Ray> &all_rays, Ray &ray, float t, Energy &energy);

    void addSpecularEnergyToHistogram(std::vector<Ray> &all_rays);

public:

    Receiver(const glm::vec3 location, const float receiver_radius) {
        this->location = location;
        this->receiver_radius = receiver_radius;
        std::cout << "creating histogram" << std::flush;
        this->histogram = new std::array<std::array<double, HISTOGRAM_SAMPLES>, N_BANDS>({0.0f});
        std::cout << "\rhistogram created" << std::endl;
    }

    void listenToRays(std::vector<Ray> &all_rays, RaySettings &raySettings);
    std::array<std::array<double, HISTOGRAM_SAMPLES>, N_BANDS>* histogram;

    std::vector<Ray> diffuse_rays;
    glm::vec3 location{};


private:

    int rays_through_receiver = 0;
    static float convertTToRealTime(float t);

    void addDiffuseEnergyToHistogram(std::vector<Ray> &all_rays, std::vector<Ray> &diffuse_rays, RaySettings &raySettings);
    static double attenuate_over_inverse_square_law(float distance);
};

enum HISTOGRAM_TYPE {
    DIFFUSE,
    SPEC,
    BOTH
};


void diffuseIteration(Receiver *self, std::vector<Ray> &all_rays, RaySettings &raySettings, int i, int block, int* ptotal_rays_done);
boost::filesystem::path getAndMakeOutputPath(const HISTOGRAM_TYPE histogramType);

