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
    ../libs/hp_thread_pool.h \
    ../libs/hp_unzip.h \
    ../libs/hp_zip.h \
    ../libs/hp_zip_crc32.h \
    ../libs/ui_utils.h \
    src/arguments.h \
    src/bam_filter.h \
    src/compose_reduced.h \
    src/enzyme.h \
    src/hp_fasta_search.h \
    src/hp_fasta_types.h

SOURCES += \
    ../libs/hp_bam_parser.cpp \
    ../libs/hp_bgzf_parser.cpp \
    ../libs/hp_bgzf_queue.cpp \
    ../libs/hp_file_map.cpp \
    ../libs/hp_unzip.cpp \
    ../libs/hp_zip.cpp \
    ../libs/hp_zip_crc32.cpp \
    ../libs/ui_utils.cpp \
    src/arguments.cpp \
    src/bam_filter.cpp \
    src/compose_reduced.cpp \
    src/enzyme.cpp \
    src/hp_fasta_search.cpp \
    src/main.cpp
