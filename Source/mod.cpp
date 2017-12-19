#include "mod.h"

double mod( double value, double div ) {
  return value - floor( value / div ) * div;
}