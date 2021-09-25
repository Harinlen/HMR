import os
import distutils.spawn
import subprocess


def find_bin(exe_name: str):
    return distutils.spawn.find_executable(exe_name)


def bin_exist(exe_path: str):
    return os.path.isfile(exe_path)


def standard_execution(exe_name: str, specify_arg: str, arguments: list, exe_path: str, block: bool):
    # Find the execution.
    if len(exe_path) == 0 or not bin_exist(exe_path):
        exe_path = find_bin(exe_name)
        if exe_path is None:
            print('Failed to find program {}, please specify the bwa path using "{}" argument.'.format(exe_name, specify_arg))
            exit(-1)
    # Use Popen to run the program.
    if block:
        subprocess.run([exe_path, *arguments])
    else:
        return subprocess.Popen([exe_path, *arguments])


def bwa(arguments: list, exe_path: str = '', block: bool = True):
    return standard_execution('bwa', '--with-bwa', arguments, exe_path, block)


def samtools(arguments: list, exe_path: str = '', block: bool = True):
    return standard_execution('samtools', '--with-samtools', arguments, exe_path, block)

