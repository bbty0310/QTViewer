#include "DrawComponent.h"
#include "pmp/io/io.h"
#include "pmp/bounding_box.h"
#include "pmp/algorithms/normals.h"
#include "GL/glut.h"
#include "GL/freeglut.h"
#include "pmp/types.h"
#include <vector>
#include <cmath>
#include <queue>
#include "Octree.h"

using namespace std;
using namespace pmp;

#define RATIO 0.75

Octree* octree = nullptr;
vector<Face> faces;
int maxLevel;
int currentLevel;

int treeHeight(OctreeNode* node)
{
	if (node->IsLeaf())
	{
		return 0;
	}
	else
	{
		int maxChildHeight = 0;
		for (int i = 0; i < 8; ++i)
		{
			if (node->children[i] != nullptr)
			{
				int childHeight = treeHeight(node->children[i]);
				maxChildHeight = max(maxChildHeight, childHeight);
			}
		}
		return 1 + maxChildHeight;
	}
}

int maxHeight(Octree& octree)
{
	return treeHeight(octree.root);
}

//int maxHeight(BVH& bvh)
//{
//	int maxHeight = 0;
//
//	// 각 루트 노드마다 트리의 높이를 계산하고 가장 큰 값을 선택
//	for (size_t i = 0; i < bvh.roots.size(); ++i)
//	{
//		int height = treeHeight(bvh.roots[i]);
//		maxHeight = max(maxHeight, height);
//	}
//
//	return maxHeight;
//}


// AABB Drawing
void DrawWireAABB(BoundingBox& box)
{
	Point minPoint = box.min();
	Point maxPoint = box.max();

	//glLineWidth(0.5f);

	// 아랫면 Drawing
	glBegin(GL_LINE_LOOP);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glEnd();

	// 윗면 Drawing
	glBegin(GL_LINE_LOOP);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glEnd();

	// 옆면 Drawing
	glBegin(GL_LINE_LOOP);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glEnd();
}

void DrawMesh(OctreeNode* node, SurfaceMesh& mesh, auto& normals)
{
	for (auto& f : node->faces)
	{
		glBegin(GL_TRIANGLES);
		for (auto v : mesh.vertices(f))
		{
			auto& p = mesh.position(v);
			auto& n = normals[v];
			glNormal3dv(n.data());
			glVertex3dv(p.data());
		}
		glEnd();
	}
}

//void DrawMesh(BV*& bv, SurfaceMesh& mesh, auto& normals)
//{
//	for (auto& f : bv->faces)
//	{
//		glBegin(GL_TRIANGLES);
//		for (auto v : mesh.vertices(f))
//		{
//			auto& p = mesh.position(v);
//			auto& n = normals[v];
//			glNormal3dv(n.data());
//			glVertex3dv(p.data());
//		}
//		glEnd();
//	}
//}

//void DrawBVbyLevel(BVH* bvh, SurfaceMesh& mesh, auto& normals, int lv)
//{
//	std::queue<BV*> q;
//
//	for (auto& bv : bvh->roots)
//	{
//		q.push(&bv);
//	}
//
//	while (!q.empty()) {
//		auto& bv = q.front();
//
//		// lv 레벨 노드를 Drawing
//		if (bv->level == lv || lv < 0)
//		{
//			BoundingBox bvBox = bv->box;
//
//			// mesh 그리기
//			DrawMesh(bv, mesh, normals);
//
//			//DrawWireAABB(bvBox);
//
//		}
//
//		if (bv->left_ != nullptr)
//			q.push(bv->left_);
//		if (bv->right_ != nullptr)
//			q.push(bv->right_);
//
//		// 3진 트리를 쓸 경우
//		/*if (bv->mid_ != nullptr)
//			q.push(bv->mid_);*/
//
//		q.pop();
//
//	}
//}

void DrawOctreeByLevel(OctreeNode* node, SurfaceMesh& mesh, auto& normals, int targetLevel, int currentLevel)
{
	if (node == nullptr) return;

	if (currentLevel == targetLevel || targetLevel < 0)
	{
		DrawMesh(node, mesh, normals);
		DrawWireAABB(node->box);
	}

	if (currentLevel < targetLevel || targetLevel < 0)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (node->children[i] != nullptr)
			{
				DrawOctreeByLevel(node->children[i], mesh, normals, targetLevel, currentLevel + 1);
			}
		}
	}
}

void DrawComponent::Init()
{
	// load the model
	pmp::read(mesh, "models/bunny_stanford.obj");
	pmp::vertex_normals(mesh);

	std::cout << "#f " << mesh.n_faces() << " #v " << mesh.n_vertices() << std::endl;

	for (auto f : mesh.faces())
	{
		faces.push_back(f);
	}
	octree = new Octree(faces, mesh);
	maxLevel = maxHeight(*octree);
	currentLevel = maxLevel * RATIO;
}

void DrawComponent::Draw()
{
	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");

	DrawOctreeByLevel(octree->root, mesh, normals, currentLevel, 0);
}

//void DrawComponent::Init()
//{
//	// load the model
//	pmp::read(mesh, "models/bunny_stanford.obj");
//	pmp::vertex_normals(mesh);
//
//	std::cout << "#f " << mesh.n_faces() << " #v " << mesh.n_vertices() << std::endl;
//
//	for (auto f : mesh.faces())
//	{
//		faces.push_back(f);
//	}
//	bvh = new BVH(faces, mesh);
//	lv = maxHeight(*bvh);
//	currentLevel = lv * RATIO;
//}
//
//void DrawComponent::Draw()
//{
//	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");
//
//	DrawBVbyLevel(bvh, mesh, normals, currentLevel);
//
//
//
//}

//void DrawComponent::DrawSphere(float level)
//{
//	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");
//
//	sphere = uv_sphere(center, radius * level, 50, 50);
//	
//	for (auto f : sphere.faces())
//	{
//		faces.push_back(f);
//		glBegin(GL_LINE_LOOP);
//		for (auto v : sphere.vertices(f))
//		{
//			auto p = sphere.position(v);
//			auto n = normals[v];
//			glNormal3d(n[0], n[1], n[2]);
//			glVertex3d(p[0], p[1], p[2]);
//
//		}
//		glEnd();
//	}
//
//}