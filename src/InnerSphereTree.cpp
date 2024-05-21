#include <iostream>
#include <vector>
#include <cmath>
#define M_PI 3.141592

struct Sphere {
    double x, y, z; // Center coordinates
    double radius;  // Radius
};

struct Node {
    Sphere sphere;
    std::vector<Node*> children;
};

double overlapVolume(const Sphere& A, const Sphere& B) {
    double distance = sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2) + pow(A.z - B.z, 2));
    if (distance >= A.radius + B.radius)
        return 0.0;
    else if (distance <= fabs(A.radius - B.radius)) {
        double smallerVolume = (A.radius < B.radius) ? (4.0 / 3.0 * M_PI * pow(A.radius, 3)) : (4.0 / 3.0 * M_PI * pow(B.radius, 3));
        return smallerVolume;
    }
    else {
        double a = A.radius * A.radius;
        double b = B.radius * B.radius;
        double x = (a - b + distance * distance) / (2 * distance);
        double z = x * x;
        double y = sqrt(a - z);
        double smallerVolume = (a * asin(y / A.radius)) + (b * asin(y / B.radius)) - (y * (x + sqrt(z + b - a)));
        return smallerVolume;
    }
}

Node* buildInnerSphereTree(const std::vector<Sphere>& spheres) {
    if (spheres.empty()) return nullptr;

    Node* root = new Node();
    root->sphere = spheres[0];

    for (size_t i = 1; i < spheres.size(); ++i) {

        if (overlapVolume(root->sphere, spheres[i]) > 0) {
            if (root->children.empty()) {
                root->children.push_back(new Node());
                root->children[0]->sphere = spheres[i];
            }
            else {
                Node* child = buildInnerSphereTree({ root->children[0]->sphere, spheres[i] });
                root->children.clear();
                root->children.push_back(child);
            }
        }
    }
    return root;
}

void computeVolume(Node* root, double& totalOverlap) {
    if (!root) return;

    if (root->children.empty()) {
        totalOverlap += overlapVolume(root->sphere, root->sphere);
    }
    else {
        for (const auto& child : root->children) {
            computeVolume(child, totalOverlap);
        }
    }
}
