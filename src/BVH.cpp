#include "BVH.hpp"

#include <algorithm>
#include <cassert>
#include <mutex>

#include "Profiler.h"

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p)) {
    if (primitives.empty())
        return;
    RAIIProfiler profiler;
    root = recursiveBuild(primitives);
}

BVHAccel::BVHAccel(const std::vector<std::unique_ptr<Object>> &p,
                   int maxPrimsInNode, SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod) {
    for (auto &&obj : p) {
        primitives.push_back(obj.get());
    }
    if (primitives.empty())
        return;
    RAIIProfiler profiler;
    root = recursiveBuild(primitives);
}

std::unique_ptr<BVHBuildNode>
BVHAccel::recursiveBuild(std::vector<Object *> objects) {
    auto node = std::make_unique<BVHBuildNode>();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (auto &object : objects)
        bounds = Union(bounds, object->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->area = objects[0]->getArea();
        return node;
    } else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});
    } else {
        Bounds3 centroidBounds = Bounds3(objects[0]->getBounds().Centroid());
        for (auto &object : objects)
            centroidBounds =
                Union(centroidBounds, object->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        std::sort(objects.begin(), objects.end(), [=](auto f1, auto f2) {
            return f1->getBounds().Centroid()[dim] <
                   f2->getBounds().Centroid()[dim];
        });
        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);
    }
    node->bounds = Union(node->left->bounds, node->right->bounds);
    node->area = node->left->area + node->right->area;
    assert(bounds.pMin == node->bounds.pMin &&
           bounds.pMax == node->bounds.pMax);
    return node;
}

Intersection BVHAccel::Intersect(const Ray &ray) const {
    Intersection isect;
    if (!root)
        return isect;

    isect = BVHAccel::getIntersection(root.get(), ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode *node,
                                       const Ray &ray) const {
    if (!node->bounds.IntersectP(ray, ray.direction_inv)) {
        return Intersection();
    }
    if (node->left != nullptr) {
        Intersection interL = getIntersection(node->left.get(), ray);
        if (!interL.happened) {
            return getIntersection(node->right.get(), ray);
        }
        Intersection interR = getIntersection(node->right.get(), ray);
        if (interR.happened) {
            return interL.distance < interR.distance ? interL : interR;
        }
        return interL;
    } else {
        return node->object->getIntersection(ray);
    }
}

void BVHAccel::getSample(BVHBuildNode *node, float p, Intersection &pos,
                         float &pdf) {
    if (node->left == nullptr) {
        node->object->Sample(pos, pdf);
        pdf *= node->area;
        return;
    }
    if (p < node->left->area)
        getSample(node->left.get(), p, pos, pdf);
    else
        getSample(node->right.get(), p - node->left->area, pos, pdf);
}

void BVHAccel::Sample(Intersection &pos, float &pdf) {
    float p = std::sqrt(get_random_float()) * root->area;
    getSample(root.get(), p, pos, pdf);
    pdf /= root->area;
}