import math
import matplotlib.tri as mtri
import numpy as np
import os
import pandas as pd


def generate_nano_thumbnails(parameter_csv, output_directory):
    parameter_df = pd.read_csv(parameter_csv)

    output_directory = os.path.join(output_directory, '')
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    for sample in parameter_df.itertuples():
        print('shape %i / %i' % (sample.Index, len(parameter_df.index)), end='\r')
        print('Generating mesh shape representation %.2f percent complete. Generating mesh %i of %i. ' %
              ((100 * sample.Index / len(parameter_df.index)), sample.Index, len(parameter_df.index)), end='\r')

        X, Y, Z, tri_indices = nano_formula_3d(sample.m, sample.n1, sample.n2, sample.n3, sample.a, sample.b, 100000)
        vertices = np.column_stack((X, Y, Z))
        np.savetxt(output_directory + str(sample.Index + 1) + '.csv', vertices, delimiter=',')


def nano_formula_3d(m, n1, n2, n3, a, b, num_points):
    num_points_root = round(math.sqrt(num_points))

    theta = np.linspace(-math.pi, math.pi, endpoint=True, num=num_points_root)
    phi = np.linspace(-math.pi / 2.0, math.pi/2.0, endpoint=True, num=num_points_root)

    theta, phi = np.meshgrid(theta, phi)
    theta, phi = theta.flatten(), phi.flatten()

    r1 = nano_formula_2d(m, n1, n2, n3, a, b, theta)
    r2 = nano_formula_2d(m, n1, n2, n3, a, b, phi)

    x = r1 * r2 * np.cos(theta) * np.cos(phi)
    y = r1 * r2 * np.sin(theta) * np.cos(phi)
    z = r2 * np.sin(phi)

    tri = mtri.Triangulation(theta, phi)
    return x, y, z, tri.triangles


def nano_formula_2d(m, n1, n2, n3, a, b, theta):
    r = abs((1 / a) * np.cos(m * theta / 4.0))**n2 + abs((1 / b) * np.sin(m * theta / 4.0))**n3
    return r**(-1 / n1)


# testing
input_params_dir = '/Users/kylimckay-bishop/dSpaceX/nanoparticles_mesh/Nanoparticles_Parameters.csv'
output_dir = '/Users/kylimckay-bishop/dSpaceX/nanoparticles_mesh/images'

generate_nano_thumbnails(input_params_dir, output_dir)