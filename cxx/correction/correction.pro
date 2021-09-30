TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -ldeflate -lpthread

INCLUDEPATH = ../libs

HEADERS += \
    ../libs/hp_bam_parser.h \
    ../libs/hp_bam_types.h \
    ../libs/hp_bgzf_parser.h \
    ../libs/hp_bgzf_queue.h \
    ../libs/hp_bgzf_types.h \
    ../libs/hp_file_map.h \
    ../libs/hp_unzip.h \
    ../libs/ui_utils.h \
    src/arguments.h \
    src/bam_correction.h \
    src/hp_fasta_parser.h \
    src/hp_fasta_types.h \
    src/hp_str_utils.h

SOURCES += \
    ../libs/hp_bam_parser.cpp \
    ../libs/hp_bgzf_parser.cpp \
    ../libs/hp_bgzf_queue.cpp \
    ../libs/hp_file_map.cpp \
    ../libs/hp_unzip.cpp \
    ../libs/ui_utils.cpp \
    src/arguments.cpp \
    src/bam_correction.cpp \
    src/hp_fasta_parser.cpp \
    src/main.cpp
