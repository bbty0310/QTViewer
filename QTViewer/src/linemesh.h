#pragma once
#include "pmp/types.h"
#include "DrawComponent.h"
#include "pmp/bounding_box.h"
#include "pmp/algorithms/normals.h"

using namespace pmp;
using namespace std;



// 한 개의 면(face)을 AABB로 감싸는 함수
BoundingBox AABB(Face face, SurfaceMesh& mesh)
{
    BoundingBox box;
    for (auto v : mesh.vertices(face))
    {
        auto& p = mesh.position(v);
        box += p;
    }
    return box;
}

// BVH 노드
class BV
{
public:
    int level;
    BoundingBox box;
    vector<BoundingBox> boxes;
    vector<Face> faces;
    BV* left_, * right_;
    BV(vector<Face> faces, int lv, SurfaceMesh& mesh);
    bool IsLeaf();
};

// BVH 트리
class BVH
{
private:

public:
    vector<BV> roots;

    BVH(vector<Face> allFaces, SurfaceMesh& mesh);
};

// BV 생성자
inline BV::BV(vector<Face> fcs, int lv, SurfaceMesh& mesh) {
    faces = fcs;
    level = lv;
    left_ = nullptr;
    right_ = nullptr;

    Point minPoint = Point(FLT_MAX, FLT_MAX, FLT_MAX);
    Point maxPoint = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (auto& face : faces)
    {
        BoundingBox faceBox = AABB(face, mesh);

        minPoint = pmp::min(faceBox.min(), minPoint);
        maxPoint = pmp::max(faceBox.max(), maxPoint);
        boxes.push_back(faceBox);
    }

    box = BoundingBox(minPoint, maxPoint);

    // 분할 기준 축
    double splitX = 0.5 * (minPoint[0] + maxPoint[0]);
    double splitY = 0.5 * (minPoint[1] + maxPoint[1]);
    double splitZ = 0.5 * (minPoint[2] + maxPoint[2]);

    vector<Face> leftFacesX, rightFacesX;
    vector<Face> leftFacesY, rightFacesY;
    vector<Face> leftFacesZ, rightFacesZ;

    // x축에 대한 분할
    for (size_t i = 0; i < faces.size(); i++)
    {
        if (boxes[i].max()[0] < splitX)
        {
            leftFacesX.push_back(faces[i]);
        }
        else
        {
            rightFacesX.push_back(faces[i]);
        }
    }

    // y축에 대한 분할
    for (size_t i = 0; i < faces.size(); i++)
    {
        if (boxes[i].max()[1] < splitY)
        {
            leftFacesY.push_back(faces[i]);
        }
        else
        {
            rightFacesY.push_back(faces[i]);
        }
    }

    // z축에 대한 분할
    for (size_t i = 0; i < faces.size(); i++)
    {
        if (boxes[i].max()[2] < splitZ)
        {
            leftFacesZ.push_back(faces[i]);
        }
        else
        {
            rightFacesZ.push_back(faces[i]);
        }
    }

    // 각 축별로 분할된 그룹을 사용하여 BV 생성
    if (!leftFacesY.empty() && !rightFacesY.empty())
    {
        // y 축에 대한 분할
        left_ = new BV(leftFacesY, level + 1, mesh);
        right_ = new BV(rightFacesY, level + 1, mesh);
     }
    
    
    else if (!leftFacesZ.empty() && !rightFacesZ.empty())
    {
        // z 축에 대한 분할
        left_ = new BV(leftFacesZ, level + 1, mesh);
        right_ = new BV(rightFacesZ, level + 1, mesh);
    }
    else if (!leftFacesX.empty() && !rightFacesX.empty())
    {
        // x 축에 대한 분할
        left_ = new BV(leftFacesX, level + 1, mesh);
        right_ = new BV(rightFacesX, level + 1, mesh);
    }
    
}

// 노드(BV)가 리프인지 검사하는 함수
inline bool BV::IsLeaf()
{
    return left_ == nullptr && right_ == nullptr;
}

// BVH 생성자
inline BVH::BVH(vector<Face> allFaces, SurfaceMesh& mesh)
{
    roots.push_back(BV(allFaces, 0, mesh));
}