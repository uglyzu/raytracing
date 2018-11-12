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
#include <iomanip>
#include "omp.h"
#include <vector>
#include "lodepng.h"
#include "texture.h"
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
time_t start, stop;

hitable *random_scene() {
	int n = 5000;
	hitable **list = new hitable *[n + 1];
	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	/*�ȴ���һ�������ڣ�0��-1000��0���뾶Ϊ1000�ĳ��������򣬽���ָ�뱣����list�ĵ�һ��Ԫ���С�*/
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			/*����forѭ���л������11+11��*(11+11)=484�����С��*/
			float choose_mat = drand48();
			/*����һ����0��1�������������Ϊ����С����ϵķ�ֵ*/
			vec3 center(a + 0.9*drand48(), 0.2, b + 0.9*drand48());
			/*�� a+0.9*(rand()%(100)/(float)(100))�����[-11,11]������-11��11��֮����������������[-11,11)֮���22��������ʹ�����ĵ�x,z�����ǣ�-11��11��֮��������*/
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				/*����С���λ�ú���ǰ��Ĵ����λ��̫����*/
				if (choose_mat < 0.8) {     //diffuse  
					/*���Ϸ�ֵС��0.8��������Ϊ�����������������˥��ϵ��x,y,z���ǣ�0��1��֮����������ƽ��*/
					list[i++] = new moving_sphere(center, center+vec3(0, 0.5*drand48(), 0), 0.0, 1.0, 0.2,
						new lambertian(new constant_texture(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48()))));
				}
				else if (choose_mat < 0.95) {
					/*���Ϸ�ֵ���ڵ���0.8С��0.95��������Ϊ���淴���򣬾��淴�����˥��ϵ��x,y,z��ģ��ϵ�����ǣ�0��1��֮����������һ�ٳ���2*/
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5*(1 + drand48()), 0.5*(1+drand48()), 0.5*(1+drand48())), 
							0.5*drand48()
						)
					);
				}
				else {
					/*���Ϸ�ֵ���ڵ���0.95��������Ϊ������*/
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
	/*������������*/
	return new bvh_node(list, i, 0.0, 1.0);
}

hitable *two_sphere(char** name) {
	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
	int n = 50;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
	list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));
	*name = "img//two_sphere.png";
	return new hitable_list(list, 2);
}

hitable *two_perlin_sphere(char** name) {
	texture *pertext = new noise_texture(5);

	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	*name = "img//two_perlin_sphere_turb.png";
	return new hitable_list(list, 2);
}

hitable *one_image_sphere(char** name) {

	int nx, ny, nn;
	unsigned char*tex_data = stbi_load("img//test.jpg", &nx, &ny, &nn, 0);
	texture *imgtext = new image_texture(tex_data, nx, ny);
	printf("image nx=%d, ny=%d\n", nx, ny);
	//material * mat = new lambertian(imgtext);

	hitable **list = new hitable*[1];
	list[0] = new sphere(vec3(0, 0, 0), 3, new lambertian(imgtext));
	*name = "img//one_image_sphere.png";
	return new hitable_list(list, 1);
}

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.01, (numeric_limits<float>::max)(), rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			//return attenuation*color(scattered, world, depth + 1);
			return attenuation;
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
	unsigned error = lodepng::encode(filename, image, width, height);
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
	char* fileName = "img//no_name.png";
	hitable *world = one_image_sphere(&fileName);
	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float dist_to_focus = 10;
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

			image[4 * nx * (ny - j-1) + 4 * i + 0] = int(255.99*col[0]);
			image[4 * nx * (ny - j-1) + 4 * i + 1] = int(255.99*col[1]);
			image[4 * nx * (ny - j-1) + 4 * i + 2] = int(255.99*col[2]);
			image[4 * nx * (ny - j-1) + 4 * i + 3] = 255;
		}
	}

	encodeOneStep(fileName, image, nx, ny);
		
	//ppmfile.close();
	stop = time(NULL);
	printf("Use Time:%ld\n", (stop - start));
	system("pause");
}