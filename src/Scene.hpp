//
// Created by Göksu Güvendiren on 2019-05-14.
//

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "Vector.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "BVH.hpp"
#include "Ray.hpp"

enum SAMPLE{MIS,LIGHT,BRDF};

class Scene
{
    int max_depth = 105;
    static constexpr float RussianRoulette = 0.8;
    static constexpr float RR_inv = 1.f / RussianRoulette;
    Vector3f backgroundColor = Vector3f(0.01, 0.01, 0.01);
    std::vector<std::unique_ptr<Material>> materials;

    float emit_area_sum_cache = 0.f;

public:
    // setting up options
    double fov = 40;
    int width = 1280;
    int height = 960;
    SAMPLE sample;
    float mis_rate = 0.5f;

    Scene(int w, int h) : width(w), height(h),sample(LIGHT)
    {}

    bool UpdateRenderConfig(SAMPLE sample_, float mis_rate_) {
        if (sample != sample_ || mis_rate != mis_rate_) {
            sample = sample_;
            mis_rate = mis_rate_;
            return true;
        } else {
            return false;
        }
    }

    void Add(std::unique_ptr<Object> object);
    void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }
    void Add(std::unique_ptr<Material> material) { materials.push_back(std::move(material)); }

    // [[nodiscard]] const std::vector<std::unique_ptr<Object>>& get_objects() const { return objects; }
    [[nodiscard]] const std::vector<std::unique_ptr<Light> >&  get_lights() const { return lights; }
    [[nodiscard]] Intersection intersect(const Ray& ray) const;
    std::unique_ptr<BVHAccel> bvh;
    void buildBVH();
    [[nodiscard]] Vector3f castRay(const Ray &ray) const;
    void sampleLight(Intersection &pos, float &pdf) const;

    // creating the scene (adding objects and lights)
    std::vector<std::unique_ptr<Object> > objects;
    std::vector<std::unique_ptr<Light> > lights;


    [[nodiscard]] Vector3f shadeBRDF(const Ray &ray,Intersection hit_result, int depth, bool useMis = false) const;
    [[nodiscard]] Vector3f shadeLight(const Ray &ray,Intersection hit_result, int depth, bool useMis = false) const;

    [[nodiscard]] float lightChoosingPdf(Vector3f x,int light)const;

    void initLight();

};
#endif // SCENE_H