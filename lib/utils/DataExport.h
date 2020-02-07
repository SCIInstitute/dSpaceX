//
// Created by kyli.nmb on 10/10/19.
//

#ifndef DSPACEX_DATAEXPORT_H
#define DSPACEX_DATAEXPORT_H

#include <vector>
#include <fstream>
#include <iostream>
#include "flinalg/Linalg.h"

class DataExport {

public:
    static void exportCrystalPartitions(std::vector<FortranLinalg::DenseVector<int>> crystalPartitions,
                                 int startPersistance, std::string filename);

};
#endif //DSPACEX_DATAEXPORT_H
