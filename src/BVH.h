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
    BV* child[8];
    BV(vector<Face> faces, int lv, SurfaceMesh& mesh);
    bool IsLeaf();
    ~BV();  // 소멸자 추가
};

// BVH 트리
class BVH
{
public:
    vector<BV*> roots;

    BVH(vector<Face> allFaces, SurfaceMesh& mesh);
    ~BVH();  // 소멸자 추가
};

// BV 생성자
BV::BV(vector<Face> fcs, int lv, SurfaceMesh& mesh) {
    faces = fcs;
    level = lv;
    for (int i = 0; i < 8; ++i) child[i] = nullptr;

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

    // 분할 기준 중심점
    Point center = box.center();

    vector<Face> groups[8];

    for (size_t i = 0; i < faces.size(); i++)
    {
        BoundingBox faceBox = boxes[i];
        Point faceCenter = faceBox.center();

        int index = 0;
        if (faceCenter[0] > center[0]) index |= 1;
        if (faceCenter[1] > center[1]) index |= 2;
        if (faceCenter[2] > center[2]) index |= 4;

        groups[index].push_back(faces[i]);
    }

    for (int i = 0; i < 8; ++i)
    {
        if (!groups[i].empty())
        {
            child[i] = new BV(groups[i], level + 1, mesh);
        }
    }
}

// 노드(BV)가 리프인지 검사하는 함수
bool BV::IsLeaf()
{
    for (int i = 0; i < 8; i++)
    {
        if (child[i] != nullptr) return false;
    }
    return true;
}

// BV 소멸자
BV::~BV() {
    for (int i = 0; i < 8; ++i) {
        delete child[i];
    }
}

// BVH 생성자
inline BVH::BVH(vector<Face> allFaces, SurfaceMesh& mesh)
{
    roots.push_back(new BV(allFaces, 0, mesh));  // BV 객체 대신 BV* 포인터를 저장
}

// BVH 소멸자
inline BVH::~BVH() {
    for (auto root : roots) {
        delete root;
    }
}

// 중점을 계산하고 출력하는 재귀 함수
void PrintCenters(BV* node)
{
    if (node == nullptr) return;

    // 현재 노드의 중점 출력
    Point center = node->box.center();
    std::cout << "Level: " << node->level << " Center: ("
        << center[0] << ", " << center[1] << ", " << center[2] << ")" << std::endl;

    // 자식 노드들의 중점 재귀적으로 계산 및 출력
    for (int i = 0; i < 8; ++i) {
        if (node->child[i] != nullptr) {
            PrintCenters(node->child[i]);
        }
    }
}