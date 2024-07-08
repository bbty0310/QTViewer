#include "DrawComponent.h"
#include "gl/freeglut.h"
#include "GL/GLU.h"
//#include "BinaryBVH.h"
//#include "TernaryBVH.h"
//#include "linemesh.h"
#include "Octree.h"
#include "pmp/io/io.h"
#include "pmp/algorithms//normals.h"
#include "pmp/bounding_box.h"
#include "pmp/mat_vec.h"

#include <queue>
#include <random>

#define RATIO 0.65	// �Ӱ����� ���� �� ��ü ������ ���� (������ ����)
#define LINE_WIDTH 10.0	// Wireframe ������ �� �׷����� ���� ���� (������ ����)
#define POINT_SIZE 5.0	// Point Cloud ������ �� �׷����� ���� ũ�� (������ ����)

using namespace std;
using namespace pmp;

BVH* bvh = nullptr;
vector<Face> faces;
int lv;
int currentLevel;
int nodeCount;

// Ʈ���� ��ü ��� ���� ��� �Լ�
//int getNodeCount(BV* bv)
//{
//	int count = 0;
//	if (bv != NULL)
//		//count = 1 + getNodeCount(bv->left_) + getNodeCount(bv->right_);// +getNodeCount(bv->mid_);
//		count = 1 + getNodeCount(bv->child1) + getNodeCount(bv->child2) + getNodeCount(bv->child3) + getNodeCount(bv->child4) + getNodeCount(bv->child5) + getNodeCount(bv->child6) + getNodeCount(bv->child7) + getNodeCount(bv->child8);
//	return count;
//}
int getNodeCount(BV* bv)
{
	int count = 0;
	if (bv != NULL) {
		for (auto child : bv->children) {
			if (child != nullptr) {
				count += getNodeCount(child);
			}
		}
		count += 1;
	}
	return count;
}
int treeHeight(BV& node)
{
	if (node.IsLeaf()) {
		return 0;
	}
	else {
		int maxChildHeight = 0;
		for (auto child : node.children) {
			if (child != nullptr) {
				maxChildHeight = max(maxChildHeight, treeHeight(*child));
			}
		}
		return 1 + maxChildHeight;
	}
}

// Ʈ���� ��ü ���� ��� �Լ�
int maxHeight(BVH& bvh)
{
	int maxHeight = 0;

	// �� ��Ʈ ��帶�� Ʈ���� ���̸� ����ϰ� ���� ū ���� ����
	for (size_t i = 0; i < bvh.roots.size(); ++i) {
		int height = treeHeight(bvh.roots[i]);
		maxHeight = max(maxHeight, height);
	}

	return maxHeight;
}
//
//int treeHeight(BV& node)
//{
//	if (node.IsLeaf())
//	{
//		return 0;
//	}
//	else
//	{
//		int leftHeight = 0;
//		int rightHeight = 0;
//		int midHeight = 0;
//
//		/*if (node.left_ != nullptr)
//		{
//			leftHeight = treeHeight(*node.left_);
//		}
//
//		if (node.right_ != nullptr)
//		{
//			rightHeight = treeHeight(*node.right_);
//		}*/
//
//		/*if (node.mid_ != nullptr)
//		{
//			midHeight = treeHeight(*node.mid_);
//		}*/
//
//		return 1 + max(leftHeight, max(rightHeight, midHeight));
//	}
//}
//
//// Ʈ���� ��ü ���� ��� �Լ�
//int maxHeight(BVH& bvh)
//{
//	int maxHeight = 0;
//
//	// �� ��Ʈ ��帶�� Ʈ���� ���̸� ����ϰ� ���� ū ���� ����
//	for (size_t i = 0; i < bvh.roots.size(); ++i)
//	{
//		int height = treeHeight(bvh.roots[i]);
//		maxHeight = max(maxHeight, height);
//	}
//
//	return maxHeight;
//}

void DrawWireAABB(BoundingBox& box)
{
	Point minPoint = box.min();
	Point maxPoint = box.max();

	glLineWidth(LINE_WIDTH);

	// �Ʒ��� Drawing
	glBegin(GL_LINE_LOOP);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glEnd();

	// ���� Drawing
	glBegin(GL_LINE_LOOP);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glEnd();


	// ���� Drawing
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

// AABB Drawing
void DrawSolidAABB(BoundingBox& box)
{
	Point minPoint = box.min();
	Point maxPoint = box.max();

	// �Ʒ��� Drawing
	glBegin(GL_POLYGON);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glEnd();

	// ���� Drawing
	glBegin(GL_POLYGON);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glEnd();


	// ���� Drawing
	glBegin(GL_POLYGON);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glEnd();
}

// Bounding Sphere Drawing
void DrawSphere(BoundingBox& box)
{
	Point centerPoint = box.center();
	Point maxPoint = box.max();

	double radius = distance(centerPoint, maxPoint);

	GLUquadric* quad = gluNewQuadric();

	glLineWidth(LINE_WIDTH);
	glPointSize(POINT_SIZE);

	// 1) �� �������� ������
	//gluQuadricDrawStyle(quad, GLU_LINE);

	// 2) �� �������� ������(����� �����ϴ� �����ڸ��� �׷����� ����)
	//gluQuadricDrawStyle(quad, GLU_SILHOUETTE);

	// 3) ����Ʈ �������� ������
	gluQuadricDrawStyle(quad, GLU_POINT);

	// 4) �ٰ��� �⺻ �������� ������
	//gluQuadricDrawStyle(quad, GLU_FILL);

	glPushMatrix();
	glTranslated(centerPoint[0], centerPoint[1], centerPoint[2]);
	gluSphere(quad, radius, 15, 15);
	glPopMatrix();

	gluDeleteQuadric(quad);
}

// Mesh Drawing
void DrawMesh(BV*& bv, SurfaceMesh& mesh, auto& normals)
{
	for (auto& f : bv->faces)
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

// Noise Drawing
void DrawRandomNoise(BV*& bv)
{
	BoundingBox box = bv->box;
	Point centerPoint = box.center();
	Point maxPoint = box.max();

	double radius = distance(centerPoint, maxPoint);

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> disTheta(0.0, 2.0 * M_PI);
	uniform_real_distribution<> disPi(0.0, 1.0);
	uniform_real_distribution<> pointProb(0.0, 1.0);
	uniform_real_distribution<> colorProb(0.0, 1.0);

	for (int i = 0; i < 100; i++)
	{
		// 10% Ȯ���� �� ����
		if (pointProb(gen) <= 0.1)
		{
			auto theta = disTheta(gen);
			auto pi = acos(2.0 * disPi(gen) - 1.0);

			auto x = centerPoint[0] + radius * sin(pi) * cos(theta);
			auto y = centerPoint[1] + radius * sin(pi) * sin(theta);
			auto z = centerPoint[2] + radius * cos(pi);

			auto r = colorProb(gen);
			auto g = colorProb(gen);
			auto b = colorProb(gen);

			glBegin(GL_POINTS);
			glColor3f(r, g, b);
			glVertex3f(x, y, z);
		}
	}
	glEnd();
}
//
//// mesh�� ���δ� Bounding Volume �׸���
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
//		if (bv->level == 0)
//		{
//			glPointSize(20.0f);
//			//randomPoint(bv);
//		}
//
//		// lv ���� ��带 Drawing
//		if (bv->level == lv)
//		{
//			BoundingBox bvBox = bv->box;
//			//DrawMesh(bv, mesh, normals);
//
//			// ���� �ڽ� BV Drawing
//			if (bv->left_ != nullptr)
//			{
//				//glColor3f(1, 0, 0);
//				BV* leftBV = bv->left_;
//
//				// 1) Wireframe AABB
//				//DrawWireAABB(leftBV->box);
//
//				// 2) Solid AABB
//				DrawSolidAABB(leftBV->box);
//
//				// 3) Sphere
//				//DrawSphere(leftBV->box);
//
//				// 4) Mesh
//				//DrawMesh(leftBV, mesh, normals);
//
//				q.push(leftBV);
//			}
//
//			// ������ �ڽ� BV Drawing
//			if (bv->right_ != nullptr)
//			{
//				//glColor3f(0, 1, 0);
//				BV* rightBV = bv->right_;
//
//				// 1) Wireframe AABB
//				//DrawWireAABB(rightBV->box);
//
//				// 2) Solid AABB
//				DrawSolidAABB(rightBV->box);
//
//				// 3) Sphere
//				//DrawSphere(rightBV->box);
//
//				// 4) Mesh
//				//DrawMesh(rightBV, mesh, normals);
//
//				q.push(rightBV);
//			}
//
//			// ��� �ڽ� BV Drawing
//			//if (bv->mid_ != nullptr)
//			//{
//			//	glColor3f(0, 0, 1);
//			//	BV* midBV = bv->mid_;
//
//			//	// 1) Wireframe AABB
//			//	DrawWireAABB(midBV->box);
//
//			//	// 2) Solid AABB
//			//	//DrawSolidAABB(midBV->box);
//
//			//	// 3) Sphere
//			//	//DrawSphere(midBV->box);
//
//			//	// 4) Mesh
//			//	//DrawMesh(midBV, mesh, normals);
//
//			//	q.push(midBV);
//			//}
//		}
//		else
//		{
//			if (bv->left_ != nullptr)
//				q.push(bv->left_);
//			if (bv->right_ != nullptr)
//				q.push(bv->right_);
//			/*if (bv->mid_ != nullptr)
//				q.push(bv->mid_);*/
//		}
//
//		q.pop();
//
//	}
//}
void DrawBVOCbyLevel(BVH* bvh, SurfaceMesh& mesh, auto& normals, int lv)
{
	std::queue<BV*> q;

	// ��Ʈ ��带 ť�� �߰�
	for (auto& bv : bvh->roots)
	{
		q.push(&bv);
	}

	while (!q.empty()) {
		auto* bv = q.front();
		q.pop();

		// ��Ʈ ��� ������ 0�� ���
		if (bv->level == 0)
		{
			glPointSize(20.0f);
			// randomPoint(bv);
		}

		// ���� ����� ������ �־��� ������ ��ġ�ϴ� ���
		if (bv->level == lv)
		{
			BoundingBox bvBox = bv->box;
			//DrawMesh(bv, mesh, normals);

			int childCount = 0;
			for (BV* child : bv->children) {
				if (child != nullptr) {
					if (childCount < 2) { // �ִ� �ڽ� ���
						// DrawWireAABB(child->box);
						// DrawSolidAABB(child->box); 
						// DrawSphere(child->box); 
						 DrawMesh(child, mesh, normals);
						q.push(child);
						childCount++; // �߰��� �ڽ� ���� ����
					}
					else { // ������ �ڽ��� ����
						break; 
					}
				}
			}
		}
		else
		{
			// ���� ����� ������ �־��� ������ ��ġ���� �ʴ� ���, �ڽ� ��带 ť�� �߰�
			for (BV* child : bv->children) {
				if (child != nullptr) {
					q.push(child);
				}
			}
		}
	}
}





void DrawComponent::Init()
{
	// load the model
	pmp::read(mesh, "models\\bunny_stanford.obj");
	pmp::vertex_normals(mesh);

	auto nf = mesh.n_faces();
	auto nv = mesh.n_vertices();
	std::cout << "#f " << nf << " #v " << nv << std::endl;

	for (auto f : mesh.faces())
	{
		faces.push_back(f);
	}

	bvh = new BVH(faces, mesh);
	lv = maxHeight(*bvh);
	currentLevel = lv * RATIO;

	cout << "Ʈ���� ��ü ��� ���� : " << nodeCount << endl;
	cout << "Ʈ���� ��ü ���� : " << lv << endl;
	cout << "���� ���� : " << currentLevel << endl;
}

void DrawComponent::Draw()
{
	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");

	DrawBVOCbyLevel(bvh, mesh, normals, currentLevel);
}