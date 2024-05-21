#ifndef INNER_SPHERE_TREE_H
#define INNER_SPHERE_TREE_H

#include <cmath>
#include <vector>
#include <queue>
#include "pmp/bounding_box.h"
#include "pmp/algorithms/normals.h"
#include "pmp/types.h"
#include "DrawComponent.h"
#include "BVH.h"
#include "pmp/io/io.h"
#include "pmp/algorithms//normals.h"
#include "pmp/bounding_box.h"
#include "GL/glut.h"
#include "GL/freeglut.h"

#define M_PI 3.141592

BoundingBox AABB(Face faces, SurfaceMesh& mesh) {
    BoundingBox box;
    for (auto v : mesh.vertices(faces))
    {
        auto& p = mesh.position(v);
        box += p;
    }
    return box;
}

float x, y, z;
const float c = 0.0174533; // 각도를 라디안으로 변환하는 상수

void drawSphere() 
{
    for (float phi = -90.0; phi <= 70.0; phi += 20.0)
    {
        glBegin(GL_QUAD_STRIP);
        for (float theta = -180.0; theta <= 180.0; theta += 20.0)
        {
            x = sinf(c * theta) * cosf(c * phi);
            y = cosf(c * theta) * cosf(c * phi);
            z = sinf(c * phi);
            glVertex3d(x, y, z);
            x = sin(c * theta) * cos(c * (phi + 20.0));
            y = cos(c * theta) * cos(c * (phi + 20.0));
            z = sin(c * (phi + 20.0));
            glVertex3d(x, y, z);
        }

        glEnd();

    }
}
class InnerSphereTree {
public:
    struct Sphere {
        double x, y, z;
        double radius;
        std::vector<Sphere> children;
    };

    InnerSphereTree() = default;

    double overlapVolume(const Sphere& A, const Sphere& B) const {
        double dx = A.x - B.x;
        double dy = A.y - B.y;
        double dz = A.z - B.z;
        double distance_squared = dx * dx + dy * dy + dz * dz;
        double r1_squared = A.radius * A.radius;
        double r2_squared = B.radius * B.radius;

        if (distance_squared >= (r1_squared + r2_squared))
            return 0.0;

        double d = sqrt(distance_squared);
        double r1 = A.radius;
        double r2 = B.radius;

        if (d + r2 <= r1)
            return (4.0 / 3.0) * M_PI * r2 * r2 * r2;

        if (d + r1 <= r2)
            return (4.0 / 3.0) * M_PI * r1 * r1 * r1;

        double part1 = r1_squared * acos((d * d + r1_squared - r2_squared) / (2 * d * r1));
        double part2 = r2_squared * acos((d * d + r2_squared - r1_squared) / (2 * d * r2));
        double part3 = 0.5 * sqrt((-d + r1 + r2) * (d + r1 - r2) * (d - r1 + r2) * (d + r1 + r2));
        return (part1 + part2 - part3);
    }

    double estimateOverlapVolume(const Sphere& A, const Sphere& B) const {
        return overlapVolume(A, B);
    }

    double checkVolumeTimeCritical(const Sphere& A, const Sphere& B, double maxTime) const {
        double estOverlap = estimateOverlapVolume(A, B);
        std::queue<std::pair<Sphere, Sphere>> Q;
        Q.push({ A, B });

        while (!Q.empty() && maxTime > 0) {
            std::pair<Sphere, Sphere> pair = Q.front();
            Q.pop();
            Sphere R = pair.first;
            Sphere S = pair.second;

            if (R.radius > 0 && S.radius > 0) {

                estOverlap -= overlapVolume(R, S);

                for (const Sphere& R : R.children) {
                    for (const Sphere& S : S.children) {
                        estOverlap += overlapVolume(R, S);
                        Q.push({ R, S });
                    }
                }
            }

            maxTime--;
        }

        return estOverlap;
    }
};

#endif
