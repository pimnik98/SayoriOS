// FLoat numbers by NDRAEY

#define FLOAT_E_MAX 1000000000
// 0 - is positive; 1 - is negative
char float_get_sign(float num) { return (*((unsigned int*)&num)) >> 31; }
int float_get_exp(float num) { return ((*((unsigned int*)&num) & 0x7f800000) >> 23) - 0x7f; }
int float_get_mantissa(float num) { return (*((unsigned int*)&num)) & 0x007fffff; }

float modf(float value, int* intres) {
	char sign = float_get_sign(value);
	int exp = float_get_exp(value);
	int man = float_get_mantissa(value) | 0x00800000;
	int expd = 23 - exp;

	*intres = man >> expd;

	if (sign) {
		*intres = -(*intres);
	}

	int frac = man & ((1 << expd) - 1);
	float reso = 1 / (1 << expd);

	return frac * reso;
}

int float_get_frac_part(float value) {
	int man = float_get_mantissa(value) | 0x00800000;
	int expd = 23 - float_get_exp(value);

	int frac = man & ((1 << expd) - 1);
	int resolution = FLOAT_E_MAX / (1 << expd);

	return frac * resolution;
}
