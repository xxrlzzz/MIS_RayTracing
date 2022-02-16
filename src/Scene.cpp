//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

#include <memory>
#include <cmath>

#include "Sphere.hpp"

void Scene::buildBVH() {
    std::cout << " - Generating BVH...\n\n";
    this->bvh.reset(new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE));
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::initLight(){
    for(auto *object: objects){
        if(object->hasEmit()){
            auto* sphere = dynamic_cast<Sphere*>(object);
            Add(std::make_unique<Light>(sphere->getCenter(),sphere->getEmission()));
        }
    }
}
void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for(const auto & object:objects){
        if(object->hasEmit()){
            emit_area_sum += object->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for(const auto & object:objects){
        if(!object->hasEmit())
            continue;
        emit_area_sum += object->getArea();
        if(p <= emit_area_sum){
            object->Sample(pos,pdf);
            break;
        }
    }
}

/**
 * power balance
 */
float misWeightPower(float a,float b) {
    float a2 = a*a, b2 = b*b;
    return a2 / (a2+b2);
}
/**
 * average balance
 */
float misWeightBalance(float a,float b) {
    return a/(a+b);
}
/**
 * use different function to mix 2 pdf
 */
float misWeight(float pdfA,float pdfB) {
//    return misWeightBalance(pdfA,pdfB);
    return misWeightPower(pdfA,pdfB);
}

// pdf of light
// only support sphere light
float sphericalLightSamplingPdf(const Vector3f x,const Sphere* sphere){
    float solidangle = NAN ;
    Vector3f w = sphere->getCenter() - x;
    float dc_2 = dotProduct(w,w);
    if(dc_2 > sphere->getRadius2()){
        float sin_theta_max_2 = clamp(sphere->getRadius2()/dc_2,0.0,1.0);
        float cos_theta_max = sqrtf(1.0f - sin_theta_max_2);
        solidangle = M_PI*2 * (1.0 - cos_theta_max);
    }else{
        solidangle = M_PI*4;
    }
    return 1.0f / solidangle;
}

// unused
float Scene::lightChoosingPdf(Vector3f x,int light)const {
    int count = lights.size();
    float cdf[count];
    for(int i=0;i<count;++i){
        float len = (lights[i]->position - x).norm();
        cdf[i] = 1.0f / (len*len);
    }
    for(int i=1;i<count;++i){
        cdf[i] += cdf[i-1];
    }
    for(int i=0;i<count;++i){
        cdf[i] /= cdf[count-1];
    }
    return cdf[light] - (light==0? 0.0f : cdf[light-1]);
}

/**
 * sample to BRDF
 * @param ray       光线
 * @param depth     递归的深度
 * @param useMis    是否是MIS
 * @return
 */
Vector3f Scene::shadeBRDF(const Ray& ray,int depth,bool useMis)const{
    Intersection intersection = intersect(ray);
    if(!intersection.happened)
        // 没有命中
        return backgroundColor;

    Vector3f Lo;
    float weight = 1.0f;
    // 获得交点信息
    Vector3f p = intersection.coords;
    Vector3f wo = -ray.direction;
    Vector3f N = intersection.normal;
    const Material* m = intersection.m;

    // 撞到光源
    if (m->hasEmission() && depth == 0) {
        return m->getEmission();
    }

    // RR test
    if (get_random_float() > RussianRoulette) {
        return backgroundColor;
    }

    // correct normal
    if(dotProduct(wo,N)<0.0f){
        N = N * -1;
    }

    // 采样
    Vector3f wi = (m->sample(wo,N)).normalized();
    float cos_a = dotProduct(wi,N);
    float pdf = m->pdf(wo,wi,N);
    Vector3f fr = m->eval(wo,wi,N);

    //  value of pdf and cos_a should be meaningful
    if (pdf < EPSILON || cos_a < 0.0f) {
        return backgroundColor;
    }

    Intersection hit = intersect(Ray(p + wi * 0.01f, wi));
    const Material *hitm = hit.m;
    // 击中光源
    if (hit.happened) {
        if (hitm->hasEmission()) {
            if (useMis) {
                // 必须是球光源
                float lightPdf = sphericalLightSamplingPdf(hit.coords, dynamic_cast<const Sphere *>(hit.obj));
                std::cout << pdf << " " << lightPdf;
                weight = misWeight(pdf, lightPdf);
            }
            Lo = hitm->getEmission();
        } else {
            // 射出下一条光线
            if (useMis)
                weight = misWeight(pdf, (1.0 / (2 * M_PI)));
            // a little offset on start point to avoid hit p again
            Lo = shadeBRDF(Ray(p + wi * 0.01f, wi), depth + 1, useMis);
        }
        Lo = Lo * fr * cos_a * RR_inv * (1.0f / pdf);
        return Lo * weight;
    } else {
        return Lo;
    }
}
/**
 * sample to the light
 * @param ray
 * @param depth
 * @param useMis
 * @return
 */
Vector3f Scene::shadeLight(const Ray& ray,int depth,bool useMis) const {
    Intersection intersection = intersect(ray);
    float weight = 1.0f;
    if (!intersection.happened)
        return backgroundColor;

    Vector3f p = intersection.coords;
    Vector3f N = intersection.normal;
    Vector3f wo = -ray.direction;
    const Material *m = intersection.m;
    if (m->hasEmission() && depth == 0) {
        // 撞到光源
        return m->getEmission();
    }
    Vector3f L_dir, L_indir;
    {
        // 直接光照
        float pdf = 0;
        Intersection sampleInterSection;
        sampleLight(sampleInterSection, pdf);
        Vector3f x = sampleInterSection.coords;
        Vector3f ws = (x - p).normalized();
        Intersection hit = intersect(Ray(p + ws * 0.01f, ws));
        float cos_a = dotProduct(ws, N);
        // 中间没有阻挡
        if (hit.coords == x && cos_a > 0.0f && pdf > EPSILON) {
            Vector3f NN = sampleInterSection.normal;
            L_dir = sampleInterSection.emit * m->eval(wo, ws, N) * cos_a * dotProduct(-ws, NN);
            float redundant = (x - p).norm();
            redundant *= redundant;
            redundant *= pdf;

            if (useMis) {
                float brdfPdf = m->pdf(wo, ws, N) / hit.obj->getArea();
                weight = misWeight(pdf, brdfPdf);
            }
            L_dir = L_dir * (1.0f / redundant) * weight;
        }
    }
    // 间接光照
    if (get_random_float() <= RussianRoulette) {
        Vector3f wi = (m->sample(wo, N)).normalized();
        Intersection hit = intersect(Ray(p + wi * 0.01f, wi));
        const Material *hitMaterial = hit.m;
        float cos_a = dotProduct(wi, N);
        float pdf = m->pdf(wo, wi, N);
        Vector3f fr = m->eval(wo, wi, N);
        if (hitMaterial != nullptr && !hitMaterial->hasEmission() && cos_a > 0.0f && pdf > EPSILON) {
            L_indir = shadeLight(Ray(p + wi * 0.01f, wi), depth + 1, useMis) * fr *
                      cos_a * (1.0f / pdf) * RR_inv * weight;
        }
    }
    return L_dir + L_indir;
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const {
    if (sample == MIS) {
        Vector3f brdf = shadeBRDF(ray, depth, true);
        Vector3f light = shadeLight(ray, depth, true);
        // float p = brdf.norm() > light.norm() ? 0.2 : 0.8;
        // return lerp(brdf, light, p);
        return lerp(brdf, light, mis_rate);
    } else if (sample == LIGHT) {
        return shadeLight(ray, depth);
    } else {
        return shadeBRDF(ray, depth);
    }
}
