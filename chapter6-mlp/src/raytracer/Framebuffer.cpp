#include <algorithm>
#include <string>
#include <glm/gtc/epsilon.hpp>

#include "Framebuffer.h"
#include "Ray.h"
#include "Timing.h"
#include "Utils.h"

#include "glm/glm.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <colourmanager.h>

bool Framebuffer::FLIP_HORIZONTALLY = false;
bool Framebuffer::FLIP_VERTICALLY = false;

Framebuffer::Framebuffer(const Options & options)
{
    m_options = options;
    m_width = m_options.WIDTH;
    m_height = m_options.HEIGHT;

    m_framebuffer = new glm::highp_dvec3[m_options.WIDTH * m_options.HEIGHT];
    memset(m_framebuffer, 0, sizeof(glm::highp_dvec3) * m_options.WIDTH * m_options.HEIGHT);

    /* Auto align space view */
    if(1)
    { 
        auto pos = m_options.OUTPUT_FILE_NAME.rfind('_');
        pos = m_options.OUTPUT_FILE_NAME.rfind('_', pos - 1);
        std::string space_view_scene = m_options.OUTPUT_FILE_NAME.substr(pos + 1);
        if (space_view_scene == "space_view")
        {
            m_options.CAM_ORIGIN = glm::highp_dvec3(0.0, m_options.ATMOSPHERE_RADIUS * 0.783436533, m_options.ATMOSPHERE_RADIUS * 1.958204334);
        }
    }
    /* END Auto align space view */

    m_options.CAM_ORIGIN *= m_options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR;
    glm::highp_dvec3 cam_pos = m_options.CAM_ORIGIN - glm::highp_dvec3(0.0f, m_options.PLANET_RADIUS * m_options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR + 1000.0f * m_options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR, 0.0f);
     
    m_cam = new Camera(m_options.WIDTH, 
                       m_options.HEIGHT,
                       cam_pos,
                       m_options.CAM_FOV,
                       m_options.CAM_PITCH,
                       m_options.CAM_YAW);
    
    m_cam->m_use_fisheye = m_options.CAM_FISHEYE;

    m_processed_pixel_counter = 0;
}

Framebuffer::~Framebuffer()
{
    delete[] m_framebuffer;
    delete m_cam;
}

void Framebuffer::render(Atmosphere & atmosphere)
{
#ifdef _DEBUG
    int num_threads = 1;
#else
    int num_threads = std::thread::hardware_concurrency();
#endif

    std::vector<int> bounds = thread_bounds(num_threads, m_options.HEIGHT);
    auto start_time         = Timing::getTime();

    printProgressBar(0, m_height, "Rendering:", "Complete");

    /* Use num_threads - 1 threads to raytrace the scene */
    for (int i = 0; i < num_threads - 1; ++i)
    {
        m_threads.push_back(std::thread(&Framebuffer::process, this, std::ref(atmosphere), bounds[i], bounds[i + 1]));
    }

    /* Use main thread to raytrace the scene */
    for (int i = num_threads - 1; i < num_threads; ++i)
    {
        process(atmosphere, bounds[i], bounds[i + 1]);
    }

    for (auto &t : m_threads)
    {
        t.join();
    }

    std::printf("\n");

    if (!m_options.OUTPUT_FILE_NAME.empty())
    {
        saveRender();
    }

    m_threads.clear();
    m_processed_pixel_counter = 0;
}

std::vector<glm::highp_dvec3> Framebuffer::getFramebufferRawData() const
{
    std::vector<glm::highp_dvec3> image;
    image.resize(m_options.WIDTH * m_options.HEIGHT * 4);

    for(uint32_t i = 0; i < m_options.WIDTH * m_options.HEIGHT; ++i)
    {
        image[i] = m_framebuffer[i];
    }

    return image;
}

void Framebuffer::process(Atmosphere & atmosphere, int left, int right)
{
    for (int y = left; y < right; ++y)
    {
        m_processed_pixel_counter += 1;

        for (uint32_t x = 0; x < m_options.WIDTH; ++x)
        {
            double x_coord = FLIP_HORIZONTALLY ? m_options.WIDTH - (x + 0.5f) : (x + 0.5f);
            double y_coord = FLIP_VERTICALLY ? m_options.HEIGHT - (y + 0.5f) : (y + 0.5f);
            Ray primary_ray = m_cam->getPrimaryRay(x_coord, y_coord);

            if (primary_ray.is_valid)
            {
                double t0, t1, t_max = std::numeric_limits<double>::max();
                if (atmosphere.intersect(primary_ray, t0, t1, true) && t1 > 0.0)
                {
                    t_max = glm::max(0.0, t0);
                }
                m_framebuffer[x + y * m_options.WIDTH] = atmosphere.computeIncidentLight(primary_ray, 0.0, t_max);
            }
        }
        printProgressBar(m_processed_pixel_counter.load(), m_height, "Rendering:", "Complete");
    }
}

std::vector<int> Framebuffer::thread_bounds(int parts, int mem) const
{
    std::vector<int> bounds;
    int delta = mem / parts;
    int reminder = mem % parts;
    int N1 = 0, N2 = 0;
    bounds.push_back(N1);

    for (int i = 0; i < parts; ++i)
    {
        N2 = N1 + delta;

        if (i == parts - 1)
        {
            N2 += reminder;
        }

        bounds.push_back(N2);
        N1 = N2;
    }

    return bounds;
}

void Framebuffer::saveRender(bool tonemap, double exposure) const
{
    std::vector<uint8_t> image;
    image.resize(m_options.WIDTH * m_options.HEIGHT * 4);

    for(uint32_t i = 0; i < m_options.WIDTH * m_options.HEIGHT; ++i)
    {
        if (tonemap)
        {
            // Apply exposure tone mapping
            m_framebuffer[i].r = 1.0 - glm::exp(-m_framebuffer[i].r * exposure);
            m_framebuffer[i].g = 1.0 - glm::exp(-m_framebuffer[i].g * exposure);
            m_framebuffer[i].b = 1.0 - glm::exp(-m_framebuffer[i].b * exposure);

            // Apply gamma correction
            m_framebuffer[i].r = glm::pow(m_framebuffer[i].r, 1.0f / 2.2f);
            m_framebuffer[i].g = glm::pow(m_framebuffer[i].g, 1.0f / 2.2f);
            m_framebuffer[i].b = glm::pow(m_framebuffer[i].b, 1.0f / 2.2f);
        }

        uint8_t r = uint8_t(255 * glm::clamp(m_framebuffer[i].r, 0.0, 100.0));
        uint8_t g = uint8_t(255 * glm::clamp(m_framebuffer[i].g, 0.0, 100.0));;
        uint8_t b = uint8_t(255 * glm::clamp(m_framebuffer[i].b, 0.0, 100.0));;
        uint8_t a = 255;

        image[4 * i + 0] = r;
        image[4 * i + 1] = g;
        image[4 * i + 2] = b;
        image[4 * i + 3] = a;
    }

    stbi_write_png((m_options.OUTPUT_FILE_NAME + ".png").c_str(), m_options.WIDTH, m_options.HEIGHT, 4, image.data(), 0);
}

void Framebuffer::saveImg(const std::string& filename, const std::vector<glm::highp_dvec3>& data)
{
    saveImg(filename, m_options.WIDTH, m_options.HEIGHT, data);
}

std::vector<glm::highp_dvec3> Framebuffer::loadImg(const std::string& filename)
{
    int w, h;
    return loadImg(filename, w, h);
}

std::vector<glm::highp_dvec3> Framebuffer::getLuminance(const std::vector<glm::highp_dvec3>& data)
{
    return getLuminance(m_options.WIDTH, m_options.HEIGHT, data);
}

std::vector<glm::highp_dvec3> Framebuffer::getChromaticity(const std::vector<glm::highp_dvec3>& data)
{
    return getChromaticity(m_options.WIDTH, m_options.HEIGHT, data);
}

std::vector<glm::highp_dvec3> Framebuffer::getMSE(const std::vector<glm::highp_dvec3>& ref, const std::vector<glm::highp_dvec3>& src, double& mse_error, double scale)
{
    std::vector<glm::highp_dvec3> image;
    image.resize(m_options.WIDTH * m_options.HEIGHT);

    double sum_sq = 0.0;

    ColourManager::Init_ColourManager();
    ColourManager manager(-1, 1); //err
    ColourMap map = CMList::getMapList(CMClassification::SEQUENTIAL)[2];
    ColourManager::setCurrentColourMap(map);

    double max_err = -999999990.0;
    double min_err = 99999999.0;

    for (uint32_t i = 0; i < m_options.WIDTH * m_options.HEIGHT; ++i)
    {
        auto src_luma = glm::dot(src[i], glm::highp_dvec3(0.2126, 0.7152, 0.0722));
        auto ref_luma = glm::dot(ref[i], glm::highp_dvec3(0.2126, 0.7152, 0.0722));

        double err = ref_luma - src_luma;
        double err2 = err * err;
        sum_sq += err2;
        
        Colour c = manager.getClassColour(err*scale);
        image[i] = glm::highp_dvec3(c.getR(), c.getG(), c.getB());

        if (err < min_err)
            min_err = err;

        if (err > max_err)
            max_err = err;
    }
    mse_error = sum_sq / image.size();

    std::cout << "mse error = " << mse_error << ", max err = " << max_err << ", min err = " << min_err << std::endl;

    return image;
}

std::vector<glm::highp_dvec3> Framebuffer::getLuminance(uint32_t width, uint32_t height, const std::vector<glm::highp_dvec3>& data)
{
    std::vector<glm::highp_dvec3> image;
    image.resize(width * height);

    ColourManager::Init_ColourManager();
    ColourManager manager(0.0, 255.0);
    ColourMap map = CMList::getMapList(CMClassification::SEQUENTIAL)[1];
    ColourManager::setCurrentColourMap(map);

    for (uint32_t i = 0; i < width * height; ++i)
    {
        auto luma = glm::dot(data[i], glm::highp_dvec3(0.2126, 0.7152, 0.0722));
        // image[i] = getScaleColor(luma);

        //const tinycolormap::Color color = tinycolormap::GetColor(luma, tinycolormap::ColormapType::Viridis);
        //image[i] = glm::highp_dvec3(color.r(), color.g(), color.b());

        Colour c = manager.getClassColour(luma * 255.0);
        image[i] = glm::highp_dvec3(c.getR(), c.getG(), c.getB());
    }

    return image;
}

std::vector<glm::highp_dvec3> Framebuffer::getChromaticity(uint32_t width, uint32_t height, const std::vector<glm::highp_dvec3>& data)
{
    std::vector<glm::highp_dvec3> image;
    image.resize(width * height);

    for (uint32_t i = 0; i < width * height; ++i)
    {
        // Apply gamma correction
        image[i].r = glm::pow(data[i].r, 1.0f / 0.4f);
        image[i].g = glm::pow(data[i].g, 1.0f / 0.4f);
        image[i].b = glm::pow(data[i].b, 1.0f / 0.4f);

        double rgb_max = glm::max(image[i].r, glm::max(image[i].g, image[i].b));
        image[i] = image[i] / rgb_max;
    }

    return image;
}

void Framebuffer::saveImg(const std::string& filename, const uint32_t width, const uint32_t height, const std::vector<glm::highp_dvec3>& data)
{
    std::vector<uint8_t> image;
    image.resize(width * height * 4);

    for (uint32_t i = 0; i < width * height; ++i)
    {
        uint8_t r = uint8_t(255 * glm::clamp(data[i].r, 0.0, 1.0));
        uint8_t g = uint8_t(255 * glm::clamp(data[i].g, 0.0, 1.0));;
        uint8_t b = uint8_t(255 * glm::clamp(data[i].b, 0.0, 1.0));;
        uint8_t a = 255;

        image[4 * i + 0] = r;
        image[4 * i + 1] = g;
        image[4 * i + 2] = b;
        image[4 * i + 3] = a;
    }

    stbi_write_png((filename + ".png").c_str(), width, height, 4, image.data(), 0);
}

std::vector<glm::highp_dvec3> Framebuffer::loadImg(const std::string& filename, int& w, int& h)
{
    std::vector<glm::highp_dvec3> out;

    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb);

    if (image == nullptr)
    {
        std::cerr << "Can't load file " << filename << std::endl;
        return out;
    }

    out.resize(width * height);

    w = width;
    h = height;

    for (uint32_t i = 0; i < width * height; ++i)
    {
        auto r = image[3 * i + 0] / 255.0;
        auto g = image[3 * i + 1] / 255.0;
        auto b = image[3 * i + 2] / 255.0;

        out[i] = glm::highp_dvec3(r, g, b);
    }

    stbi_image_free(image);

    return out;
}

void Framebuffer::createColorScaleBar(const std::string& output_filename, ColorScaleBarType type, unsigned width)
{
    std::vector<uint8_t> image;
    image.resize(width * 4);

    ColourManager::Init_ColourManager();
    ColourManager manager(0.0, 1.0);
    ColourMap map = CMList::getMapList(CMClassification::SEQUENTIAL)[int(type) + 1];
    ColourManager::setCurrentColourMap(map);

    for (uint32_t i = 0; i < width; ++i)
    {
        Colour c = manager.getClassColour(i / float(width - 1));
        auto output_color = glm::highp_dvec3(c.getR(), c.getG(), c.getB());

        uint8_t r = uint8_t(255 * glm::clamp(output_color.r, 0.0, 1.0));
        uint8_t g = uint8_t(255 * glm::clamp(output_color.g, 0.0, 1.0));;
        uint8_t b = uint8_t(255 * glm::clamp(output_color.b, 0.0, 1.0));;
        uint8_t a = 255;

        image[4 * i + 0] = r;
        image[4 * i + 1] = g;
        image[4 * i + 2] = b;
        image[4 * i + 3] = a;
    }

    stbi_write_png((output_filename + ".png").c_str(), width, 1, 4, image.data(), 0);
}
