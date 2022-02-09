# -*- coding: utf-8 -*-
# cython: language_level=3
import argparse
import os
import sys
import numpy as np
import struct
from ui_utils import *
import external_tools as ext
from subprocess import check_call
from sklearn.cluster import KMeans


def parse_arguments():
    parser = argparse.ArgumentParser(description='HMR Partition')
    parser.add_argument('-n', '--nodes', dest='nodes', type=str, required=True, help='Enzyme count result (.fasta.enz_count)')
    parser.add_argument('-e', '--edges', dest='edges', type=str, required=True, help='HiC pair count result (.bam.edge)')
    parser.add_argument("-o", "--output", dest='output', type=str, required=True, help="Partition group files (.txt)")
    parser.add_argument("-g", "--groups", dest='groups', type=int, required=True, help="Number of groups")
    parser.add_argument("-m", "--memory", dest='memory', type=int, default=8, help="Memory for Java virtual machine in GiB (default: 8)")
    parser.add_argument("-c", "--change-per-node", dest='change_per_node', type=float, default=2.0, help="Target distance change per node before stop the algorithm (default: 2.0)")
    parser.add_argument('-t', '--threads', dest='threads', type=int, default=1, help='Number of jobs (default: 1)')
    return parser.parse_args(sys.argv[1:])


def force_atlas2(graph_filepath, coord_filepath, n_jobs=1, target_change_per_node=2.0, memory=1, random_state=806):
    # Construct the jar paths.
    ext_path = os.path.join(os.path.dirname(__file__), 'ext')
    jar_cp = [os.path.join(ext_path, 'forceatlas2.jar'), os.path.join(ext_path, 'gephi-toolkit-0.9.2-all.jar')]
    # For Windows, use ;, for UNIX, use :
    if sys.platform == 'win32':
        cp_split = ';'
    else:
        cp_split = ':'
    jar_cp = cp_split.join(jar_cp)
    if not os.path.isfile(graph_filepath):
        raise FileExistsError('Graph file not exist.')
    # Construct the command.
    command = ['java',
               '-Djava.awt.headless=true',
               '-Xmx{memory}g'.format(memory=memory),
               '-cp', jar_cp,
               'kco.forceatlas2.Main',
               '--input', graph_filepath,
               '--output', coord_filepath,
               '--nthreads', str(n_jobs),
               '--seed', str(random_state),
               '--targetChangePerNode', str(target_change_per_node),
               '--2d']
    # Run the command.
    check_call(command)


def read_coords(coord_filepath):
    # Read the coordinates path.
    with open(coord_filepath, 'r') as coords_f:
        coord_rows = coords_f.read().split('\n')
        total_len = len(coord_rows) - 2
        coord = [[]] * total_len
        for ii, coord_row in enumerate(coord_rows):
            # Ignore the first row and last row.
            if ii == 0 or len(coord_row) == 0:
                continue
            # Loop and parse the x, y
            _, x, y = coord_row.split('\t')
            # Parse the x, y.
            coord[ii-1] = [float(x), float(y)]
    return np.asarray(coord)


def hmr_main():
    args = parse_arguments()
    # Check file existence.
    if not os.path.isfile(args.nodes):
        error_exit('Enzyme count count file does not exist:\n{}'.format(args.nodes))
    if not os.path.isfile(args.edges):
        error_exit('HiC pair count file does not exist:\n{}'.format(args.edges))
    # Construct the graph network path.
    output_dir = os.path.dirname(args.output)
    _, node_name = os.path.split(args.nodes)
    node_base_name, _ = os.path.splitext(node_name)
    graph_path = os.path.join(output_dir, node_base_name + '.net')
    time_print('Generating network file...')
    # Use HMR correction to generate the corrected FASTA file.
    if ext.hmr_tool('graph', ['-n', args.nodes, '-e', args.edges, '-o', graph_path]) != 0:
        return 1
    # Run Force Atlas 2 to get the coordinate.
    time_print('Running ForceAtlas2 on graph...')
    graph_dir, graph_name = os.path.split(graph_path)
    graph_name, _ = os.path.splitext(graph_name)
    coords_path = os.path.join(graph_dir, '{}.coords.txt'.format(graph_name))
    force_atlas2(graph_path, coords_path, n_jobs=args.threads, target_change_per_node=args.change_per_node,
                 memory=args.memory, random_state=806)
    # K-clusters on coordinates.
    time_print('Running K-Means on result...')
    coords = read_coords(coords_path)
    args.groups = int(args.groups)
    km = KMeans(n_clusters=args.groups).fit(coords)
    # Based on the labels, categories the item.
    labels = [[] for _ in range(args.groups)]
    for contig_id, label in enumerate(km.labels_):
        labels[label].append(contig_id)
    # Apply hyper-prune.
    rs_center = km.cluster_centers_
    # Write the binary group data.
    group_bin = '{}.bin'.format(args.output)
    with open(group_bin, 'wb') as group_result_bin:
        # Write the group no.
        group_result_bin.write(args.groups.to_bytes(4, byteorder='little', signed=False))
        # Write the center positions.
        for x, y in rs_center:
            group_result_bin.write(struct.pack("f", x))
            group_result_bin.write(struct.pack("f", y))
        # Write the group information.
        for contig_list in labels:
            group_result_bin.write(len(contig_list).to_bytes(4, byteorder='little', signed=False))
            for contig_id in contig_list:
                group_result_bin.write(contig_id.to_bytes(4, byteorder='little', signed=False))
    # Run partition tool to extract the data.
    time_print('Unpack the binary group into text files...')
    if ext.hmr_tool('graph', ['-n', args.nodes, '-u', group_bin, '-g', args.output]) != 0:
        return 1
    time_print('Complete.')


if __name__ == '__main__':
    hmr_main()
