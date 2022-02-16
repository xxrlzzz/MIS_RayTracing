//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include "global.hpp"

enum MaterialType { DIFFUSE, GLOSSY };

class Material {
  protected:
    // Compute reflection direction
    [[nodiscard]] Vector3f reflect(const Vector3f &I, const Vector3f &N) const {
        return I - 2 * dotProduct(I, N) * N;
    }

    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and
    // negate the normal N
    [[nodiscard]] Vector3f refract(const Vector3f &I, const Vector3f &N,
                                   const float &ior) const {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) {
            cosi = -cosi;
        } else {
            std::swap(etai, etat);
            n = -N;
        }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? Vector3f() : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior,
                 float &kr) const {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {
            std::swap(etai, etat);
        }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        } else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) /
                       ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) /
                       ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is
        // given by: kt = 1 - kr;
    }

    static Vector3f toWorld(const Vector3f &a, const Vector3f &N) {
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)) {
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x * invLen);
        } else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y * invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
        float sign_ = N.z > 0.0 ? 1 : -1;
        float A = -1.0f / (sign_ + N.z);
        float b = N.x * N.y * A;
        Vector3f b1(1.0f + sign_ * N.x * N.x * A, sign_ * b, -sign_ * N.x);
        Vector3f b2(b, sign_ + N.y * N.y * A, -N.y);
        return a.x * b1 + a.y * b2 + a.z * N;
    }

    static Vector3f sphericalToCartesian(float rho, float phi, float theta) {
        float sin_a = std::sin(theta);
        return Vector3f(sin_a * std::cos(phi), sin_a * std::sin(phi),
                        std::cos(theta)) *
               rho;
    }

    MaterialType m_type;
    Vector3f m_emission;
    float ior{};

  public:
    Vector3f Kd, Ks;
    std::unique_ptr<Material> static Create(MaterialType t = DIFFUSE,
                                            Vector3f e = Vector3f(),
                                            Vector3f kd = Vector3f());
    // float specularExponent{};

    virtual ~Material();

    explicit Material(MaterialType t, Vector3f e, Vector3f kd);
    [[nodiscard]] inline MaterialType getType() const { return m_type; }
    // inline Vector3f getColor();
    [[nodiscard]] inline Vector3f getEmission() const { return m_emission; }
    [[nodiscard]] inline bool hasEmission() const {
        return m_emission.norm() > EPSILON;
    }
    virtual void setSpecularExponent(float s) = 0;
    // sample a ray by Material properties
    virtual Vector3f sample(const Vector3f &wi, const Vector3f &N) const = 0;
    // given a ray, calculate the PdF of this ray
    [[nodiscard]] virtual float pdf(const Vector3f &wi, const Vector3f &wo,
                                    const Vector3f &N) const = 0;
    // given a ray, calculate the contribution of this ray
    [[nodiscard]] virtual Vector3f eval(const Vector3f &wi, const Vector3f &wo,
                                        const Vector3f &N) const = 0;
};

#endif // RAYTRACING_MATERIAL_H
