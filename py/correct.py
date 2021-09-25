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
    # The first step is to call build index using bwa and samtools.
    time_print('Building index for reference file...')
    sam_index_proc = ext.samtools(['faidx', args.reference], args.sam, block=False)
    bwa_index_proc = ext.bwa(['index', args.reference], args.bwa, block=False)
    # Wait for process end.
    sam_index_proc.wait()
    bwa_index_proc.wait()


if __name__ == '__main__':
    hmr_main()
