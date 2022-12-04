#include "Colour.h"

/*
Constructor for Colour, if given value is bigger than 1 the value is assumed to be on range 0-255 (and thus divided)
*/
Colour::Colour(float ri, float gi, float bi) {
	r = ri > 1 ? ri / 255 : ri;
	g = gi > 1 ? gi / 255 : gi;
	b = bi > 1 ? bi / 255 : bi;
}