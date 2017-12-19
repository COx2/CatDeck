#pragma once

#include "math.h"

class Spring {
public:
  Spring();
  ~Spring();
  
  double getPosition();
  double getVelocity();
  double getTarget();
  
  void setPosition( double value );
  void setVelocity( double value );
  void setTarget( double value );
  void setK( double value );
  void setZeta( double value );
  
  void update( double deltaTime );
  
private:
  double position;
  double velocity;
  double target;
  double k;
  double zeta;
};