#ifndef READ_HPP_
#define READ_HPP_

#include "bootsector.hpp"

void read(Mbr& mbr, const uint8_t* data);
void read(PbrFat& pbr, const uint8_t* data);

#endif
