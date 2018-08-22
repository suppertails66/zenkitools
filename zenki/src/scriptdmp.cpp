#include "sms/SmsPattern.h"
#include "util/TIfstream.h"
#include "util/TBufStream.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Sms;

std::map<int, int> scriptptrToIndex;
std::map<int, int> indexToScriptptr;
std::map<int, int> indexToLength;
std::map<int, int> scriptptrToLength;
int curscrindex;

const static int numStrings = 0xDA;
const static int stringTableBase = 0x70000;

std::string toRawHex(int value) {
  return TStringConversion::intToString(value, TStringConversion::baseHex)
            .substr(2, std::string::npos);
}

void dismPos(TStream& ifs, std::ostream& ofs,
             std::ostream& spaceOfs,
             TThingyTable& thingy,
             int pos) {
  ifs.seek(pos);
//  TBufStream stream(len);
//  stream.writeFrom(ifs, len);
//  stream.seek(0);
  
//  addFreeBlock(ofs, pos, len);

  std::string output;
  output += "// ";
  
  while (true) {
    if (ifs.peek() == 0x00) {
      ifs.get();
      break;
    }
    
    TThingyTable::IdLengthPair next
      = thingy.matchId(ifs);
    if (next.first == -1) {
      throw TGenericException(T_SRCANDLINE,
                              "dismPos()",
                              "No match at: "
                                + TStringConversion::intToString(ifs.tell(),
                                    TStringConversion::baseHex));
    }
    
    std::string str = thingy.getEntry(next.first);
    output += str;
    
    // linebreak
    if (next.first == 0x0D) {
      output += '\n';
      output += "// ";
    }
  }
  
//  int len = ifs.tell() - pos;

//  ofs << "// $" << toRawHex(pos) << " (" << len << " bytes)" << endl;
  ofs << output;
  ofs << endl;
  

  ofs << endl << endl << "#END()" << endl << endl;
}

void dismFixedPos(TStream& ifs, std::ostream& ofs,
                  std::ostream& spaceOfs,
                  TThingyTable& thingy,
                  int pos, int w, int h) {
  ifs.seek(pos);
//  TBufStream stream(len);
//  stream.writeFrom(ifs, len);
//  stream.seek(0);
  
//  addFreeBlock(ofs, pos, len);

  std::string output;
  output += "// ";
  
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      TThingyTable::IdLengthPair next
        = thingy.matchId(ifs);
      if (next.first == -1) {
        throw TGenericException(T_SRCANDLINE,
                                "dismPos()",
                                "No match at: "
                                  + TStringConversion::intToString(ifs.tell(),
                                      TStringConversion::baseHex));
      }
      std::string str = thingy.getEntry(next.first);
      output += str;
    }
    output += '\n';
  }
  
  int len = w * h;

  ofs << "// $" << toRawHex(pos) << " (" << len << " bytes)" << endl << endl;
  ofs << "#SETMSGLOC($"
    << TStringConversion::intToString(pos, TStringConversion::baseHex)
        .substr(2, std::string::npos)
    << ", " << len << ")" << endl << endl;
  ofs << output;
  ofs << endl;
  

  ofs << endl << endl << "#ENDMSG()" << endl << endl;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Kishin Douji Zenki (Game Gear) dialogue dumper" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outprefix> <table>" << endl;
    return 0;
  }
  
  {
    std::ofstream ofs(argv[2], ios_base::trunc);
  }
  
//  TIfstream ifs(argv[1], ios_base::binary);
  TBufStream ifs;
  ifs.open(argv[1]);
  std::string outprefix = std::string(argv[2]);
//  std::ofstream spaceOfs(".scrtemp");
  std::ofstream spaceOfs("zenki_space.txt");
  TThingyTable thingy;
  thingy.readSjis(string(argv[3]));
  
//  addFixedBlock(spaceOfs, 0x2FA0, 0x1060);
//  addFreeBlock(spaceOfs, 0x7BF0, 0x400);
//  addFreeBlock(spaceOfs, 0xBA40, 0x5C0);
//  addFreeBlock(spaceOfs, 0xEC00, 0x1400);
  
//  clearFreeBlocks(ofs);
//  addFreeBlock(ofs, 0xDE98, 0x1FF);
//  ofs << endl;
  
/*  ofs << "//===========================================================" << endl
      << "// Cutscenes" << endl
      << "//===========================================================" << endl;
  
  ofs << endl
      << "//================================" << endl
      << "// Intro 1" << endl
      << "//================================" << endl;
//  addPointer(ofs, 0xD881, 1);
//  dismPos(ifs, ofs, spaceOfs, thingy, 0xDE98, 0x1FF);

  addPointer(ofs, 0xD881, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xDE98, 0xBB);
  addPointer(ofs, 0xD9A9, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xDF53, 0x76);
  addPointer(ofs, 0xDA0B, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xDFC9, 0x57);
  addPointer(ofs, 0xDA63, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE020, 0x77); */
  
/*  {
    std::ofstream ofs((outprefix + "zenki_strings.txt").c_str());
    for (int i = 0; i < numStrings; i++) {
      ifs.seek(stringTableBase + (i * 2));
      int addr = (ifs.readu16le() - 0x8000) + stringTableBase;
  //    ifs.seek(addr);
      
      ofs << "// String " << TStringConversion::intToString(i,
                                TStringConversion::baseHex)
           << std::endl;
      dismPos(ifs, ofs, spaceOfs, thingy, addr);
    }
  } */
  
  {
    std::ofstream ofs((outprefix + "zenki_stringmaps.txt").c_str());
    
    ofs << endl
      << "//================================" << endl
      << "// Credits" << endl
      << "//================================" << endl << endl;
    
//    dismFixedPos(ifs, ofs, spaceOfs, thingy, 0x73D92 + (0x14 * 0), 0x14, 0x1);
//    dismFixedPos(ifs, ofs, spaceOfs, thingy, 0x73DA6 + (0x14 * 1), 0x14, 0x1);
    for (int i = 0; i < 22; i++) {
      dismFixedPos(ifs, ofs, spaceOfs, thingy, 0x73D92 + (0x14 * i), 0x14, 0x1);
    }
    
    ofs << endl
      << "//================================" << endl
      << "// Game over yes/no prompt" << endl
      << "//================================" << endl << endl;
    dismFixedPos(ifs, ofs, spaceOfs, thingy, 0x73CE2, 0x8, 0x1);
  }
  
  return 0;
}
