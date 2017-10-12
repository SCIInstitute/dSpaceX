#pragma once

#include "HDProcessResult.h"
#include <string>

class HDProcessResultSerializer {
public:
  static HDProcessResult* read(std::string path);
  static void write(HDProcessResult *result, std::string path);
};