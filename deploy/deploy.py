# -*- coding: utf-8 -*-
import argparse
import os
import sys
import shutil
import stat


def parse_arguments():
    parser = argparse.ArgumentParser(description='HMR Correction')
    parser.add_argument('path', type=str, help='HMR install target path')
    return parser.parse_args(sys.argv[1:])


def deploy_cpp_to():
    pass


def main():
    args = parse_arguments()
    # Check the directory name is hmr or not.
    if os.path.basename(args.path) != 'hmr':
        args.path = os.path.join(args.path, 'hmr')
    # Generate the target directory.
    os.makedirs(args.path, exist_ok=True)
    # Get the src paths.
    src_root_path = os.path.dirname(os.path.dirname(__file__))
    # Deploy the python files.
    shutil.copytree(os.path.join(src_root_path, 'py'), args.path, dirs_exist_ok=True)
    # Copy the bash file.
    shell_path = os.path.join(src_root_path, 'shells')
    for filename in os.listdir(shell_path):
        filepath = os.path.join(shell_path, filename)
        if not os.path.isfile(filepath):
            continue
        target_path = os.path.join(args.path, filename)
        shutil.copyfile(filepath, target_path)
        # Change mode.
        os.chmod(target_path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IROTH | stat.S_IXOTH)
    # Use cmake to compile and deploy the C++ codes.



if __name__ == '__main__':
    main()
