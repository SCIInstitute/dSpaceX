import re
from glob import glob
import nrrd


def read_nrrd_files(directory):
    designs = []
    for file in glob(directory + '*.nrrd'):
        shape, header = nrrd.read(file)
        # TODO improve this - if there is more than one number (like a date) in a file name this will break
        design_number = list(map(int, re.findall(r'\d+', file)))[0]
        designs.append({'id': design_number, 'shape': shape, 'header': header})
    designs.sort(key=lambda d: d['id'])
    return designs





