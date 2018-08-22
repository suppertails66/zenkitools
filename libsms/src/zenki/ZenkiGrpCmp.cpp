#include "zenki/ZenkiGrpCmp.h"
#include "sms/PsCmp.h"
#include "sms/StCmp.h"
#include "util/TStringConversion.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


void ZenkiGrpCmp::cmpZenki(BlackT::TStream& src, BlackT::TStream& dst) {
  dst.put(4);
  PsCmp::cmpPs(src, dst, 4);
}

void ZenkiGrpCmp::decmpZenki(BlackT::TStream& src, BlackT::TStream& dst) {
  unsigned char numPlanes = src.get();
  PsCmp::decmpPs(src, dst, 4);
}


}
