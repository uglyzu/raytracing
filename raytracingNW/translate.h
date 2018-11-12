#ifndef TRANSLATE
#define TRANSLATE

#include "helper.h"
#include "hitable.h"

class translate : public hitable {
public:
	translate() {}
	translate(hitable *p, const vec3& displacement) : ptr(p), offset(displacement) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& aabb_box) const;

	hitable *ptr;
	vec3 offset;
};

bool translate::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	ray moved_r(r.origin() - offset, r.direction(), r.time());

	if (ptr->hit(moved_r, t_min, t_max, rec)) {
		rec.p += offset;
		return true;
	}
	else
		return false;
}

bool translate::bounding_box(float t0, float t1, aabb& aabb_box) const {
	if (ptr->bounding_box(t0, t1, aabb_box)) {
		aabb_box = aabb(aabb_box.min() + offset, aabb_box.max() + offset);
		return true;
	}
	else
		return false;
}

class rotate_y : public hitable {
public:
	rotate_y(hitable *p, float angle);
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const {
		box = aabb_box;
		return hasbox;
	}

	float sin_theta, cos_theta;
	bool hasbox;
	aabb aabb_box;
	hitable *ptr;

};

rotate_y::rotate_y(hitable *p, float angle) : ptr(p) {
	float radians = (M_PI / 180.0)*angle;
	sin_theta = sin(radians);
	cos_theta = cos(radians);
	hasbox = ptr->bounding_box(0, 1, aabb_box);
	vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				float x = i*aabb_box.max().x() + (1 - i)*aabb_box.min().x();
				float y = j*aabb_box.max().y() + (1 - j)*aabb_box.min().y();
				float z = k*aabb_box.max().z() + (1 - k)*aabb_box.min().z();
				float newx = cos_theta*x + sin_theta*z;
				float newz = -sin_theta*x + cos_theta*z;
				vec3 tester(newx, y, newz);
				for (int c = 0; c < 3; c++) {
					if (tester[c] > max[c])
						max[c] = tester[c];
					if (tester[c] < min[c])
						min[c] = tester[c];
				}
			}
		}
	}
	aabb_box = aabb(min, max);
}


bool rotate_y::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	vec3 origin = r.origin();
	vec3 direction = r.direction();
	origin[0] = cos_theta * r.origin()[0] - sin_theta*r.origin()[2];
	origin[2] = sin_theta * r.origin()[0] + cos_theta*r.origin()[2];
	direction[0] = cos_theta * r.direction()[0] - sin_theta*r.direction()[2];
	direction[2] = sin_theta * r.direction()[0] + cos_theta*r.direction()[2];
	ray rotated_r(origin, direction, r.time());
	if (ptr->hit(rotated_r, t_min, t_max, rec)) {
		vec3 p = rec.p;
		vec3 normal = rec.normal;
		p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
		p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
		normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
		normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];
		rec.p = p;
		rec.normal = normal;
		return true;
	}
	else
		return false;
}

#endif
