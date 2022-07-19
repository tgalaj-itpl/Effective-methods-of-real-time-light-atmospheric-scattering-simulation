#pragma once
#include "skymodels/Atmosphere.h"

class Midpoint : public Atmosphere
{
public:
    explicit Midpoint(const Options & options);
    ~Midpoint() = default;

private:
    std::vector<IntegrationData> integrator(Ray ray, double a, double b, unsigned n, bool precomptute) override;
};

