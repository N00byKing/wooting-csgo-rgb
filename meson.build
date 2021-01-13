project('wooting-csgo-rgb', 'cpp')

httpdep = dependency('httplib')
wootdep = dependency('libwooting-rgb-sdk')
jsondep = dependency('RapidJSON')

sources = ['main.cpp', 'handler.cpp']

executable('wooting-csgo-rgb', sources, dependencies: [httpdep, wootdep, jsondep])
