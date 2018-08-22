#include "zenki/ZenkiScriptAsm.h"
#include "util/TStringConversion.h"
#include "util/TParse.h"
#include "util/TIfstream.h"
#include "exception/TException.h"
#include "exception/TGenericException.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


ZenkiAsmSection::ZenkiAsmSection()
  : data(0x10000) { };

ZenkiScriptAsm::ZenkiScriptAsm(BlackT::TStream& src__,
//                  ZenkiScriptOpCollection& dst__,
                  ZenkiAsmStruct& asmStruct__,
                  std::string filename__)
  : src(src__),
//    dst(dst__),
    asmStruct(asmStruct__),
    filename(filename__),
    lineNum(0) {
  
}
  
void ZenkiScriptAsm::operator()() {
  try {
    while (!src.eof()) {
      std::string line;
      src.getLine(line);
      
      // remove trailing comments
      for (int i = 0; i < line.size(); i++) {
        if (line[i] == ';') {
          line = line.substr(0, i);
          break;
        }
      }

      // read until end of line or until a comment
//      TParse::getUntilChars(src, ";\n\r");
//      src.unget();
//      if (src.get() == ';') { std::string garbage; src.getLine(garbage); }
      
      ++lineNum;
      
      if (line.size() <= 0) continue;
      
      // discard lines containing only ASCII spaces and tabs
  //    bool onlySpace = true;
  //    for (int i = 0; i < line.size(); i++) {
  //      if ((line[i] != ' ')
  //          && (line[i] != '\t')) {
  //        onlySpace = false;
  //        break;
  //      }
  //    }
  //    if (onlySpace) continue;
      
      TBufStream ifs(line.size());
      ifs.write(line.c_str(), line.size());
      ifs.seek(0);
      
      // linebreak issues?
      if ((ifs.size() <= 1) && src.eof()) break;
      
      // get command name
      std::string str = TParse::getSpacedSymbol(ifs);
      
      // empty line
      if (str.empty()) continue;
      
      // comment
      if (str[0] == ';') {
        continue;
      }
      
      // directive
      if (str[0] == '.') {
        processDirective(ifs, str.substr(1, std::string::npos));
  //      std::cout << str.substr(1, std::string::npos) << std::endl;
        continue;
      }
      
      // label
      if (str[str.size() - 1] == ':') {
        std::string label = str.substr(0, str.size() - 1);
        asmStruct.activeSection.labels[label]
          = asmStruct.activeSection.data.tell();
  //      std::cout << label << std::endl;
        continue;
      }
      
      // operation
      ifs.seek(0);
      TParse::skipSpace(ifs);
  //    if (ifs.eof()) continue;
      ZenkiScriptOp op;
      op.parseOp(ifs, asmStruct);
      op.writeOp(asmStruct.activeSection.data);
      
      // check for comments
  //    if ((ifs.size() >= 2)
  //        && (ifs.peek() == '/')) {
  //      ifs.get();
  //      if (ifs.peek() == '/') continue;
  //      else ifs.unget();
  //    }
      
      // check for special stuff
  //    if (ifs.peek() == '#') {
  //      // directives
  //      ifs.get();
  //      processDirective(ifs);
  //      continue;
  //    }
      
  //    while (!ifs.eof()) {
  //      outputNextSymbol(ifs);
  //    }
    }
  
  }
/*  catch (BlackT::TException& e) {
    if (filename.size() > 0) {
      std::cerr << "In file " << filename << ":" << std::endl;
    }
    else if (filename.size() > 0) {
      std::cerr << "In main file:" << std::endl;
    }
    
    std::cerr << "  Line " << lineNum << ":" << std::endl
              << "    Error: "
              << e.what() << std::endl;
//              << "  Source: " << e.source() << std::endl;
    throw e;
  } */
  catch (std::exception& e) {
    if (filename.size() > 0) {
      std::cerr << "In file " << filename << ":" << std::endl;
    }
    else {
      std::cerr << "In main file:" << std::endl;
    }
    
    std::cerr << "  Line " << lineNum << ":" << std::endl
//              << "    Unhandled exception: "
              << "    Error: "
              << e.what() << std::endl;
    throw e;
  }
}
  
void ZenkiScriptAsm::processDirective(BlackT::TStream& ifs, std::string name) {
  for (int i = 0; i < name.size(); i++) {
    name[i] = tolower(name[i]);
  }
  
  if (name.compare("db") == 0) {
    processDbDw(ifs, 1);
  }
  else if (name.compare("dw") == 0) {
    processDbDw(ifs, 2);
  }
  else if (name.compare("define") == 0) {
    processDefine(ifs);
  }
  else if (name.compare("include") == 0) {
    processInclude(ifs);
  }
  else {
    throw BlackT::TGenericException(T_SRCANDLINE,
                            "ZenkiScriptAsm::processDirective()",
                            std::string("Unknown directive: ")
                              + name);
  }
}

void ZenkiScriptAsm::processDbDw(BlackT::TStream& ifs, int size) {
  
  TParse::skipSpace(ifs);
  do {
    
    ZenkiScriptArg arg;
    ZenkiScriptOp::parseArg(ifs, arg);

    if (arg.type == ZenkiScriptArg::literal) {
      asmStruct.activeSection.data.writeInt(
        arg.value, size, EndiannessTypes::little, SignednessTypes::nosign);
    }
    else if (arg.type == ZenkiScriptArg::expression) {
      ZenkiAsmExpression expr;
      switch (size) {
      case 1:
        expr.dataType = ZenkiAsmExpression::oneByte;
        break;
      case 2:
        expr.dataType = ZenkiAsmExpression::twoByte;
        break;
      default:
        throw BlackT::TGenericException(T_SRCANDLINE,
                                "ZenkiScriptAsm::processDbDw()",
                                std::string("Illegal size: ")
                                  + TStringConversion::intToString(size));
        break;
      }
      expr.addr = asmStruct.activeSection.data.tell();
      expr.expression = arg.expressionValue;
      
      asmStruct.activeSection.pendingExpressions.push_back(expr);
      asmStruct.activeSection.data.writeInt(
        0, size, EndiannessTypes::little, SignednessTypes::nosign);
    }
    else {
      throw BlackT::TGenericException(T_SRCANDLINE,
                              "ZenkiScriptAsm::processDbDw()",
                              std::string("Unknown arg type: ")
                                + TStringConversion::intToString(arg.type));
    }
    
    ifs.unget();
    TParse::skipSpace(ifs);
  } while (!ifs.eof() && (ifs.get() == ','));
  
}

void ZenkiScriptAsm::processDefine(BlackT::TStream& ifs) {
  std::string name = TParse::matchName(ifs);
  // literals only
  int value = TParse::matchInt(ifs);
  
  asmStruct.definitions[name] = value;
}

void ZenkiScriptAsm::processInclude(BlackT::TStream& ifs) {
  std::string name = TParse::matchString(ifs);
  
  TIfstream incifs(name.c_str(), std::ios_base::binary);
  ZenkiScriptAsm(incifs, asmStruct, name)();
}


}

 
