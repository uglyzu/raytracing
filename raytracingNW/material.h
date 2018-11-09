#ifndef MATERIALH
#define MATERIALH

#include "hitable.h"

vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0*vec3(rand() % 100 / float(100), rand() % 100 / float(100), rand() % 100 / float(100)) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1);
	return p;
}

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered)const = 0;
};

#endif // !MATERIALH

