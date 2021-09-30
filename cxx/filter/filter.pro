TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lz -lpthread

INCLUDEPATH = ../libs

SOURCES += \
    ../libs/hp_unzip.cpp \
    ../libs/ui_utils.cpp \
    src/arguments.cpp \
    src/bam_filter.cpp \
    src/compose_reduced.cpp \
    src/enzyme.cpp \
    ../libs/hp_bam_parser.cpp \
    ../libs/hp_bgzf_parser.cpp \
    ../libs/hp_bgzf_queue.cpp \
    src/hp_fasta_search.cpp \
    ../libs/hp_file_map.cpp \
    src/hp_zip.cpp \
    src/hp_zip_crc32.cpp \
    src/main.cpp

HEADERS += \
    ../libs/hp_unzip.h \
    ../libs/ui_utils.h \
    src/arguments.h \
    src/bam_filter.h \
    src/compose_reduced.h \
    src/enzyme.h \
    ../libs/hp_bam_parser.h \
    ../libs/hp_bam_types.h \
    ../libs/hp_bgzf_parser.h \
    ../libs/hp_bgzf_queue.h \
    ../libs/hp_bgzf_types.h \
    src/hp_fasta_search.h \
    src/hp_fasta_types.h \
    ../libs/hp_file_map.h \
    ../libs/hp_thread_pool.h \
    src/hp_zip.h \
    src/hp_zip_crc32.h

