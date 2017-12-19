#include "Spring.h"

// ------

Spring::Spring() {
  position = 0.0;
  velocity = 0.0;
  target = 0.0;
  k = 1000.0;
  zeta = 1.0;
}

Spring::~Spring() {}

// ------

double Spring::getPosition() { return position; }
double Spring::getVelocity() { return velocity; }
double Spring::getTarget() { return target; }

// ------

void Spring::setPosition( double value ) { position = value; }
void Spring::setVelocity( double value ) { velocity = value; }
void Spring::setTarget( double value ) { target = value; }
void Spring::setK( double value ) { k = value; }
void Spring::setZeta( double value ) { zeta = value; }

// ------

void Spring::update( double deltaTime ) {
  double gap = position - target;
  velocity += ( -k * gap - 2.0 * velocity * sqrtf( k ) * zeta ) * deltaTime;
  position += velocity * deltaTime;
}