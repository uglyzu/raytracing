#include <iostream>
#include <fstream>
#include "moving_sphere.h"
#include "sphere.h"
#include "hitable_list.h"
#include "bvh_node.h"
#include <limits>  
#include "float.h"
#include "vec3.h"
#include "ray.h"
#include "camera.h"
#include "lambertian.h"
#include "metal.h"
#include "dielectic.h"
#include <iomanip>
#include "omp.h"
#include <vector>
#include "lodepng.h"

using namespace std;
time_t start, stop;

hitable *random_scene() {
	int n = 5000;
	hitable **list = new hitable *[n + 1];
	/*定义一个包含n+1个元素的数组，数组的每个元素是指向hitable对象的指针。然后将数组的指针赋值给list。所以，list是指针的指针。*/
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	/*先创建一个中心在（0，-1000，0）半径为1000的超大漫射球，将其指针保存在list的第一个元素中。*/
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			/*两个for循环中会产生（11+11）*(11+11)=484个随机小球*/
			float choose_mat = drand48();
			/*产生一个（0，1）的随机数，作为设置小球材料的阀值*/
			vec3 center(a + 0.9*drand48(), 0.2, b + 0.9*drand48());
			/*” a+0.9*(rand()%(100)/(float)(100))”配合[-11,11]产生（-11，11）之间的随机数，而不是[-11,11)之间的22个整数。使得球心的x,z坐标是（-11，11）之间的随机数*/
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				/*避免小球的位置和最前面的大球的位置太靠近*/
				if (choose_mat < 0.8) {     //diffuse  
					/*材料阀值小于0.8，则设置为漫反射球，漫反射球的衰减系数x,y,z都是（0，1）之间的随机数的平方*/
					list[i++] = new moving_sphere(center, center+vec3(0, 0.5*drand48(), 0), 0.0, 1.0, 0.2,
						new lambertian(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48())));
				}
				else if (choose_mat < 0.95) {
					/*材料阀值大于等于0.8小于0.95，则设置为镜面反射球，镜面反射球的衰减系数x,y,z及模糊系数都是（0，1）之间的随机数加一再除以2*/
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5*(1 + drand48()), 0.5*(1+drand48()), 0.5*(1+drand48())), 
							0.5*drand48()
						)
					);
				}
				else {
					/*材料阀值大于等于0.95，则设置为介质球*/
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
	/*定义三个大球*/
	return new bvh_node(list, i, 0.0, 1.0);
}

hitable *random_scene2() {
	int n = 5;
	hitable **list = new hitable *[n + 1];
	int i = 1;
	
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
	printf("random_scene2 i = %d\n", i);
	return new bvh_node(list, i, 0.0, 1.0);
}

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.01, (numeric_limits<float>::max)(), rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation*color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
	}
}

void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

int main() {
	start = time(NULL);
	const int nx = 200;
	const int ny = 100;
	int ns = 100;
	std::vector<unsigned char> image;
	image.resize(nx * ny * 4);
	//ofstream ppmfile("img\\12\\2.ppm");
	//ppmfile << "P3\n" << nx << " " << ny << "\n255\n";

	hitable *world = random_scene();
	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float dist_to_focus = (lookfrom - lookat).length();
	float aperture = 0;
	camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus,0.0, 1.0);

#pragma omp parallel for
	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++)
			{
				float random = rand() % (100) / (float)(100);
				float u = float(i + random) / float(nx);
				float v = float(j + random) / float(ny);
				ray r = cam.get_ray(u, v);
				//vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			//int ir = int(255.99*col[0]);
			//int ig = int(255.99*col[1]);
			//int ib = int(255.99*col[2]);
			//ppmfile << ir << " " << ig << " " << ib << "\n" << endl;
			image[4 * nx * (ny - j-1) + 4 * i + 0] = int(255.99*col[0]);
			image[4 * nx * (ny - j-1) + 4 * i + 1] = int(255.99*col[1]);
			image[4 * nx * (ny - j-1) + 4 * i + 2] = int(255.99*col[2]);
			image[4 * nx * (ny - j-1) + 4 * i + 3] = 255;
		}
	}

	encodeOneStep("img//01.png", image, nx, ny);
		
	//ppmfile.close();
	stop = time(NULL);
	printf("Use Time:%ld\n", (stop - start));
	system("pause");
}