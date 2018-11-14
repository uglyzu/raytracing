#ifndef MATERIALH
#define MATERIALH

#include "hitable.h"
#include "texture.h"


vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0*vec3(rand() % 100 / float(100), rand() % 100 / float(100), rand() % 100 / float(100)) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1);
	return p;
}

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& albedo, ray& scattered, float& pdf)const { 
		return false; }
	virtual float scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)const {
		return false;
	}
	virtual vec3 emitted(float u, float v, const vec3& p)const {
		return vec3(0, 0 ,0);
	}
};


class lambertian :public material {
public:
	lambertian(texture* a) :albedo(a) {}
	float scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)const {
		float cosine = dot(rec.normal, unit_vector(scattered.direction()));
		if (cosine < 0) cosine = 0;
		return cosine/M_PI;
	}
	bool scatter(const ray& r_in, const hit_record& rec, vec3& alb, ray& scattered, float& pdf)const {
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, unit_vector(target - rec.p), r_in.time());
		alb = albedo->value(rec.u, rec.v, rec.p);
		pdf = dot(rec.normal, scattered.direction()) / M_PI;
		return true;
	}
	texture *albedo;
};

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n)*n;
}

class metal :public material {
public:
	metal(const vec3& aa, float f) :albedo(aa), fuzz(f) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered)const {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

	vec3 albedo;
	float fuzz;
};

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);
	float dt = dot(uv, n);
	float discriminat = 1.0 - ni_over_nt*ni_over_nt*(1 - dt*dt);
	if (discriminat > 0) {
		refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminat);
		return true;
	}
	else {
		return false;
	}
}

float schlick(float cosine, float ref_idx) {

	/*���������ʵ��Schlick's approximation������ref_idx=n2/n1*/
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0*r0;
	return r0 + (1 - r0)*pow((1 - cosine), 5);
}

class dielectric :public material {
public:
	dielectric(float ri) :ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered)const {
		vec3 outward_normal;
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = vec3(1.0, 1.0, 1.0);
		vec3 refracted;
		float reflect_prob;
		float cosine;
		if (dot(r_in.direction(), rec.normal) > 0) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
			cosine = ref_idx*dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			//scattered = ray(rec.p, refracted);
			reflect_prob = schlick(cosine, ref_idx);
		}
		else {
			scattered = ray(rec.p, reflected);
			reflect_prob = 1.0;
			//return false;
		}
		if ((rand() % (100) / (float)(100)) < reflect_prob) {
			scattered = ray(rec.p, reflected);
		}
		else {
			scattered = ray(rec.p, refracted);
		}
		return true;
	}
	float ref_idx;
};

class diffuse_light :public material {
public:
	diffuse_light(texture* a):emit(a){}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered)const { return false; }
	virtual vec3 emitted(float u, float v, const vec3& p)const {
		return emit->value(u, v, p);
	}
	texture* emit;
};

class isotropic : public material {
public:
	isotropic(texture *a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		scattered = ray(rec.p, random_in_unit_sphere());
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}
	texture *albedo;
};

#endif // !MATERIALH

