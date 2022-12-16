#pragma once

// Float numbers by NDRAEY
bool float_get_sign(float num);
int float_get_exp(float num);
int float_get_mantissa(float num);

float modf(float value, int* intres);
int float_get_frac_part(float value);
