# -*- coding: utf-8 -*-
# cython: language_level=3
import os
import sys
import traceback
import importlib.util

hmr_mods = {
    'correct': 'Divided FASTA sequence into smaller parts.',
    'filter': 'Filter the BAM file.'
}


def print_usage_func_line(command: str, explain):
    if len(command) > 21:
        print('  {}'.format(command))
        print('  {}{}'.format(' ' * 22, explain))
        return
    print('  {}{}{}'.format(command, ' ' * (22 - len(command)), explain))


def print_usage():
    print('Usage: hmr [command] [options for command]')
    print('arguments:')
    for mod_name in sorted(hmr_mods):
        print_usage_func_line(mod_name, hmr_mods[mod_name])
    exit(-1)


def module_load_err(command: str):
    print('Failed to load command {} module.'.format(command))
    exit(-1)


def pop_command():
    # Check the arguments.
    if len(sys.argv) < 2:
        print('Lacking the command for HMR.')
        print_usage()
    command = sys.argv[1]
    # Pop the argument.
    sys.argv = sys.argv[1:]
    return command


def main():
    command = pop_command()
    if command == '-c':
        # Ignore and check again.
        command = pop_command()
    # Check whether the command exist in the list.
    if command.lower() not in hmr_mods:
        print('Unknown command "{}"'.format(command))
        print_usage()
    # Load the module.
    bin_dir = os.path.dirname(__file__)
    module_list = list(filter(lambda x: x.startswith(command) and x.endswith('.py'), os.listdir(bin_dir)))
    if len(module_list) == 0:
        module_load_err(command)
    module_name = module_list[0]
    # Try to load and execute the command.
    try:
        mod_spec = importlib.util.spec_from_file_location('', os.path.join(os.path.dirname(__file__), module_name))
        module = importlib.util.module_from_spec(mod_spec)
        mod_spec.loader.exec_module(module)
        # Check whether module contains the execution port.
        if hasattr(module, 'hmr_main'):
            module.hmr_main()
        else:
            print('Invalid command module {}.'.format(module_name))
    except Exception:
        print('The following rrror happens when running command {}:'.format(command))
        print(traceback.format_exc())


if __name__ == '__main__':
    main()
