#pragma once

#include "pmp/types.h"
#include <Eigen/Core>
#include <vector>
#include <cmath>

namespace pmp {

    struct Point3D {
        double x, y, z;
    };

    struct Sphere {
        Point3D center;
        double radius;
    };

    Sphere findBoundingSphere(const std::vector<Point3D>& points) {
        Point3D center = { 0, 0, 0 };
        int num_points = points.size();

        // 중심점 계산
        for (const auto& point : points) {
            center.x += point.x;
            center.y += point.y;
            center.z += point.z;
        }
        center.x /= num_points;
        center.y /= num_points;
        center.z /= num_points;

        // 반지름 계산
        double max_distance = 0;
        for (const auto& point : points) {
            double distance = std::sqrt(std::pow(point.x - center.x, 2) +
                std::pow(point.y - center.y, 2) +
                std::pow(point.z - center.z, 2));
            max_distance = std::max(max_distance, distance);
        }

        return { center, max_distance };
    }

} // namespace pmp
