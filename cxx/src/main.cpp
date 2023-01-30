#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define EXIT_FAILURE "return -1"

#include <GLFW/glfw3.h>
#include <iostream>
#include <boost/filesystem.hpp>

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <trackball.h>
#include <window.h>
#include <vector>
#include <rays/Gmm.h>
#include <chrono>

#include "draw.h"
#include "Receiver.h"
#include "config.h"
#include "auto_runner.h"

constexpr glm::ivec2 windowResolution { 800, 800 };
Config* global_config;


void update_ray_iteration(std::vector<Ray> &all_rays, RaySettings &ray_settings, Receiver &receiver, int seed, Gmm &gmm);

void inline saveFileOfHistogram(std::vector<Ray> &all_rays, RaySettings &ray_settings) {
    Receiver receiver {global_config->RECEIVER_LOCATION, global_config->RECEIVER_RADIUS};


    HISTOGRAM_TYPE histogramType;

    if (global_config->SPECULAR_ENERGY) {
        histogramType = SPEC;
    }

    if (global_config->DIFFUSE_ENERGY) {
        histogramType = DIFFUSE;
    }

    if (global_config->DIFFUSE_ENERGY && global_config->SPECULAR_ENERGY) {
        histogramType = BOTH;
    }



    auto output_path = getAndMakeOutputPath(histogramType);

    receiver.listenToRays(all_rays, ray_settings);
    receiver.saveToFile(output_path / "histogram.csv");
    receiver.saveSettings(output_path / "histogram.json");
    std::cout << "Histogram has been written to file" << std::endl;
}


Config initConfig();
std::chrono::high_resolution_clock::time_point start_time;

GLFWwindow * initGl() {
    GLFWwindow* window;


    if (!glfwInit())
        EXIT_FAILURE;

    window = glfwCreateWindow(640, 480, "Display", NULL, NULL);


    if (!window)
    {
        glfwTerminate();
        EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    return window;
}

static void openGlStartLoop(Trackball &camera) {


    // Clear screen.
    glClearDepth(1.0f);
    glClearColor(0.1, 0.3, 0.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    // Normals will be normalized in the graphics pipeline.
    glEnable(GL_NORMALIZE);

    // transparency
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Activate the light in the legacy OpenGL mode.
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);


    // Create light components
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 0.4f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 0.4f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 0.4f };
    glm::vec4 senderLightPos { global_config->SENDER_LOCATION.x, global_config->SENDER_LOCATION.y, global_config->SENDER_LOCATION.z, 0 };

    // Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(senderLightPos));

    glEnable(GL_LIGHT1);
    glm::vec4 receiverLightPos { global_config->RECEIVER_LOCATION.x, global_config->RECEIVER_LOCATION.y, global_config->RECEIVER_LOCATION.z, 0 };

    // Assign created components to GL_LIGHT1
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT1, GL_POSITION, glm::value_ptr(receiverLightPos));




    // Draw front and back facing triangles filled
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);
    // Interpolate vertex colors over the triangles.
    glShadeModel(GL_SMOOTH);

    // Load view matrix.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const glm::mat4 viewMatrix = camera.viewMatrix();
    glMultMatrixf(glm::value_ptr(viewMatrix));

    // Load projection matrix.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const glm::mat4 projectionMatrix = camera.projectionMatrix();
    glMultMatrixf(glm::value_ptr(projectionMatrix));

    glShadeModel(GL_SMOOTH);

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);


}

int autoRun(std::vector<Ray> &all_rays, RaySettings &ray_settings, Receiver &receiver, Gmm &gmm) {
    int seed = global_config->SEED;
    std::cout << "Auto run seed: " << seed << std::endl;
    int is_steps = global_config->IMPORTANCE_SAMPLING ? global_config->AUTO_IMPORTANCE_SAMPLING_STEPS : 0;

    for (int i = 0; i < is_steps; i++) {
        seed++;
        update_ray_iteration(all_rays, ray_settings, receiver, seed, gmm);
    }

    if (global_config->QUIT_AFTER_AUTO_RUN) {
        saveFileOfHistogram(all_rays, ray_settings);
        return 0;
    }

    return 0;


}

int main(int argc, char** argv) {

    start_time = std::chrono::steady_clock::now();

    std::cout << "Starting with special type: " << SPECIAL_TYPE << std::endl;

    std::cout << "Loading model......" << std::endl;

    boost::filesystem::path configFile = boost::filesystem::path(argv[2]);
    Config config = initConfig(configFile);
    global_config = &config;

    std::string path = boost::filesystem::current_path().string() + global_config->filename.string();
    std::vector<Mesh> meshes = loadMesh(path);

    std::vector<Mesh> sourcePlanes;

    if (global_config->USE_SOURCE_PLANE) {
        std::string source_obj = boost::filesystem::current_path().string() + global_config->source_obj.string();
        sourcePlanes = loadMesh(source_obj);
    }

    AudioReflection audioReflection = {global_config->SCATTERING_COEFFICIENT, global_config->ABSORPTION_COEFFICIENT};
    for (int i = 0; i < meshes.size(); i++) {
        meshes.at(i).audioReflection = &audioReflection;
    }

    std::cout << "Model loaded" << std::endl;


    Window window { argv[0], windowResolution, OpenGLVersion::GL2 };
    Trackball camera { &window, glm::radians(50.0f), 3.0f };


    int ray_array_size = global_config->MAX_HIT_LEVEL * global_config->RAYS_CAST;
    std::vector<Ray> all_rays (ray_array_size);


    RaySettings ray_settings{global_config->RAYS_CAST, global_config->MAX_HIT_LEVEL, meshes};
    if (global_config->USE_SOURCE_PLANE) {
        ray_settings.initialize_source_locations(sourcePlanes);
    }

    initialize_meshes(ray_settings);




    Receiver receiver {global_config->RECEIVER_LOCATION, global_config->RECEIVER_RADIUS};
    Gmm gmm {};

    update_ray_iteration(all_rays, ray_settings, receiver, global_config->SEED, gmm);


    Draw draw (global_config->RAYS_CAST);

    int seed = 0;

    if (global_config->AUTO_RUN) {
        return autoRun(all_rays, ray_settings, receiver, gmm);
    }

    while(!window.shouldClose()) {
        openGlStartLoop(camera);

        if (global_config->USE_SOURCE_PLANE) {
            drawSourcePlanes(sourcePlanes);
            drawSources(ray_settings.sourceLocations);
        } else {
            drawShape(SENDER, global_config->SENDER_LOCATION);
        }

        drawShape(RECEIVER, global_config->RECEIVER_LOCATION);




        if (camera.keypress.pressed && camera.keypress.key == GLFW_KEY_C) {
            camera.keypress.pressed = false;
            update_ray_iteration(all_rays, ray_settings, receiver, seed, gmm);
            seed++;
        }

        if ((camera.keypress.pressed && camera.keypress.key == GLFW_KEY_F) || AUTO_OUTPUT) {
            camera.keypress.pressed = false;

            saveFileOfHistogram(all_rays, ray_settings);

            if (AUTO_OUTPUT) {
                return 0;
            }
        }



        bool animate = camera.isAnimate();
        float current_t = camera.current_t;


        draw.drawRays(receiver.diffuse_rays);

        if (DRAW_DIFFUSE_RAYS_ONLY) {
            draw.drawRays(receiver.diffuse_rays);
        } else if (animate) {
            draw.drawRaysAnimation(all_rays, current_t);
        } else {
            draw.drawRays(all_rays);
        }


        drawScene(meshes);


        window.swapBuffers();
        glfwPollEvents();
    }



    glfwTerminate();
    return 0;
}




void
update_ray_iteration(std::vector<Ray> &all_rays, RaySettings &ray_settings, Receiver &receiver, int seed, Gmm &gmm) {
    if (!global_config->IMPORTANCE_SAMPLING) {
        generateRays(all_rays, global_config->SENDER_LOCATION, ray_settings, seed);
        return;
    }

    std::cout << "Generating new rays with seed " << seed << std::endl;
    gmm.generateRays(all_rays, global_config->SENDER_LOCATION, ray_settings, seed);

    // for drawing
    if (DRAW_ONLY_INTERSECTIONS) {
        receiver.addSpecularEnergyToHistogram(all_rays);
    }
}






