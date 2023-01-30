#include "Receiver.h"
#include "config.h"
#include <vector>
#include <set>
#include <rays/RayTracing.h>
#include <glm/geometric.hpp>
#include <fstream>
#include <better_assert.hpp>
#include <thread>
#include <sstream>
#include <boost/format.hpp>


boost::filesystem::path getAndMakeOutputPath(const HISTOGRAM_TYPE histogramType) {

    std::stringstream output_folder_name("");
    output_folder_name << (global_config->IMPORTANCE_SAMPLING ? "IS" : "NON_IS");

    output_folder_name << (histogramType == DIFFUSE ? "_DIFF" : "");
    output_folder_name << (histogramType == SPEC ? "_SPEC" : "");
    output_folder_name << (histogramType == BOTH ? "_BOTH" : "");

    int num_rays = global_config->RAYS_CAST;

    std::vector<std::string> amount_of_rays_string;
    while (num_rays > 0) {
        int remainder = num_rays % 1000;
        num_rays = num_rays / 1000;
        if (remainder == 0) {
            amount_of_rays_string.push_back(boost::str(boost::format("%03d") % remainder));
        } else {
            amount_of_rays_string.push_back(boost::str(boost::format("%d") % remainder));
        }
        amount_of_rays_string.push_back("_");

    }

    for (int i = amount_of_rays_string.size() - 1; i >= 0; i--) {
        output_folder_name << amount_of_rays_string.at(i);
    }

    std::string output_folder_name_string = output_folder_name.str();


    boost::filesystem::path output_path = boost::filesystem::current_path() / "output" / global_config->output_location / output_folder_name_string;
    auto makedir_command = "mkdir -p \"" + output_path.string() + "\"";
    system(makedir_command.c_str());

    return output_path;
}

void Receiver::listenToRays(std::vector<Ray> &all_rays, RaySettings &raySettings) {

    if (global_config->DIFFUSE_ENERGY) {
        addDiffuseEnergyToHistogram(all_rays, diffuse_rays, raySettings);
        auto output_path = getAndMakeOutputPath(DIFFUSE);
        saveToFile(output_path / "histogram.csv");
        saveSettings(output_path / "histogram.json");
        std::cout << "diffuse is saved at " << output_path << std::endl;
    }



    if (global_config->SPECULAR_ENERGY) {
        addSpecularEnergyToHistogram(all_rays);
    }

}

void Receiver::addEnergyToHistogram(std::vector<Ray> &all_rays, Ray &ray, float t, Energy &energy) {
    float total_t = ray.total_previous_t + t;
    float distance = total_t;
    double inverse_square_law_attenuation = attenuate_over_inverse_square_law(distance);
    float time_elapsed = convertTToRealTime(total_t);
    int histogram_index = (int) (time_elapsed * HISTOGRAM_SAMPLES_PER_SECOND);
    if (histogram_index >= HISTOGRAM_SAMPLES) {
        std::cout << "Increase histogram samples for time elapsed: " << time_elapsed << std::endl;
        return;
    }

    float probability_factor = 1.0f;


    if (global_config->IMPORTANCE_SAMPLING && ADJUST_ENERGY_WITH_PROBABILITY) {
        probability_factor = 1.0f / ray.initNums.probability;

        if (probability_factor > MAX_PROBABILITY_FACTOR) {

            probability_factor = MAX_PROBABILITY_FACTOR;
        }
    }



    for (int band = 0; band < N_BANDS; band++) {
        (*this->histogram).at(band).at(histogram_index) += inverse_square_law_attenuation * probability_factor * energy.values[band];
    }
}

void Receiver::addSpecularEnergyToHistogram(std::vector<Ray> &all_rays) {
    Sphere receiverSphere {global_config->RECEIVER_LOCATION, global_config->RECEIVER_RADIUS};

    std::set<int> ray_received_list;


    for (int i = 0; i < all_rays.size(); i++) {
        Ray &ray = all_rays.at(i);

        if (i % 10000 == 0) {
            std::cout << "\rSpecular ray: " << i << "/" << all_rays.size() << std::flush;
        }

        std::optional<float> t_sphere = intersectWithSphere(receiverSphere, ray);

        if (!t_sphere.has_value()) {
            continue;
        }

        float t = t_sphere.value();

        addEnergyToHistogram(all_rays, ray, t, ray.current_energy);
        ray_received_list.insert(ray.ray_start_index);
        ray.received = true;
        rays_through_receiver++;
    }

    std::cout << std::endl;

    for (Ray &ray : all_rays) {
        if (ray_received_list.find(ray.ray_start_index) != ray_received_list.end()) {
            ray.received_chain = true;
        }
    }

    std::cout << "Number of rays through receiver: " << rays_through_receiver << std::endl;


}


void diffuseIteration(Receiver *self, std::vector<Ray> &all_rays, RaySettings &raySettings, int i, int block,
                      int *ptotal_rays_done) {
    int s = i * block;
    int e = s + block;
    for (int ray_i = s; ray_i < e; ray_i++) {

        if (ray_i % 10000 == 0) {
            std::cout << "\rDiffuse ray: ";
            std::cout << *ptotal_rays_done << "/" << raySettings.amount_of_rays * raySettings.max_hit_level;
            *ptotal_rays_done += 10000;
        }




        Ray ray = all_rays.at(ray_i);

        if (ray.t >= infT) {
            continue;
        }

        glm::vec3 intersectionPoint = ray.hitInfo.hitPoint;
        glm::vec3 direction = glm::normalize(self->location - intersectionPoint);

        Ray diffuseRay {intersectionPoint, direction};
        detectHit(diffuseRay, raySettings);

        float distance_to_receiver = glm::length(self->location - intersectionPoint);
        float diffuse_ray_t = diffuseRay.t;

        // ray hits object before the receiver
        if (diffuse_ray_t < distance_to_receiver) {
            continue;
        }

        // ray may not hit anything
        if (diffuse_ray_t > infT) {
            continue;
        }

        Energy &a = ray.hitInfo.hitAudioReflection->absorption_coefficient;
        Energy &s = ray.hitInfo.hitAudioReflection->scattering_coefficient;
        float cos_theta = glm::abs(glm::dot(direction, ray.hitInfo.hitNormal));
        float cos_gamma_2 = self->receiver_radius / distance_to_receiver;
        if (distance_to_receiver < self->receiver_radius) {
            // attenuation is ignored
            cos_gamma_2 = 1;
        }

        float attenuation = 1.0f;

        // from schroder,Dirk p.64 eq 5.20
        // energy * (1 - a) * s * (1 - cos_gamma_2) * 2 * cos_theta * attenuation;
        Energy diffuseEnergy = ray.current_energy;
        diffuseEnergy.multiply(a.complement());
        diffuseEnergy.multiply(s);
        diffuseEnergy.multiply((1 - cos_gamma_2) * 2 * cos_theta * attenuation);

        self->addEnergyToHistogram(all_rays, ray, distance_to_receiver + ray.t, diffuseEnergy);
    }

}


void Receiver::addDiffuseEnergyToHistogram(std::vector<Ray> &all_rays, std::vector<Ray> &diffuse_rays, RaySettings &raySettings) {

    int block = all_rays.size() / NUM_THREADS;
    std::vector<std::thread> threads = std::vector<std::thread>();
    int total_rays_done = 0;
    int* ptotal_rays_done = &total_rays_done;

    for (int i = 0; i < NUM_THREADS; i++) {
        std::thread t(diffuseIteration, this, std::ref(all_rays), std::ref(raySettings), i, block, ptotal_rays_done);
        threads.push_back(std::move(t));
    }

    std::cout << "\rDiffuse ray: ";
    std::cout << *ptotal_rays_done << "/" << raySettings.amount_of_rays * raySettings.max_hit_level;

    for (auto& th : threads) {
        th.join();
    }
    std::cout << std::endl;

}


float Receiver::convertTToRealTime(float t) {
    // t in ray tracer is 1 m/s
    return t / SPEED_OF_SOUND;
}

void Receiver::saveToFile(boost::filesystem::path path) {
    std::ofstream out(path.c_str());

    for (int band = 0; band < N_BANDS; band++) {

        for (int i = 0; i < HISTOGRAM_SAMPLES; i++) {
            double num = (*this->histogram).at(band).at(i);

            out << num <<',';
            if (num != 0) {
                std::cout << std::flush;
            }
        }
        out << '\n';
    }


    out << '\n';
    out << std::flush;

}


void Receiver::saveSettings(boost::filesystem::path path) {
    std::ofstream out(path.c_str());

    auto end_time = std::chrono::high_resolution_clock::now();
    auto milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    out << "{";
    out << "\"HISTOGRAM_SAMPLES\":" << HISTOGRAM_SAMPLES << ",";
    out << "\"HISTOGRAM_SAMPLING_FREQUENCY\":" << HISTOGRAM_SAMPLING_FREQUENCY << ",";
    out << "\"HISTOGRAM_SECONDS\":" << HISTOGRAM_SECONDS << ",";
    out << "\"RAY_COUNT\":" << global_config->RAYS_CAST << ",";
    out << "\"MILLISECONDS_ELAPSED\":" << milliseconds_elapsed << ",";
    out << "\"RAYS_RECEIVED_BY_SPHERE\":" << rays_through_receiver << ",";
    out << "\"IMPORTANCE_SAMPLING\":" << global_config->IMPORTANCE_SAMPLING << ",";
    out << "\"VOLUME\":" << global_config->VOLUME << ",";
    out << "\"N_BANDS\":" << N_BANDS << ",";
    out << "\"BANDS\":[";
    for (int i = 0; i <= N_BANDS; i++) {
        out << BANDS[i];
        if (i != N_BANDS) {
            out << ", ";
        }
    }
    out << "]";

    out << "}";

    out << '\n';
}

double Receiver::attenuate_over_inverse_square_law(float distance) {
    return 1.0 / (distance*distance);
}


