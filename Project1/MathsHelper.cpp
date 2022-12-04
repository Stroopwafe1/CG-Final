#include "MathsHelper.h"

namespace MathsHelper {
	/*
	Clamps a given input to either min or max when it exceeds them
	@param min - The minimum value it is allowed to have
	@param max - The maximum value it is allowed to have
	@param input - The input value to clamp
	*/
	float clamp(float min, float max, float input) {
		if (input < min)
			return min;
		else if (input > max)
			return max;
		return input;
	}

	/*
	Returns the linear interpolation point of the given input from min to max
	@param min - The value it should have when input = 0
	@param max - The value it should have when input = 1
	@param input - The input value to determine the linear interpolation of
	@returns A value on a linear scale from min to max at point input
	*/
	float lerp(float min, float max, float input) {
		return min + (max - min) * clamp(0, 1, input);
	}
}