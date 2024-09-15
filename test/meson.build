# SPDX-License-Identifier: GPL-2.0-or-later
test(
    'listing test',
    executable(
        'ls_test',
        'ls_test.c',
        '../src/db.c',
        '../src/ls.c',
        dependencies: spatialite,
        install: false,
        include_directories: ['../src/'],
    ),
    protocol: 'tap',
)

test(
    'glyph test',
    executable(
        'glyph_test',
        'glyph_test.c',
        '../src/glyph.c',
        dependencies: ffmpeg,
        install: false,
        include_directories: ['../src'],
    ),
    protocol: 'tap',
)

test(
    'video data test',
    executable(
        'video_data_test',
        'video_data_test.c',
        '../src/video_data.c',
        '../src/glyph.c',
        dependencies: ffmpeg,
        install: false,
        include_directories: ['../src'],
    ),
    protocol: 'tap',
)

test(
    'output data test',
    executable(
        'output_data_test',
        'output_data_test.c',
        '../src/output_data.c',
        '../src/db.c',
        dependencies: spatialite,
        install: false,
        include_directories: ['../src'],
    ),
    protocol: 'tap',
)
