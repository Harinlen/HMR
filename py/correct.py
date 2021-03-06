# -*- coding: utf-8 -*-
# cython: language_level=3
import argparse
import sys
import os
import external_tools as ext
from ui_utils import *


def parse_arguments():
    parser = argparse.ArgumentParser(description='HMR Correction')
    parser.add_argument('-1', '--r1', dest='r1', type=str, required=True, help='R1 reads (.fastq.gz)')
    parser.add_argument('-2', '--r2', dest='r2', type=str, required=True, help='R2 reads (.fastq.gz)')
    parser.add_argument('-r', '--reference', dest='reference', type=str, required=True, help='Reference genome (.fasta)')
    parser.add_argument('-o', '--output', dest='output', type=str, required=True, help='Corrected HiC genome (.fasta)')
    parser.add_argument("-p", "--percent", dest='percent', type=float, default=0.95, help="Percent of the map to saturate (default: 0.95)")
    parser.add_argument("-s", "--sensitive", dest='sensitive', type=float, default=0.5, help="sensitivity to depletion score (default: 0.5)")
    parser.add_argument("-q", "--mapq", dest='mapq', type=int, default=1, help="MAPQ of mapping lower bound (default: 1)")
    parser.add_argument("-w", "--wide", dest='wide', type=int, default=25000, help="Resolution for first pass search of mismatches (default: 25000)")
    parser.add_argument("-n", "--narrow", dest='narrow', type=int, default=1000, help="Resolution for the precise mismatch localizaton, n<w (default: 1000)")
    parser.add_argument("-d", "--depletion", dest='depletion', type=int, default=100000, help="The size of the region to aggregate the depletion score in the wide path, d >= 2*w (default: 100000)")
    parser.add_argument('-t', '--threads', dest='threads', type=int, default=1, help='Number of threads (default: 1)')
    parser.add_argument('--with-bwa', dest='bwa', type=str, default='', help='Specify the bwa binary path')
    parser.add_argument('--with-sam', '--with-samtools', dest='sam', type=str, default='', help='Specify the samtools binary path')
    return parser.parse_args(sys.argv[1:])


def hmr_main():
    args = parse_arguments()
    # Check file existence.
    if not os.path.isfile(args.r1):
        error_exit('R1 reads file does not exist:\n{}'.format(args.r1))
    if not os.path.isfile(args.r2):
        error_exit('R2 reads file does not exist:\n{}'.format(args.r2))
    if not os.path.isfile(args.reference):
        error_exit('Reference file does not exist:\n{}'.format(args.reference))
    # Check our correction binary exists or not.
    if len(ext.fetch_binary('hmr-correction')) == 0:
        error_exit('Failed to find the HMR correction binary, please reinstall HMR.')
    # Get the output file path.
    output_dir = os.path.dirname(args.output)
    if len(output_dir) == 0:
        output_dir = os.path.abspath(os.curdir)
    if not os.path.isdir(output_dir):
        os.makedirs(output_dir, exist_ok=True)
    # Convert the absolute path.
    args.r1 = os.path.abspath(args.r1)
    args.r2 = os.path.abspath(args.r2)
    # The first step is to call build index using bwa and samtools.
    time_print('Building index for reference file...')
    sam_index_proc = ext.samtools(['faidx', args.reference], args.sam)
    bwa_index_proc = ext.bwa(['index', args.reference], args.bwa)
    # Wait for process end.
    sam_index_proc.wait()
    bwa_index_proc.wait()
    time_print('Index build completed')
    time_print('Running mapping pipeline')
    # Run the mapping pipeline.
    sorted_bam_path = os.path.join(output_dir, 'sorted.bam')
    mapping_proc = ext.run_pipeline([
        ext.stage(ext.bwa, ['mem', '-SP5M', '-t', str(args.threads // 2), args.reference, args.r1, args.r2], args.bwa),
        ext.stage(ext.samtools, ['view', '-hF', '256', '-'], args.sam),
        ext.stage(ext.samtools, ['sort', '-@', str(args.threads // 2), '-o', sorted_bam_path, '-T', os.path.join(output_dir, 'tmp.ali')], args.sam)])
    # Wait for pipeline complete.
    mapping_proc.wait()
    time_print('Correcting the sequence')
    # Use HMR correction to generate the corrected FASTA file.
    ext.hmr_tool('correction', ['-m', sorted_bam_path, '-r', args.reference, '-o', args.output,
                                '-p', str(args.percent), '-s', str(args.sensitive), '-q', str(args.mapq), '-w', str(args.wide), '-n', str(args.narrow),
                                '-d', str(args.depletion), '-t', str(args.threads)])
    # Remove the sorted.bam and tmp.ali
    time_print('Cleaning up the temporary files')
    # Remove the sorted.bam.
    pass
    time_print('Correction finished')


if __name__ == '__main__':
    hmr_main()
