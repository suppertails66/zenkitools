#ifndef ZENKISCRIPTLINK_H
#define ZENKISCRIPTLINK_H


#include "util/TStream.h"
#include "util/TBufStream.h"
#include "zenki/ZenkiScriptOps.h"
#include "zenki/ZenkiScriptAsm.h"
#include "zenki/ZenkiScriptLink.h"
#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>

namespace Sms {

class ZenkiScriptLink {
public:
  ZenkiScriptLink(ZenkiAsmStruct& asmStruct__,
                 BlackT::TStream& dst__,
                 int loadAddr__);
  
  virtual void operator()();
protected:
  ZenkiAsmStruct& asmStruct;
  BlackT::TStream& dst;
  int loadAddr;
};

}


#endif
