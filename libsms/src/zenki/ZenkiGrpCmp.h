#ifndef ZENKIGRPCMP_H
#define ZENKIGRPCMP_H


#include "util/TStream.h"
#include "util/TThingyTable.h"
#include <map>
#include <string>
#include <iostream>

namespace Sms {


class ZenkiGrpCmp {
public:
  static void cmpZenki(BlackT::TStream& src, BlackT::TStream& dst);
  static void decmpZenki(BlackT::TStream& src, BlackT::TStream& dst);
  
protected:
  
};


}


#endif
