# -*- coding: utf-8 -*-
# cython: language_level=3
import os
import distutils.spawn
import subprocess
from ui_utils import time_print


def fetch_binary(exe_name: str, exe_path: str = ''):
    # Find the execution.
    if len(exe_path) == 0:
        # We guess a path, which is under the current directory.
        exe_path = os.path.join(os.path.dirname(__file__), exe_name)
    if not os.path.isfile(exe_path):
        # Try to find the system level path.
        exe_path = distutils.spawn.find_executable(exe_name)
        if not isinstance(exe_path, str):
            return ''
    return exe_path


def run_program(exe_path: str, arguments: list, block: bool, stdin, stdout):
    if block:
        subprocess.run([exe_path, *arguments])
    else:
        return subprocess.Popen([exe_path, *arguments],
                                stdin=stdin,
                                stdout=stdout)


def standard_execution(exe_name: str, specify_arg: str, arguments: list, exe_path: str, block: bool, stdin, stdout):
    # Find the execution.
    exe_path = fetch_binary(exe_name, exe_path)
    if len(exe_path) == 0:
        print('Failed to find program {}, please specify the bwa path using "{}" argument.'.format(exe_name, specify_arg))
        exit(-1)
    # Use Popen to run the program.
    time_print('Running {} {}'.format(exe_name, ' '.join(str(x) for x in arguments)))
    return run_program(exe_path, arguments, block, stdin, stdout)


def bwa(arguments: list, exe_path: str = '', block: bool = False, stdin=None, stdout=None):
    return standard_execution('bwa', '--with-bwa', arguments, exe_path, block, stdin, stdout)


def samtools(arguments: list, exe_path: str = '', block: bool = False, stdin=None, stdout=None):
    return standard_execution('samtools', '--with-samtools', arguments, exe_path, block, stdin, stdout)


def hmr_tool(mod_name: str, arguments: list):
    run_program(fetch_binary('hmr-{}'.format(mod_name)), arguments, True, None, None)


def stage(program_method, *args, **kwargs):
    # Enable the pipeline mode.
    kwargs['block'] = False
    kwargs['stdin'] = None
    kwargs['stdout'] = None
    return [program_method, args, kwargs]


def run_pipeline(stages: list):
    if len(stages) < 2:
        raise Exception('Pipeline should be built with at least 2 programs.')
    # Change the last stage to non-pipeline mode.
    for ii in range(len(stages) - 1):
        stages[ii][2]['stdout'] = subprocess.PIPE
    # Create the first item in the stages.
    init_program_method, init_args, init_kwargs = stages[0]
    proc = init_program_method(*init_args, **init_kwargs)
    # Remove the first proc.
    stages.pop(0)
    for program_method, args, kwargs in stages:
        # Build the pipeline.
        kwargs['stdin'] = proc.stdout
        stage_proc = program_method(*args, **kwargs)
        # Close the last stage output to avoid deadlock?
        proc.stdout.close()
        # Update the stage.
        proc = stage_proc
    return proc
