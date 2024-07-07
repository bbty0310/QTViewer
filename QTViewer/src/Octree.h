#pragma once
#include "pmp/types.h"
#include "DrawComponent.h"
#include "pmp/bounding_box.h"
#include "pmp/algorithms/normals.h"

using namespace pmp;
using namespace std;

// �� ���� ��(face)�� AABB�� ���δ� �Լ�
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

// BVH ���
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

// BVH Ʈ��
class BVH
{
private:

public:
    vector<BV> roots;

    BVH(vector<Face> allFaces, SurfaceMesh& mesh);
};

// BV ������
inline BV::BV(vector<Face> fcs, int lv, SurfaceMesh& mesh) {
    faces = fcs;
    level = lv;
    fill(begin(children), end(children), nullptr); // ��� �ڽ� ��带 nullptr�� �ʱ�ȭ

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

    // ���� ���� ��
    double splitX = 0.5 * (minPoint[0] + maxPoint[0]);
    double splitY = 0.5 * (minPoint[1] + maxPoint[1]);
    double splitZ = 0.5 * (minPoint[2] + maxPoint[2]);
   
    vector<Face> childFaces[8];
    int cnt[8] = { 0 };
    // �� �࿡ ���� ����
    for (size_t i = 0; i < faces.size(); i++)
    {
        int index = 0;
        if (boxes[i].center()[0] >= splitX) index |= 1;
        if (boxes[i].center()[1] >= splitY) index |= 2;
        if (boxes[i].center()[2] >= splitZ) index |= 4;
        childFaces[index].push_back(faces[i]);
        cnt[index]++;
    }

    // �� �ະ�� ���ҵ� �׷��� ����Ͽ� BV ����
    for (int i = 0; i < 8; i++) {
        if (!childFaces[i].empty() && childFaces[i].size() < faces.size()) {
            children[i] = new BV(childFaces[i], level + 1, mesh);
        }
    }
}

// ���(BV)�� �������� �˻��ϴ� �Լ�
inline bool BV::IsLeaf()
{
    return all_of(begin(children), end(children), [](BV* child) { return child == nullptr; });
}

// BVH ������
inline BVH::BVH(vector<Face> allFaces, SurfaceMesh& mesh)
{
    roots.push_back(BV(allFaces, 0, mesh));
}
