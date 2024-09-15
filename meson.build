# SPDX-License-Identifier: GPL-2.0-or-later
project('onde_dirigi', 'c', version: '0.1', default_options: ['warning_level=3'])

fs = import('fs')
if fs.exists('test/data/private/header.h')
    add_project_arguments('-DHAS_PRIVATE_DATA=1', language: 'c')
else
    message('No private test data present - will skip some tests')
endif


ffmpeg = [
    dependency('libavutil'),
    dependency('libavformat'),
    dependency('libavcodec'),
]
spatialite = [dependency('spatialite')]

executable(
    'parse_directory',
    'src/glyph.c',
    'src/video_data.c',
    'src/db.c',
    'src/output_data.c',
    'src/ls.c',
    'src/parse_directory.c',
    install: false,
    dependencies: ffmpeg + spatialite,
)

executable(
    'debug_video',
    'src/glyph.c',
    'src/video_data.c',
    'src/output_data.c',
    'src/db.c',
    'src/debug_video.c',
    install: false,
    dependencies: ffmpeg + spatialite,
)

subdir('test')
