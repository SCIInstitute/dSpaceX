//
// Created by kyli.nmb on 10/10/19.
//

#include "DataExport.h"

/**
 * Exports comma seperated list of crystal ID for each sample based on persistence level.
 * @param crystalPartitions
 * @param startPersistance
 * @param filename
 */
void DataExport::exportCrystalPartitions(std::vector<FortranLinalg::DenseVector<int>> crystalPartitions,
                                         int startPersistance, std::string filename) {
    std::ofstream outfile;
    outfile.open(filename);
    for (auto persistenceItr = (crystalPartitions.begin() + startPersistance);
         persistenceItr != crystalPartitions.end(); ++persistenceItr) {
        for (unsigned int sampleIndex = 0; sampleIndex < persistenceItr->N(); ++sampleIndex) {
            if (sampleIndex == (persistenceItr->N() - 1)) { // Don't want dangling commas
                outfile << persistenceItr->operator()(sampleIndex);
            } else {
                outfile << persistenceItr->operator()(sampleIndex) << ",";
            }
        }
        outfile << "\n";
    }
    outfile.close();
}
