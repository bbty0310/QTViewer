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
    BV* children[8];
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
    fill(begin(children), end(children), nullptr); // 모든 자식 노드를 nullptr로 초기화

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
    auto lengths = maxPoint - minPoint;

    // 분할 기준 축
    double splitX = 0.5 * (minPoint[0] + maxPoint[0]);
    double splitY = 0.5 * (minPoint[1] + maxPoint[1]);
    double splitZ = 0.5 * (minPoint[2] + maxPoint[2]);
   
    vector<Face> childFaces[8];
    int cnt[8] = { 0 };
    // 각 축에 대한 분할
    for (size_t i = 0; i < faces.size(); i++)
    {
        int index = 0;
        if (boxes[i].center()[0] >= splitX) index |= 1;
        if (boxes[i].center()[1] >= splitY) index |= 2;
        if (boxes[i].center()[2] >= splitZ) index |= 4;
        childFaces[index].push_back(faces[i]);
        cnt[index]++;
    }

    // 각 축별로 분할된 그룹을 사용하여 BV 생성
    for (int i = 0; i < 8; i++) {
        if (!childFaces[i].empty() && childFaces[i].size() < faces.size()) {
            children[i] = new BV(childFaces[i], level + 1, mesh);
        }
    }
}

// 노드(BV)가 리프인지 검사하는 함수
inline bool BV::IsLeaf()
{
    return all_of(begin(children), end(children), [](BV* child) { return child == nullptr; });
}

// BVH 생성자
inline BVH::BVH(vector<Face> allFaces, SurfaceMesh& mesh)
{
    roots.push_back(BV(allFaces, 0, mesh));
}
