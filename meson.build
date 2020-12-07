project('wooting-csgo-rgb', 'cpp')

pkg_config_path = ('/usr/local/lib/pkgconfig')

httpdep = dependency('httplib')
wootdep = dependency('libwooting-rgb-sdk')
jsondep = dependency('jsoncpp')

sources = ['main.cpp', 'handler.cpp']
include = ['handler.hpp']

executable('wooting-csgo-rgb', sources, dependencies: [httpdep, wootdep, jsondep])