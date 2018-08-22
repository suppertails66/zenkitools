#ifndef ZENKISCRIPTDISM_H
#define ZENKISCRIPTDISM_H


#include "util/TStream.h"
#include "zenki/ZenkiScriptOps.h"
#include <vector>
#include <iostream>

namespace Sms {

class ZenkiScriptDism {
public:
  ZenkiScriptDism(BlackT::TStream& src__,
                  ZenkiScriptOpCollection& dst__,
                  ZenkiDismStruct& dismStruct__);
  
  virtual void operator()();
protected:
  BlackT::TStream& src;
  ZenkiScriptOpCollection& dst;
  ZenkiDismStruct& dismStruct;
};

}


#endif
