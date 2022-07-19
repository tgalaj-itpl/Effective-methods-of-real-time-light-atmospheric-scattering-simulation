#include <filesystem>
#include <iostream>
#include <memory>

#include "RootDir.h"
#include "raytracer/Framebuffer.h"
#include "raytracer/Timing.h"
#include "skymodels/midpoint/Midpoint.h"
#include "skymodels/precomputed_ss/PrecomputedSS.h"
#include "skymodels/deep_as/DeepAS.h"
#include "skymodels/img_based/ImgBased.h"
#include <include/stb_image_write.h>

#define FIGURES_DIR ROOT_DIR "/output/figures/"

/**
  * Enable to generate comparisons and image data like luminance, chromaticity, RMSE.
  * Disable to turn on normal rendering mode, based on below settings, for the enabled methods.
  */
#define CALC_MEASUREMENTS 1
#define MANUAL_EXPERIMENTS 1

#define ENABLE_MIDPOINT       0
#define ENABLE_DEEP_AS        1
#define ENABLE_IMG_BASED      0
#define ENABLE_PRECOMPUTED_SS 1

/**
  * Turn on to activate generation of fisheye renders and dataset that 
  * can be used in python framework to train img based network.
  *
  * NOTE: CALC_MEASUREMENTS should be disabled.
  */
#define RENDER_IMG_SEQUENCE 0


/* The following defines are set in Atmosphere.h */
const std::string mie_phase_func_name =
#if HG_PHASE_FUNC
"_hg";
#elif CS_PHASE_FUNC
"_cor-sha";
#elif S_PHASE_FUNC
"_schlick";
#endif

#if CALC_MEASUREMENTS
std::string time_to_string(int x)
{
    std::string out = "";

    if (x < 10)
    {
        out += "0";
    }

    out += std::to_string(x);
    return out;
}

struct Measurement
{
    int hours;
    int minutes;
    double sun_zenith;
    double sun_azimuth;
};

void calcRMSE(const std::string& name_ref, const std::string& name_src, std::vector<Measurement>& measurements, Options options, double color_error_scale=40.0)
{
    Framebuffer framebuffer(options);

    for (auto& m : measurements)
    {
        std::vector<std::string> experiments = { "rel_error" };

        for (int i = 0; i < experiments.size(); ++i)
        {
            std::string dir = ROOT_DIR "/output/egsr/4_" + experiments[i] + "/";
            std::string pre_filename = experiments[i] + "_" + time_to_string(m.hours) + "h" + time_to_string(m.minutes) + "_";

            std::string renders_dir = ROOT_DIR "/output/egsr/1_renders/";
            std::string renders_pre_filename = "renders_" + time_to_string(m.hours) + "h" + time_to_string(m.minutes) + "_";

            auto ref = framebuffer.loadImg(renders_dir + renders_pre_filename + name_ref + ".png");
            auto src = framebuffer.loadImg(renders_dir + renders_pre_filename + name_src + ".png");

            double mse;
            framebuffer.saveImg(dir + pre_filename + name_ref + "_vs_" + name_src, framebuffer.getMSE(ref, src, mse, color_error_scale));
            
            double rmse = std::sqrt(mse);

            /* Save rmse to a file */
            std::ofstream out_file(dir + pre_filename + name_ref + "_vs_" + name_src + ".txt");
            if (out_file.good())
            {
                out_file << rmse;
            }
            else
            {
                std::cout << "cant open file!" << std::endl;
            }
            out_file.close();
        }
    }
}

void calcAvgRMSE(const std::string& name_ref, const std::string& name_src, std::vector<Measurement>& measurements)
{
    double avg_rmse = 0.0;
    int samples_count = 0;

    std::vector<std::string> experiments = { "rel_error" };

    for (auto& m : measurements)
    {
        for (int i = 0; i < experiments.size(); ++i)
        {
            std::string dir = ROOT_DIR "/output/egsr/4_" + experiments[i] + "/";
            std::string pre_filename = experiments[i] + "_" + time_to_string(m.hours) + "h" + time_to_string(m.minutes) + "_";

            /* Load file and compute rmse */
            double rmse = 0.0;
            
            std::ifstream in_file(dir + pre_filename + name_ref + "_vs_" + name_src + ".txt");
            if (in_file.good())
            {
                in_file >> rmse;
            }
            else
            {
                std::cout << "cant open file!" << std::endl;
            }
            in_file.close();
            
            samples_count++;
            avg_rmse += rmse;
        }
    }

    avg_rmse = avg_rmse / double(samples_count);

    /* Save rmse to a file */
    std::string dir = ROOT_DIR "/output/egsr/4_" + experiments[0] + "/";

    std::ofstream out_file(dir + "avg_rmse_" + name_ref + "_vs_" + name_src + ".txt");
    if (out_file.good())
    {
        out_file << avg_rmse;
    }
    else
    {
        std::cout << "cant open file!" << std::endl;
    }
    out_file.close();
}
#if !MANUAL_EXPERIMENTS
void doExperiments(Atmosphere& atmosphere, const std::string& name, std::vector<Measurement>& measurements, Options options, double exposure = 1.0)
{
    for (auto& m : measurements)
    {
        auto zenith_angle = glm::radians(m.sun_zenith);
        auto azimuth_angle = glm::radians(m.sun_azimuth);

        options.SUN_DIRECTION = glm::normalize(glm::highp_dvec3(glm::cos(azimuth_angle) * glm::sin(zenith_angle),
            -glm::cos(zenith_angle),
            glm::sin(azimuth_angle) * glm::sin(zenith_angle)));

        Framebuffer framebuffer(options);
        atmosphere.setSunDirection(options.SUN_DIRECTION);
        atmosphere.m_zenith_angle = zenith_angle;
        atmosphere.m_azimuth_angle = azimuth_angle;
        framebuffer.render(atmosphere);

        std::vector<std::string> experiments = { "renders", "abs_luminance", "chroma" };

        for (int i = 0; i < experiments.size(); ++i)
        {
            std::string dir = ROOT_DIR "/output/egsr/" + std::to_string(i + 1) + "_" + experiments[i] + "/";
            std::string pre_filename = experiments[i] + "_" + time_to_string(m.hours) + "h" + time_to_string(m.minutes) + "_";

            std::string filename = dir + pre_filename + name;
            framebuffer.m_options.OUTPUT_FILE_NAME = filename;

            switch (i)
            {
            case 0: // renders
                framebuffer.saveRender(true, exposure);
                break;
            case 1: // abs luminance
                framebuffer.saveImg(filename, framebuffer.getLuminance(framebuffer.getFramebufferRawData()));
                break;
            case 2: // chromaticity
                framebuffer.saveImg(filename, framebuffer.getChromaticity(framebuffer.getFramebufferRawData()));
                break;
            }
        }
    }
}

int main(int argc, char** argv)
{
    int sizeof_vec3 = sizeof(glm::vec4);

    // 6, 7, 8, 9_30, 10_30, 11_30, 12_30, 13_30
    std::vector<Measurement> measurements = { {6, 0, 86.6242, 63.8769}, {7, 0, 76.32, 73.4198}, {8, 0, 65.4976, 82.7762} };
    std::vector<double> sun_zeniths       = { 48.9379, 46.2088, 43.5081, 40.846, 38.2352, 35.6912, 33.2334, 30.8862, 28.6804, 26.6542, 24.8547, 23.3369, 22.1608, 21.3844, 21.053, 21.1879, 21.7802 };
    std::vector<double> sun_azimuths      = { 98.1425, 101.094, 104.233, 107.600, 111.244, 115.223, 119.609, 124.483, 129.936, 136.066, 142.959, 150.668, 159.179, 168.365, 177.978, 187.669, 197.073 };

    int num_measurements = 17;
    for (int i = 0; i < num_measurements; ++i)
    {
        int minutes = 9 * 60 + 30 + i * 15;
        measurements.push_back({minutes / 60, minutes % 60, sun_zeniths[i], sun_azimuths[i]});
    }

    for (auto& mes : measurements)
    {
        std::cout << time_to_string(mes.hours) << ":" << time_to_string(mes.minutes) << " | sun zenith angle: " << mes.sun_zenith << " | sun azimuth angle: " << mes.sun_azimuth << std::endl;
    }
    std::cout << std::endl;

    std::string scene_file_name = "fisheye_res.txt";

    Options options;
    Scene::loadScene(scene_file_name, options);

    /* Preload LUT */
    const std::string planet_name = "earth";

    unsigned view_samples                = 512;
    unsigned light_samples               = 128;
    unsigned precomputed_lut_view_angles = 128;
    unsigned precomputed_lut_sun_angles  = 128;
    unsigned precomputed_lut_altitudes   = 128;

    std::string precomputed_file_name = "ss_lut_" + std::to_string(view_samples) + "_" +
                                                    std::to_string(light_samples) + "_" +
                                                    std::to_string(precomputed_lut_altitudes) + "_" +
                                                    std::to_string(precomputed_lut_sun_angles) + "_" +
                                                    std::to_string(precomputed_lut_view_angles) + "_rm_" + planet_name + ".txt";

    scene_file_name = "";
    PrecomputedSS atmosphere_dummy(options, view_samples, light_samples);
    atmosphere_dummy.loadSingleScatteringLUTToVector(precomputed_file_name);

    auto precomputed_lut = atmosphere_dummy.getLUT();

    if (precomputed_lut.empty())
    {
        atmosphere_dummy.precomputeSingleScattering(precomputed_lut_view_angles, precomputed_lut_sun_angles, precomputed_lut_altitudes);
        atmosphere_dummy.saveSingleScatteringLUT(precomputed_file_name);

        precomputed_lut = atmosphere_dummy.getLUT();
    }
    std::cout << "Elek's LUT size in bytes = " << sizeof(precomputed_lut[0]) * precomputed_lut.size() + sizeof(precomputed_lut[0][0]) * precomputed_lut[0].size() + sizeof(precomputed_lut[0][0][0]) * precomputed_lut[0][0].size() << std::endl << std::endl;
    /* End LUT prealoading*/

    Scene::loadScene(scene_file_name, options);
    options.printConfiguration();
    options.OUTPUT_FILE_NAME = "";

    /* Init atmo models */
    DeepAS atmosphere_deep(options);
    ImgBased atmosphere_img_based(options);
    PrecomputedSS atmosphere_elek(options, view_samples, light_samples);
    atmosphere_elek.loadSingleScatteringLUT(precomputed_lut);

    /* Perform experiments */
    std::cout << "Calculating Deep ..." << std::endl;
#if !USE_10LAYERS
    doExperiments(atmosphere_deep, "deep", measurements, options); 
#else
    doExperiments(atmosphere_deep, "deep_10layers", measurements, options); 
#endif
    std::cout << std::endl;

    std::cout << "Calculating ImgBased ..." << std::endl;
#if !IMG_BASED_TF
    #if USE_IMG_BASED_SYNTH
    doExperiments(atmosphere_img_based, "img_based_synth", measurements, options); 
    #else
    Framebuffer::FLIP_HORIZONTALLY = true;
    Framebuffer::FLIP_VERTICALLY   = true;
    doExperiments(atmosphere_img_based, "img_based_real", measurements, options, 0.28); 
    #endif
#else
    Framebuffer::FLIP_HORIZONTALLY = true;
    Framebuffer::FLIP_VERTICALLY   = true;
    doExperiments(atmosphere_img_based, "img_based_tf", measurements, options, 0.28); 
#endif
    Framebuffer::FLIP_HORIZONTALLY = false;
    Framebuffer::FLIP_VERTICALLY = false;
    std::cout << std::endl;

    std::cout << "Calculating Elek ..." << std::endl;
    doExperiments(atmosphere_elek, "elek", measurements, options); 
    std::cout << std::endl;

    /* Calculate RMSE for each image and average */
    std::cout << "Calculating RMSE Elek vs Deep..." << std::endl;
#if !USE_10LAYERS
    calcRMSE("elek", "deep", measurements, options); 
    calcAvgRMSE("elek", "deep", measurements); 
#else
    calcRMSE("elek", "deep_10layers", measurements, options);
    calcAvgRMSE("elek", "deep_10layers", measurements); 
#endif
    std::cout << std::endl;

#if 0
    std::cout << "Calculating RMSE Elek vs ImgBased synth..." << std::endl;
    calcRMSE("elek", "img_based_synth", measurements, options, 30.0); 
    calcAvgRMSE("elek", "img_based_synth", measurements);
    std::cout << std::endl;
#endif

#if 0
    measurements = { {9, 30, 0.0, 0.0}, {9, 45, 0.0, 0.0}, {10, 0, 0.0, 0.0}, {10, 15, 0.0, 0.0}, {10, 30, 0.0, 0.0}, {10, 45, 0.0, 0.0}, {11, 0, 0.0, 0.0}, 
                     {11, 15, 0.0, 0.0}, {11, 30, 0.0, 0.0}, {11, 45, 0.0, 0.0}, {12, 0, 0.0, 0.0}, {12, 15, 0.0, 0.0}, {12, 30, 0.0, 0.0}, {12, 45, 0.0, 0.0}, {13, 0, 0.0, 0.0} };
    std::cout << "Calculating RMSE Kiders vs ImgBased TF..." << std::endl;
    calcRMSE("kiders", "img_based_tf", measurements, options, 2.5);
    calcAvgRMSE("kiders", "img_based_tf", measurements);
    std::cout << std::endl;

    std::cout << "Calculating RMSE Kiders vs ImgBased Real..." << std::endl;
    calcRMSE("kiders", "img_based_real", measurements, options, 2.5);
    calcAvgRMSE("kiders", "img_based_real", measurements);
    std::cout << std::endl;
#endif

    Framebuffer::createColorScaleBar(ROOT_DIR "/output/color_scale_luminance", Framebuffer::ColorScaleBarType::LUMINANCE, 400);
    Framebuffer::createColorScaleBar(ROOT_DIR "/output/color_scale_rmse",      Framebuffer::ColorScaleBarType::RMSE,      400);

    return EXIT_SUCCESS;
}
#elif MANUAL_EXPERIMENTS
int main(int argc, char** argv)
{
    std::vector<std::string> all_args(argv + 1, argv + argc);
    std::string sub_dir = "ieee-cnn/";
    std::string methods_names[] = { "cnn_elek", "cnn_deep_2ru", "cnn_deep_6ru"};

    // Daytimes array
    // 6, 7, 8, 9_30, 10_30, 11_30, 12_30, 13_30
    std::vector<Measurement> measurements = { {6,  0,  86.6242, 63.8769}, 
                                              {7,  0,  76.3200, 73.4198}, 
                                              {8,  0,  65.4976, 82.7762}, 
                                              {9,  30, 48.9379, 98.1425}, 
                                              {10, 30, 38.2352, 111.244}, 
                                              {11, 30, 28.6804, 129.936}, 
                                              {12, 30, 22.1608, 159.179}, 
                                              {13, 30, 21.7802, 197.073} };

    for (auto& mes : measurements)
    {
        std::cout << time_to_string(mes.hours) << ":" << time_to_string(mes.minutes) << " | sun zenith angle: " << mes.sun_zenith << " | sun azimuth angle: " << mes.sun_azimuth << std::endl;
    }
    std::cout << std::endl;

    std::vector<std::string> experiments = { "2_abs_luminance", "3_chroma" };

    for(uint32_t i = 0; i < experiments.size(); ++i)
    {
       for (auto& method_name : methods_names)
       {
           for (auto& m : measurements)
           {
               std::string src_dir = ROOT_DIR "/output/" + sub_dir + "1_renders/";
               std::string src_pre_filename = "renders_" + time_to_string(m.hours) + "h" + time_to_string(m.minutes) + "_";
               std::string src_filename = src_dir + src_pre_filename + method_name + ".png";

                std::string out_dir = ROOT_DIR "/output/" + sub_dir + experiments[i] + "/";
                std::string out_pre_filename = experiments[i] + "_" + time_to_string(m.hours) + "h" + time_to_string(m.minutes) + "_";
                std::string out_filename = out_dir + out_pre_filename.substr(2) + method_name;

                int width = 0, height = 0;
                auto img = Framebuffer::loadImg(src_filename, width, height);

                switch (i)
                {
                case 0: // abs luminance
                    std::cout << "abs_luma " << out_filename << "\n";
                    Framebuffer::saveImg(out_filename, width, height, Framebuffer::getLuminance(width, height, img));
                    break;
                case 1: // chromaticity
                    std::cout << "chroma " << out_filename << "\n";
                    Framebuffer::saveImg(out_filename, width, height, Framebuffer::getChromaticity(width, height, img));
                    break;
                }
            }
            std::cout << "\n";
        }
    }
    // Generate abs luminance comparisions
    // Generate chroma comparisons

    return EXIT_SUCCESS;
}
#endif
#else
int main(int argc, char ** argv)
{
    std::string scene_file_name;

    std::vector<std::string> all_args(argv + 1, argv + argc);

    argc = 1;
    all_args = { "fisheye_dawn.txt", 
                 "fisheye_dusk.txt", 
                 "space_view.txt", 
                 "surface_view_dawn.txt",
                 "surface_view_dusk.txt"};
    
    /**
      * To change the properties of a planet, change the planet name variable
      * and also set appropriate defines in Options.h
      */
    const std::string planet_name = "earth";

    unsigned view_samples                = 512;
    unsigned light_samples               = 128;
    unsigned precomputed_lut_view_angles = 64;
    unsigned precomputed_lut_sun_angles  = 64;
    unsigned precomputed_lut_altitudes   = 64;

    std::string precomputed_file_name = "ss_lut_" + std::to_string(view_samples)                + "_" + 
                                                    std::to_string(light_samples)               + "_" + 
                                                    std::to_string(precomputed_lut_altitudes)   + "_" +
                                                    std::to_string(precomputed_lut_sun_angles)  + "_" +
                                                    std::to_string(precomputed_lut_view_angles) + "_rm_" + planet_name + ".txt";

#if !RENDER_IMG_SEQUENCE
    #if ENABLE_PRECOMPUTED_SS
    Options options_dummy;
    scene_file_name = all_args[0];
    Scene::loadScene(scene_file_name, options_dummy);
    
    scene_file_name = "";
    PrecomputedSS atmosphere_dummy(options_dummy, view_samples, light_samples);
    atmosphere_dummy.loadSingleScatteringLUTToVector(precomputed_file_name);
    
    auto precomputed_lut = atmosphere_dummy.getLUT();

    if (precomputed_lut.empty())
    {
        atmosphere_dummy.precomputeSingleScattering(precomputed_lut_view_angles, precomputed_lut_sun_angles, precomputed_lut_altitudes);
        atmosphere_dummy.saveSingleScatteringLUT(precomputed_file_name);

        precomputed_lut = atmosphere_dummy.getLUT();
    }
    #endif

    for (int i = 0; i < argc; ++i)
    {
        if (argc <= 1)
        {
            scene_file_name = "surface_view_dusk_photo.txt";
        }
        else
        {
            scene_file_name = all_args[i];
        }

        Options options;
        Scene::loadScene(scene_file_name, options);

        options.printConfiguration();
        std::string output_file_name = options.OUTPUT_FILE_NAME;

        #if ENABLE_MIDPOINT
        {
            options.OUTPUT_FILE_NAME = FIGURES_DIR + output_file_name + std::string("_") + "midpoint" + mie_phase_func_name;

            Framebuffer framebuffer(options);
            Midpoint atmosphere(options);

            auto start_time = Timing::getTime();
            framebuffer.render(atmosphere);
            std::cout << "\n" << "Midpoint processing time = " << Timing::getTime() - start_time << "s" << std::endl << std::endl;
        }
        #endif

        #if ENABLE_DEEP_AS
        {
            options.OUTPUT_FILE_NAME = FIGURES_DIR + output_file_name + std::string("_") + "deep_as" + mie_phase_func_name;

            Framebuffer framebuffer(options);
            DeepAS atmosphere(options);

            auto start_time = Timing::getTime();
            framebuffer.render(atmosphere);
            std::cout << "\n" << "Deep Atmospheric Scattering processing time = " << Timing::getTime() - start_time << "s" << std::endl << std::endl;
        }
        #endif

        #if ENABLE_IMG_BASED
        {
            options.OUTPUT_FILE_NAME = FIGURES_DIR + output_file_name + std::string("_") + "img_based" + mie_phase_func_name;

            Framebuffer framebuffer(options);
            ImgBased atmosphere(options);

            auto start_time = Timing::getTime();
            framebuffer.render(atmosphere);
            std::cout << "\n" << "Image Based NN processing time = " << Timing::getTime() - start_time << "s" << std::endl << std::endl;
        }
        #endif

        #if ENABLE_PRECOMPUTED_SS
        {
            options.OUTPUT_FILE_NAME = FIGURES_DIR + output_file_name + std::string("_") + "elek" + mie_phase_func_name;
            //options.OUTPUT_FILE_NAME = FIGURES_DIR "final/elek_" + planet_name + "_" + output_file_name;

            Framebuffer framebuffer(options);
            PrecomputedSS atmosphere(options, view_samples, light_samples);
            
            atmosphere.loadSingleScatteringLUT(precomputed_lut);

            auto start_time = Timing::getTime();
            framebuffer.render(atmosphere);
            std::cout << "\n" << "Elek processing time = " << Timing::getTime() - start_time << "s" << std::endl << std::endl;
        }
        #endif
    }

    std::cout << std::endl;
#else
	/**
      * Remember to disable multithreading in Framebuffer::render() for Release version. [TODO: This should be improved in the following versions of the code]
	  */
    /* Render animation */
    {
        Options options;
        Scene::loadScene("fisheye_res.txt", options);
        options.WIDTH  = 256;
        options.HEIGHT = 256;

        std::string output_file_name = options.OUTPUT_FILE_NAME;

        options.OUTPUT_FILE_NAME = FIGURES_DIR "animation/" + output_file_name + "_" + "precomputed_ea" + mie_phase_func_name;

        PrecomputedSS atmosphere(options, view_samples, light_samples, FIGURES_DIR "animation/synth_ea_dataset.txt");
        atmosphere.loadSingleScatteringLUTToVector(precomputed_file_name);

        if (atmosphere.getLUT().empty())
        {
            atmosphere.precomputeSingleScattering(precomputed_lut_view_angles, precomputed_lut_sun_angles, precomputed_lut_altitudes);
            atmosphere.saveSingleScatteringLUT(precomputed_file_name);
        }

        auto start_time = Timing::getTime();

        unsigned num_zenith_angles  = 30;
        unsigned num_azimuth_angles = 30;
        unsigned total_frames = num_zenith_angles * num_azimuth_angles;

        printf("Rendering image sequence of %d frames.\n\n", total_frames);
        for (unsigned z = 0; z < num_zenith_angles; ++z)
        {
            double zenith_angle = glm::pi<double>() / (num_zenith_angles - 1) * z;

            for (unsigned a = 0; a < num_azimuth_angles; ++a)
            {
                int    index         = z * num_zenith_angles + a;
                double azimuth_angle = glm::pi<double>() / (num_azimuth_angles - 1) * a;

                printf("Rendering image %d/%d, (%.2fm %.2fm) (elevation, azimuth)\n", index + 1, total_frames, glm::degrees(zenith_angle), glm::degrees(azimuth_angle));
                options.OUTPUT_FILE_NAME = FIGURES_DIR "animation/" + output_file_name + "_" + "precomputed_ea" + std::to_string(index);

                atmosphere.setSunDirection(glm::highp_dvec3( glm::cos(azimuth_angle)* glm::sin(zenith_angle),
                                                            -glm::cos(zenith_angle),
                                                             glm::sin(azimuth_angle)* glm::sin(zenith_angle)));
                atmosphere.m_azimuth_angle = azimuth_angle;
                atmosphere.m_zenith_angle = zenith_angle;

                Framebuffer framebuffer(options);
                framebuffer.render(atmosphere);
            }
        }

        std::cout << "\n" << "Precomputed processing time = " << Timing::getTime() - start_time << "s" << std::endl << std::endl;

        /* Uncomment to generate video from the generated image sequence using ffmpeg */
        //system("ffmpeg -framerate 60 -i skydome_precomputed_hg_%d.png -vf format=yuv420p skydome_precomputed_hg.mp4");
    }
#endif

    return EXIT_SUCCESS;
}
#endif