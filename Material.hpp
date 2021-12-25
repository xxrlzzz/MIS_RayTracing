//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

enum MaterialType { DIFFUSE , GLOSSY};

class Material{
private:

    // Compute reflection direction
    [[nodiscard]] Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
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
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    [[nodiscard]] Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
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
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    static Vector3f toWorld(const Vector3f &a, const Vector3f &N){
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)){
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x *invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y *invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
        float sign_ = N.z>0.0? 1:-1;
        float A = -1.0f/(sign_+N.z);
        float b = N.x * N.y *A;
        Vector3f b1(1.0f+sign_*N.x*N.x*A,sign_*b,-sign_*N.x);
        Vector3f b2(b,sign_+N.y*N.y*A,-N.y);
        return a.x * b1 + a.y * b2+ a.z * N;
    }

    static Vector3f sphericalToCartesian(float rho, float phi, float theta){
        float sin_a = std::sin(theta);
        return Vector3f(sin_a*std::cos(phi),sin_a*std::sin(phi),std::cos(theta)) * rho;
    }

public:
    MaterialType m_type;
    Vector3f m_emission;
    float ior{};
    Vector3f Kd, Ks;
    float specularExponent{};

    inline explicit Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0)) ;
    [[nodiscard]] inline MaterialType getType() const;
    //inline Vector3f getColor();
    [[nodiscard]] inline Vector3f getEmission() const;
    [[nodiscard]] inline bool hasEmission() const;

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);
    // given a ray, calculate the PdF of this ray
    [[nodiscard]] inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) const;
    // given a ray, calculate the contribution of this ray
    [[nodiscard]] inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) const;

};

Material::Material(MaterialType t, Vector3f e){
    m_type = t;
    m_emission = e;
}

MaterialType Material::getType() const{return m_type;}
Vector3f Material::getEmission() const {return m_emission;}
bool Material::hasEmission() const {
    return m_emission.norm() > EPSILON;
}

Vector3f Material::sample(const Vector3f &wi, const Vector3f &N){

    switch(m_type){
        case DIFFUSE :
        {
            // uniform sample on the hemisphere
            float x_1 = get_random_float(), x_2 = get_random_float();
            float theta = acosf(sqrtf(1.0f - x_1));
            float phi = (M_PI*2) * x_2;
            return toWorld(sphericalToCartesian(1.0,phi,theta),N);
        }
        case GLOSSY:
        {
            float x_1 = get_random_float(),x_2 = get_random_float();
            float cos_a = std::pow(x_1,1.f/specularExponent);
            float phi = x_2 * M_PI*2;
            float theta = std::acos(cos_a);
            Vector3f H = toWorld(sphericalToCartesian(1.0,phi,theta),N);
            Vector3f L = reflect(wi*-1.0,H);
            return L;
        }
    }
    return Vector3f(0.0f);
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) const{
    switch(m_type){
        case DIFFUSE :
        {
            // uniform sample probability 1 / (2 * PI)
             return std::max(0.0f,dotProduct(N,wo)) * M_1_PI;
//            return 0.5f / M_PI;
        }
        case GLOSSY:
        {
            Vector3f H = normalize(wi + wo);
            float cos_a = dotProduct(N,H);
            float normalizationFactor = (specularExponent + 1.0f) / (M_PI*2);
            float pdf = std::pow(cos_a, specularExponent) * normalizationFactor / (4.0f * dotProduct(wi,H));
//            return std::min(pdf,1.0f);
            return pdf;
        }
    }
    return 0.0f;
}
// evalueteSample
// wi => 视点方向, wo => 出射方向
Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) const{
    switch(m_type){
        case DIFFUSE:
        {
            // calculate the contribution of diffuse model
            return Kd * M_1_PI;
        }
        case GLOSSY:
        {
            Vector3f H = normalize(wi + wo);
            float cos_a = dotProduct(N,H);
            return (specularExponent + 2.0f)*Kd / (8.0 * M_PI)  * powf(cos_a,specularExponent);
        }
    }
    return Vector3f(0.0f);
}

#endif //RAYTRACING_MATERIAL_H
