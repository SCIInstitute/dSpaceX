"""mesh_to_volume

Convert a mesh to a binary volume.

Author: Cameron Christensen
Created: 2020.11.01

"""

import trimesh
import numpy
import nrrd

"""
Returns a trimesh object or None.
"""
def read_mesh(filename):
    try:
        return trimesh.load_mesh(filename)
    except:
        return None
    
"""
Writes a nrrd of this volume data.
:param spacing: voxel size
:param origin: 
"""
def write_volume(filename, data, spacing = [1,1,1], origin = [0,0,0])

I started this, then started using it for ShapeWorks. Is it needed here?
It was desired to create larger binary volumes since no code was provided to do so.
But is it needed? It would be nice for... ???
