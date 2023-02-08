#include "AtmoNN.h"
#include "Timing.h"

AtmoNN::AtmoNN(const Options& options)
    : m_atmosphere(nullptr),
      m_cam(nullptr),
      m_options(options)
{
}

AtmoNN::~AtmoNN()
{
    delete m_atmosphere;
    delete m_cam;
    m_threads.clear();
}

void AtmoNN::init(RenderContext* target)
{
    /* Init Camera */
    m_cam = new Camera(target->getWidth(),
                       target->getHeight(),
                       m_options.CAM_ORIGIN,
                       m_options.CAM_FOV,
                       m_options.CAM_PITCH,
                       m_options.CAM_YAW,
                       m_options);

    m_cam->m_use_fisheye = m_options.CAM_FISHEYE;

    /* Init atmosphere */
    m_atmosphere = new DeepAS(m_options);

    /* Threads */
#ifdef _DEBUG
    m_num_threads = 1;
#else
    m_num_threads = std::thread::hardware_concurrency() - 1;
    std::cout << "num threads = " << m_num_threads << std::endl;
#endif

    m_bounds = thread_bounds(m_num_threads, target->getHeight());
    m_threads.resize(m_num_threads - 1);
}

void AtmoNN::updateAndRender(RenderContext* target, float delta)
{
    /* Use num_threads - 1 threads to raytrace the scene */
    for (int i = 0; i < m_num_threads - 1; ++i)
    {
        m_threads[i] = (std::thread(&AtmoNN::process, this, target, m_bounds[i], m_bounds[i + 1], delta));
    }

    /* Use main thread to raytrace the scene */
    for (int i = m_num_threads - 1; i < m_num_threads; ++i)
    {
        process(target, m_bounds[i], m_bounds[i + 1], delta);
    }

    for (auto& t : m_threads)
    {
        t.join();
    }
}

void AtmoNN::setSunDirection(double elevation_angle, double azimuth_angle)
{
    float el = glm::radians(elevation_angle);
    float az = glm::radians(azimuth_angle);

    glm::highp_dvec3 direction;
    direction.x = glm::sin(el) * glm::cos(az);
    direction.y = glm::cos(el);
    direction.z = glm::sin(el) * glm::sin(az);

    m_atmosphere->setSunDirection(-direction);
}

void AtmoNN::setCameraDir(double yaw, double pitch)
{
    m_cam->setDirection(yaw, pitch);
}

void AtmoNN::setCameraPosition(glm::highp_dvec3 new_position)
{
    m_cam->setPosition(new_position);
}

void AtmoNN::process(const RenderContext* const target, int left, int right, float delta)
{
    for (unsigned y = left; y < right; ++y)
    {
        for (unsigned x = 0; x < target->getWidth(); ++x)
        {
            Ray primary_ray = m_cam->getPrimaryRay(x + 0.5, y + 0.5);

            if (primary_ray.is_valid)
            {
                double t0, t1, t_max = std::numeric_limits<double>::max();
                if (m_atmosphere->intersect(primary_ray, t0, t1, true) && t1 > 0.0)
                {
                    t_max = glm::max(0.0, t0);
                }

                auto hdr_color = m_atmosphere->computeIncidentLight(primary_ray, 0.0, t_max);
                auto ldr_color = tonemap(hdr_color);

                target->drawPixel(x, y, ldr_color.r, ldr_color.g, ldr_color.b, 255);
            }
        }
    }
}

std::vector<int> AtmoNN::thread_bounds(int parts, int mem) const
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

glm::highp_ivec3 AtmoNN::tonemap(const glm::highp_dvec3& hdr_color, double exposure, double gamma)
{
    // Apply exposure tone mapping
    auto color = 1.0 - glm::exp(-hdr_color * exposure);

    // Apply gamma correction
    color.r = glm::pow(color.r, 1.0 / gamma);
    color.g = glm::pow(color.g, 1.0 / gamma);
    color.b = glm::pow(color.b, 1.0 / gamma);

    uint8_t r = uint8_t(255 * glm::clamp(color.r, 0.0, 1.0));
    uint8_t g = uint8_t(255 * glm::clamp(color.g, 0.0, 1.0));;
    uint8_t b = uint8_t(255 * glm::clamp(color.b, 0.0, 1.0));;

    return glm::highp_ivec3(r, g, b);
}