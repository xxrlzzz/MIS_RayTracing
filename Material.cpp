
#include "Material.hpp"

Material::Material(MaterialType t, Vector3f e, Vector3f Kd_)
    : m_type(t), m_emission(e), Kd(Kd_) {}
Material::~Material() = default;

class MaterialGLOSSY : public Material {
  public:
    MaterialGLOSSY(Vector3f e, Vector3f kd) : Material(GLOSSY, e, kd) {}
    ~MaterialGLOSSY() = default;
    [[nodiscard]] Vector3f sample(const Vector3f &wi,
                                  const Vector3f &N) const override;
    [[nodiscard]] float pdf(const Vector3f &wi, const Vector3f &wo,
                            const Vector3f &N) const override;
    [[nodiscard]] Vector3f eval(const Vector3f &wi, const Vector3f &wo,
                                const Vector3f &N) const override;
    void setSpecularExponent(float f) override { specularExponent = f; }

  private:
    float specularExponent{};
};

class MaterialDIFFUSE : public Material {
  public:
    MaterialDIFFUSE(Vector3f e, Vector3f kd) : Material(DIFFUSE, e, kd) {}
    ~MaterialDIFFUSE() = default;
    [[nodiscard]] Vector3f sample(const Vector3f &wi,
                                  const Vector3f &N) const override;
    [[nodiscard]] float pdf(const Vector3f &wi, const Vector3f &wo,
                            const Vector3f &N) const override;
    [[nodiscard]] Vector3f eval(const Vector3f &wi, const Vector3f &wo,
                                const Vector3f &N) const override;
    void setSpecularExponent(float f) override {}
};

// static
std::unique_ptr<Material> Material::Create(MaterialType t, Vector3f e,
                                           Vector3f kd) {
    switch (t) {
    case DIFFUSE:
        return std::make_unique<MaterialDIFFUSE>(e, kd);
    case GLOSSY:
        return std::make_unique<MaterialGLOSSY>(e, kd);
    }
    return nullptr;
}

Vector3f MaterialDIFFUSE::sample(const Vector3f &wi, const Vector3f &N) const {

    // uniform sample on the hemisphere
    float x_1 = get_random_float(), x_2 = get_random_float();
    float theta = acosf(sqrtf(1.0f - x_1));
    float phi = (M_PI * 2) * x_2;
    return toWorld(sphericalToCartesian(1.0, phi, theta), N);
}

Vector3f MaterialGLOSSY::sample(const Vector3f &wi, const Vector3f &N) const {
    float x_1 = get_random_float(), x_2 = get_random_float();
    float cos_a = std::pow(x_1, 1.f / specularExponent);
    float phi = x_2 * M_PI * 2;
    float theta = std::acos(cos_a);
    Vector3f H =
        Material::toWorld(Material::sphericalToCartesian(1.0, phi, theta), N);
    Vector3f L = Material::reflect(wi * -1.0, H);
    return L;
}

float MaterialDIFFUSE::pdf(const Vector3f &wi, const Vector3f &wo,
                           const Vector3f &N) const {

    return std::max(0.0f, dotProduct(N, wo)) * M_1_PI;
}
float MaterialGLOSSY::pdf(const Vector3f &wi, const Vector3f &wo,
                          const Vector3f &N) const {
    Vector3f H = normalize(wi + wo);
    float cos_a = dotProduct(N, H);
    float normalizationFactor = (specularExponent + 1.0f) / (M_PI * 2);
    float pdf = std::pow(cos_a, specularExponent) * normalizationFactor /
                (4.0f * dotProduct(wi, H));
    return pdf;
}

Vector3f MaterialDIFFUSE::eval(const Vector3f &wi, const Vector3f &wo,
                               const Vector3f &N) const {

    // calculate the contribution of diffuse model
    return Kd * M_1_PI;
}

Vector3f MaterialGLOSSY::eval(const Vector3f &wi, const Vector3f &wo,
                              const Vector3f &N) const {

    Vector3f H = normalize(wi + wo);
    float cos_a = dotProduct(N, H);
    return (specularExponent + 2.0f) * Kd / (8.0 * M_PI) *
           powf(cos_a, specularExponent);
}
