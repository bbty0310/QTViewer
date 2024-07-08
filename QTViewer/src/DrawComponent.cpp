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

#define RATIO 0.65	// 임계지점 측정 시 전체 레벨의 비율 (실험적 조정)
#define LINE_WIDTH 10.0	// Wireframe 렌더링 시 그려지는 선의 굵기 (실험적 조정)
#define POINT_SIZE 5.0	// Point Cloud 렌더링 시 그려지는 점의 크기 (실험적 조정)

using namespace std;
using namespace pmp;

BVH* bvh = nullptr;
vector<Face> faces;
int lv;
int currentLevel;
int nodeCount;

// 트리의 전체 노드 개수 계산 함수
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

// 트리의 전체 높이 계산 함수
int maxHeight(BVH& bvh)
{
	int maxHeight = 0;

	// 각 루트 노드마다 트리의 높이를 계산하고 가장 큰 값을 선택
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
//// 트리의 전체 높이 계산 함수
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

void DrawWireAABB(BoundingBox& box)
{
	Point minPoint = box.min();
	Point maxPoint = box.max();

	glLineWidth(LINE_WIDTH);

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

// AABB Drawing
void DrawSolidAABB(BoundingBox& box)
{
	Point minPoint = box.min();
	Point maxPoint = box.max();

	// 아랫면 Drawing
	glBegin(GL_POLYGON);
	glVertex3d(maxPoint[0], minPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], minPoint[1], minPoint[2]);
	glEnd();

	// 윗면 Drawing
	glBegin(GL_POLYGON);
	glVertex3d(minPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], minPoint[2]);
	glVertex3d(maxPoint[0], maxPoint[1], maxPoint[2]);
	glVertex3d(minPoint[0], maxPoint[1], maxPoint[2]);
	glEnd();


	// 옆면 Drawing
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

	// 1) 선 집합으로 렌더링
	//gluQuadricDrawStyle(quad, GLU_LINE);

	// 2) 선 집합으로 렌더링(평면을 구분하는 가장자리가 그려지지 않음)
	//gluQuadricDrawStyle(quad, GLU_SILHOUETTE);

	// 3) 포인트 집합으로 렌더링
	gluQuadricDrawStyle(quad, GLU_POINT);

	// 4) 다각형 기본 형식으로 렌더링
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
		// 10% 확률로 점 생성
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
//// mesh를 감싸는 Bounding Volume 그리기
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
//		// lv 레벨 노드를 Drawing
//		if (bv->level == lv)
//		{
//			BoundingBox bvBox = bv->box;
//			//DrawMesh(bv, mesh, normals);
//
//			// 왼쪽 자식 BV Drawing
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
//			// 오른쪽 자식 BV Drawing
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
//			// 가운데 자식 BV Drawing
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

	// 루트 노드를 큐에 추가
	for (auto& bv : bvh->roots)
	{
		q.push(&bv);
	}

	while (!q.empty()) {
		auto* bv = q.front();
		q.pop();

		// 루트 노드 레벨이 0인 경우
		if (bv->level == 0)
		{
			glPointSize(20.0f);
			// randomPoint(bv);
		}

		// 현재 노드의 레벨이 주어진 레벨과 일치하는 경우
		if (bv->level == lv)
		{
			BoundingBox bvBox = bv->box;
			//DrawMesh(bv, mesh, normals);

			int childCount = 0;
			for (BV* child : bv->children) {
				if (child != nullptr) {
					if (childCount < 2) { // 최대 자식 출력
						// DrawWireAABB(child->box);
						// DrawSolidAABB(child->box); 
						// DrawSphere(child->box); 
						 DrawMesh(child, mesh, normals);
						q.push(child);
						childCount++; // 추가된 자식 개수 증가
					}
					else { // 나머지 자식은 무시
						break; 
					}
				}
			}
		}
		else
		{
			// 현재 노드의 레벨이 주어진 레벨과 일치하지 않는 경우, 자식 노드를 큐에 추가
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

	cout << "트리의 전체 노드 개수 : " << nodeCount << endl;
	cout << "트리의 전체 높이 : " << lv << endl;
	cout << "현재 레벨 : " << currentLevel << endl;
}

void DrawComponent::Draw()
{
	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");

	DrawBVOCbyLevel(bvh, mesh, normals, currentLevel);
}