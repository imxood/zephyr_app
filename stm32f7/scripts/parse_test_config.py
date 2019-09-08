import os, sys
import re

SCRIPT_DIR = os.path.realpath(os.path.dirname(__file__))
PROJECT_DIR = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))

prj_file = 'prj.conf'
parse_prj_files = ['prj1.conf', 'prj2.conf']

COMMENT = re.compile(r'^[^\w]*#+')
CONFIG_ITEM = re.compile(r'^CONFIG_\w+=[xy]$')



ADD_GLOB_SOURCES_IF_FLAG = '^add_glob_sources_if'


cmake_files = ['CMakeLists.txt']

# parse all proj.conf path
for cmake_file in cmake_files:

    if not os.path.isabs(cmake_file):
        cmake_file = os.path.join(PROJECT_DIR, cmake_file)

    if not os.path.exists(cmake_file):
        print('the file {} does not exist', cmake_file)
        sys.exit()

    with open(cmake_file, 'r') as cmake_f:

        for line in cmake_f.readlines():

            ADD_GLOB_SOURCES_IF_FLAG


# parse all opened config item

if not os.path.isabs(prj_file):
    prj_file = os.path.join(PROJECT_DIR, prj_file)

if not os.path.exists(prj_file):
    print('the file {} does not exist', prj_file)
    sys.exit()

proj_items = dict()

with open(prj_file, 'r') as prj_f:

    for line in prj_f.readlines():

        line = line.strip()

        if len(line) == 0:
            continue

        if COMMENT.match(line):
            continue

        if CONFIG_ITEM.match(line):
            items = line.split('=')
            proj_items[items[0]] = items[1]


print(proj_items)

for parse_file in parse_prj_files:

    if not os.path.isabs(parse_file):
        parse_file = os.path.join(PROJECT_DIR, parse_file)

    if not os.path.exists(parse_file):
        print('the file {} does not exist', parse_file)
        sys.exit()

    print('the file {} is parsed', parse_file)
