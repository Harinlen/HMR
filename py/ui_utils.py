# -*- coding: utf-8 -*-
# cython: language_level=3
import time


def error_exit(*args, **kwargs):
    print(*args, **kwargs)
    exit(-1)


def time_print(*args, **kwargs):
    print("\033[32m%s\033[0m " % (time.strftime('[%H:%M:%S]', time.localtime(time.time()))), end='')
    print(*args, **kwargs)
