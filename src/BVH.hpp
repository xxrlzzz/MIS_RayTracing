//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_BVH_H
#define RAYTRACING_BVH_H

#include <atomic>
#include <vector>
#include <memory>
#include <ctime>
#include <mutex>
#include <unordered_map>

#include "Object.hpp"
#include "Ray.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"

struct BVHBuildNode;
// BVHAccel Forward Declarations
struct BVHPrimitiveInfo;

// BVHAccel Declarations
inline int leafNodes, totalLeafNodes, totalPrimitives, interiorNodes;
class BVHAccel {

public:
    // BVHAccel Public Types
    enum class SplitMethod { NAIVE, SAH };

    // BVHAccel Public Methods
    explicit BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);
    explicit BVHAccel(const std::vector<std::unique_ptr<Object>>& p, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);
    [[nodiscard]] Bounds3 WorldBound() const;
    ~BVHAccel() = default;

    [[nodiscard]] Intersection Intersect(const Ray &ray) const;
    Intersection getIntersection(BVHBuildNode* node, const Ray& ray)const;
    bool IntersectP(const Ray &ray) const;
    std::unique_ptr<BVHBuildNode> root;

    // BVHAccel Private Methods
    std::unique_ptr<BVHBuildNode> recursiveBuild(std::vector<Object *>objects);

    // BVHAccel Private Data
    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<Object*> primitives;

    void getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf);
    void Sample(Intersection &pos, float &pdf);
};

struct BVHBuildNode {
    Bounds3 bounds;
    std::unique_ptr<BVHBuildNode> left = nullptr;
    std::unique_ptr<BVHBuildNode> right = nullptr;
    Object* object = nullptr;
    float area{};

public:
    int splitAxis=0, firstPrimOffset=0, nPrimitives=0;
    // BVHBuildNode Public Methods
    BVHBuildNode(){
        bounds = Bounds3();
        left = nullptr;right = nullptr;
        object = nullptr;
    }
};

#endif //RAYTRACING_BVH_H
