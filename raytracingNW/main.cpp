#include <iostream>
#include <fstream>
#include "moving_sphere.h"
#include "sphere.h"
#include "box.h"
#include "translate.h"
#include "constant_medium.h"
#include "material.h"
#include "hitable.h"
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
vec3 lookfrom(278, 278, -800);
vec3 lookat(278, 278, 0);
float vfov = 40.0;
float dist_to_focus = 10;
float aperture = 0;
bool SunLight = true;

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.01, (numeric_limits<float>::max)(), rec)) {
		ray scattered;
		vec3 attenuation;
		vec3 emmitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		float pdf;
		vec3 albedo;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf)) {
			//return attenuation*color(scattered, world, depth + 1);
			//return attenuation;
			//return emmitted + attenuation*color(scattered, world, depth + 1);
			return emmitted + albedo*rec.mat_ptr->scattering_pdf(r, rec, scattered)*color(scattered, world, depth + 1)/pdf;
		}
		else {
			return emmitted;
		}
	}
	else {
		return vec3(0, 0, 0);
	}
}

void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
	unsigned error = lodepng::encode(filename, image, width, height);
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

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

hitable *simple_light(char** name) {
	texture *pertext = new noise_texture(4);

	hitable **list = new hitable*[4];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	list[2] = new sphere(vec3(0, 7, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3,5,1,3,-2, new diffuse_light(new constant_texture(vec3(4,4,4))));

	*name = "img//simple_light.png";
	return new hitable_list(list, 4);
}

hitable *cornell_box(char** name) {
	texture *pertext = new noise_texture(4);

	hitable **list = new hitable*[6];
	int i = 0;
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material *light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));

	list[i++] = new flip_normal(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
	list[i++] = new flip_normal(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normal(new xy_rect(0, 555, 0, 555, 555, white));

	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295)); 
	*name = "img3//cornell_box01.png";
	lookfrom = vec3(278, 278, -800);
	lookat = vec3(278, 278, 0);
	vfov = 40.0;
	dist_to_focus = 10;
	aperture = 0;
	return new hitable_list(list, i);
}

hitable *volume(char** name) {
	texture *pertext = new noise_texture(4);

	hitable **list = new hitable*[8];
	int i = 0;
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material *light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));

	list[i++] = new flip_normal(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(113, 443, 127, 432, 554, light);
	list[i++] = new flip_normal(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normal(new xy_rect(0, 555, 0, 555, 555, white));

	hitable*b1 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	hitable*b2 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));
	list[i++] = new constant_medium(b1, 0.01, new constant_texture(vec3(1.0, 1.0, 1.0)));
	list[i++] = new constant_medium(b2, 0.01, new constant_texture(vec3(0.0, 0.0, 0.0)));
	*name = "img//volume.png";
	return new hitable_list(list, i);
}

hitable *Final(char** name) {
	lookfrom = vec3(478, 278, -600);
	lookat = vec3(278, 278, 0);
	SunLight = false;
	vfov = 40;
	int nb = 20;
	hitable **list = new hitable*[30];
	hitable **boxlist = new hitable*[10000];
	hitable **boxlist2 = new hitable*[10000];
	material *ground = new lambertian(new constant_texture(vec3(0.48, 0.83, 0.53)));
	int b = 0;
	for (int i = 0; i < nb; i++) {
		for (int j = 0; j < nb; j++) {
			float w = 100;
			float x0 = -1000 + i*w;
			float z0 = -1000 + j*w;
			float y0 = 0;
			float x1 = x0 + w;
			float y1 = 100 * (drand48() + 0.01);
			float z1 = z0 + w;
			boxlist[b++] = new box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
		}
	}
	int l = 0;
	list[l++] = new bvh_node(boxlist, b, 0, 1);
	list[l++] = new xz_rect(123, 423, 147, 412, 554, new diffuse_light(new constant_texture(vec3(10, 10, 10))));
	vec3 center(400, 400, 200);
	list[l++] = new moving_sphere(center, center + vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7, 0.3, 0.1))));//�ƶ���
	list[l++] = new sphere(vec3(260, 150, 45), 50, new dielectric(1.5));//������
	list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8, 0.8, 0.9), 10.0));//������

	hitable *boundary = new sphere(vec3(360, 150, 145), 70, new dielectric(1.5));
	list[l++] = boundary;
	list[l++] = new constant_medium(boundary, 0.2, new constant_texture(vec3(0.2, 0.4, 0.9)));//��ɫ��

	//boundary = new sphere(vec3(0, 0, 0), 5000, new dielectric(1.5));
	//list[l++] = new constant_medium(boundary, 0.0001, new constant_texture(vec3(1.0, 1.0, 1.0)));
	int nx, ny, n;
	unsigned char *tex_data = stbi_load("img//timg.jpg", &nx, &ny, &n, STBI_rgb);
	material *emat = new lambertian(new image_texture(tex_data, nx, ny));
	list[l++] = new sphere(vec3(400, 200, 400), 100, emat);//��ͼ��
	list[l++] = new sphere(vec3(220, 280, 300), 80, new lambertian(new noise_texture(4.0)));//������

	//������
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxlist2[j] = new sphere(vec3(165 * drand48(), 165 * drand48(), 165 * drand48()), 10, new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73))));
	}
	list[l++] = new translate(new rotate_y(new bvh_node(boxlist2, ns, 0.0, 1.0), 15), vec3(-100, 270, 395));

	*name = "img//final.png";
	return new bvh_node(list, l, 0.0, 1.0);
}


int main() {
	start = time(NULL);
	const int nx = 500;
	const int ny = 500;
	int ns = 100;
	std::vector<unsigned char> image;
	image.resize(nx * ny * 4);
	//ofstream ppmfile("img\\12\\2.ppm");
	//ppmfile << "P3\n" << nx << " " << ny << "\n255\n";
	char* fileName = "img//no_name.png";
	hitable *world = cornell_box(&fileName);
	
	camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus,0.0, 1.0);

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