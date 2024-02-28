#pragma once
#include <iostream>
#include <math.h>


static float LinToDb(float input) {
	return log10f(input) * 10.0f;
}

static float DbToLin(float input) {
	return powf(10.0f, input / 10.0f);
}

static float pi = 3.14159265358979323846f;