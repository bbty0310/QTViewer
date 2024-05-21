#include "pmp/types.h"
#include "pmp/surface_mesh.h"
#include <vector>

#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

struct Sphere {
	pmp::Scalar x, y, z;
	pmp::Scalar radius;
	pmp::Point center;
};


//최대 반지름 계산
pmp::Scalar& find_radius(pmp::SurfaceMesh& mesh)
{
	pmp::Scalar radius = 0;

	for (auto f : mesh.faces())
	{
		// find radius
		for (auto v : mesh.vertices(f)) {

			auto p1 = mesh.position(v);

			float x1 = p1[0];
			float y1 = p1[1];
			float z1 = p1[2];
			for (auto w : mesh.vertices(f)) {
				auto p2 = mesh.position(w);

				float x2 = p2[0];
				float y2 = p2[1];
				float z2 = p2[2];

				pmp::Scalar tmp_length = std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2) + std::pow(z1 - z2, 2)) * 10/ 2.0;


				if (tmp_length > radius)
				{
					radius = tmp_length;
				}

			}

		}

	}
	return radius;
}

//원 중심 계산
pmp::Point& find_center(pmp::SurfaceMesh& mesh)
{
	pmp::Scalar radius = 0;
	pmp::Point center = pmp::Vector<pmp::Scalar,3>(0, 0, 0);
	for (auto f : mesh.faces())
	{
		// find radius
		for (auto v : mesh.vertices(f)) {

			auto p1 = mesh.position(v);

			float x1 = p1[0];
			float y1 = p1[1];
			float z1 = p1[2];
			for (auto w : mesh.vertices(f)) {
				auto p2 = mesh.position(w);

				float x2 = p2[0];
				float y2 = p2[1];
				float z2 = p2[2];

				
				pmp::Scalar tmp_length = std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2) + std::pow(z1 - z2, 2)) * 10 / 2.0;

				if (tmp_length > radius)
				{
					radius = tmp_length;
					center = pmp::Vector<pmp::Scalar, 3>((p1[0] - p2[0]) / 2, (p1[1] - p2[1]) / 2, (p1[2] - p2[2]) / 2);
				}
			}
		}

	}
	return center;
}

//저장 및 출력 level value : 0 ~ 1.7
// level값과 radius의 값 계산 수정할 예정
void loadSphereMesh(pmp::SurfaceMesh& mesh, pmp::VertexProperty<pmp::Point> &normals, pmp::Point center, pmp::Scalar radius, double level)
{
	for (auto f : mesh.faces())
	{
		glBegin(GL_TRIANGLES);

		for (auto v : mesh.vertices(f))
		{
			auto p = mesh.position(v);
			double length = std::sqrt(std::pow(center[0] - p[0], 2) + std::pow(center[1] - p[1], 2) + std::pow(center[2] - p[2], 2));
			if (length <= radius * level)
			{
				auto n = normals[v];
				glNormal3d(n[0], n[1], n[2]);
				glVertex3d(p[0], p[1], p[2]);
			}

		}
		glEnd();

	}
}


#endif
