#!/usr/bin/env python3

import os
import sys
import argparse
import json
import re
import serial
import subprocess as sp

# PROJECT = "samples/hello_world"

PROJECT = "zephyr_app/stm32f7"
# PROJECT = "tests/kernel/fifo/fifo_api"
BOARD = "stm32f767"

GENERATOR = "Eclipse CDT4 - Ninja"

# PROJECT = "zephyr_app/esp32"
# BOARD = "esp32"

# PROJECT = "tests/kernel/mem_protect/mem_protect"
# PROJECT = "samples/hello_world"
HOME = os.environ.get("HOME")

ZEPHYR_PROJECT = "{}/develop/sources/zephyrproject".format(HOME)
ZEPHYR_BASE = "{}/zephyr".format(ZEPHYR_PROJECT)

WORKSPACE = ZEPHYR_PROJECT

OUTPUT = os.path.join(WORKSPACE, 'builds')


Env = {
    'ZEPHYR_BASE': ZEPHYR_BASE,
    'ZEPHYR_SDK_INSTALL_DIR': '{}/programs/zephyr-sdk'.format(HOME),
    'ZEPHYR_TOOLCHAIN_VARIANT': 'zephyr',
    'PATH': '/home/maxu/programs/BullseyeCoverage/bin:/home/maxu/programs/zephyr-sdk/arm-zephyr-eabi/bin:{}'.format(os.environ['PATH'])
}

if BOARD == 'esp32':
    Env['ZEPHYR_TOOLCHAIN_VARIANT'] = 'espressif'
    Env['ESPRESSIF_TOOLCHAIN_PATH'] = '{}/programs/xtensa-esp32-elf'.format(HOME)
    Env['ESP_IDF_PATH'] = '{}/develop/sources/esp-idf-v3.1.3'.format(HOME)


RunEnv = {
    'SCRIPT_FILE': os.path.realpath(__file__),
    'SCRIPT_DIR': os.path.dirname(os.path.realpath(__file__))
}

if not OUTPUT.startswith("/"):
    OUTPUT = os.path.realpath('{}/../{}'.format(RunEnv['SCRIPT_DIR'], OUTPUT))

print("OUTPUT: ", OUTPUT)

VERBOSE = True


C_CPP_PROPERTIES = '''
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [],
            "browse": {
                "limitSymbolsToIncludedHeaders": false
            },
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "clang-x64"
        }
    ],
    "version": 4
}
'''

SETTINGS_TEMPLATE = '''
{
	"editor.fontSize": 16,
    "terminal.integrated.fontSize": 16,
    "editor.mouseWheelZoom": true,
    "editor.renderWhitespace": "all"
}
'''

LAUNCH_TEMPLATE = '''
{
    "version": "0.2.0",
	"configurations": []
}
'''

LAUNCH_CORTEX_DEBUG = '''
{
    "type": "cortex-debug",
    "request": "launch",
    "servertype": "openocd",
    "cwd": "${workspaceRoot}",
    "executable": "./zephyr_output/zephyr/zephyr.elf",
    "name": "Stm32F7 Debug",
    "runToMain": true,
    "configFiles": [
        "${workspaceRoot}/zephyr_app/stm32f7/board/support/openocd.cfg"
    ]
}
'''

args = None

# 创建一个xstr类，用于处理从文件中读出的字符串
class xstr:

    def __init__(self, instr):
        self.instr = instr

    # 删除“//”标志后的注释
    def rmCmt(self):
        qtCnt = cmtPos = slashPos = 0
        rearLine = self.instr
        # rearline: 前一个“//”之后的字符串，
        # 双引号里的“//”不是注释标志，所以遇到这种情况，仍需继续查找后续的“//”
        while rearLine.find('//') >= 0: # 查找“//”
            slashPos = rearLine.find('//')
            cmtPos += slashPos
            # print 'slashPos: ' + str(slashPos)
            headLine = rearLine[:slashPos]
            while headLine.find('"') >= 0: # 查找“//”前的双引号
                qtPos = headLine.find('"')
                if not self.isEscapeOpr(headLine[:qtPos]): # 如果双引号没有被转义
                    qtCnt += 1 # 双引号的数量加1
                headLine = headLine[qtPos+1:]
                # print qtCnt
            if qtCnt % 2 == 0: # 如果双引号的数量为偶数，则说明“//”是注释标志
                # print self.instr[:cmtPos]
                return self.instr[:cmtPos]
            rearLine = rearLine[slashPos+2:]
            # print rearLine
            cmtPos += 2
        # print self.instr
        return self.instr

    # 判断是否为转义字符
    def isEscapeOpr(self, instr):
        if len(instr) <= 0:
            return False
        cnt = 0
        while instr[-1] == '\\':
            cnt += 1
            instr = instr[:-1]
        if cnt % 2 == 1:
            return True
        else:
            return False

def loadJson(JsonPath):
    try:
        srcJson = open(JsonPath, 'r')
    except:
        print('cannot open ' + JsonPath)
        quit()

    dstJsonStr = ''
    for line in srcJson.readlines():
        if not re.match(r'\s*//', line) and not re.match(r'\s*\n', line):
            xline = xstr(line)
            dstJsonStr += xline.rmCmt()

    # print dstJsonStr
    dstJson = None
    try:
        dstJson = json.loads(dstJsonStr)
    except:
        print(JsonPath + ' is not a valid json file')
    return dstJson


def get_real_path(output, path):
    return path if os.path.isabs(path) else os.path.realpath(output + '/' + path)


def initEnv():
    """
        1. 创建tool工具在zephyr目录下
    """
    cmd = "ln -sf {} /home/maxu/develop/sources/zephyrproject/zephyr/tool".format(RunEnv["SCRIPT_FILE"])
    os.system(cmd)

def update_c_cpp_properties():

    compile_file = "{}/compile_commands.json".format(OUTPUT)

    defines = set()
    includePath = set()
    forcedInclude = set()

    if os.path.exists(compile_file):
        data = loadJson(compile_file)
        for compile_item in data:
            if os.path.splitext(compile_item['file'])[-1].lower() == ".s":
                continue
            command = compile_item['command']
            defines = defines | set(map(lambda x: x[2:], filter(lambda x: x.startswith('-D'), command.split())))
            includePath = includePath | set(filter(lambda x: x.startswith(ZEPHYR_BASE) is False, map(lambda x: get_real_path(OUTPUT, x[2:]), filter(lambda x: x.startswith('-I'), command.split()))))

    for path in ('zephyr/include/generated/autoconf.h', 'zephyr/include/generated/generated_dts_board_fixups.h', 'zephyr/include/generated/generated_dts_board_unfixed.h'):
        path = get_real_path(OUTPUT, path)
        forcedInclude.add(path)

    autoconf_file = "{}/zephyr/include/generated/autoconf.h".format(OUTPUT)

    if os.path.exists(autoconf_file):
        with open(autoconf_file, 'r') as f:
            for line in f.readlines():
                parts = line.split()
                if len(parts) == 3:
                    defines.add("{}={}".format(parts[1], parts[2]))

    if len(defines) == 0:
        return

    config_file = "{}/.vscode/c_cpp_properties.json".format(WORKSPACE)

    data = json.loads(C_CPP_PROPERTIES)

    json_defines = data['configurations'][0]['defines']
    json_defines.extend(defines)

    json_includePath = data['configurations'][0]['includePath']
    json_includePath.extend(includePath)

    # json_browsePath = data['configurations'][0]['browse']['path']
    # json_browsePath.extend(includePath)

    # json_forcedInclude = data['configurations'][0]['forcedInclude']
    # json_forcedInclude.extend(forcedInclude)

    # data['configurations'][0]['compileCommands'] = "{}/compile_commands.json".format(OUTPUT)

    with open(config_file, 'w') as f:
        json.dump(data, f, indent=4, ensure_ascii=False)


def update_settings():

    settings_data = None
    settings_file = "{}/.vscode/settings.json".format(ZEPHYR_BASE)

    if os.path.exists(settings_file):
        try:
            settings_data = loadJson(settings_file)
        except json.decoder.JSONDecodeError as err:
            print("json load file '{}' Error: {}".format(settings_file, err))

    settings_configue = json.loads(SETTINGS_TEMPLATE)

    if settings_data == None:
        settings_data = settings_configue
    else:
        settings_data.update(settings_configue)

    with open(settings_file, 'w') as f:
        json.dump(settings_data, f, indent=4, ensure_ascii=False)


def update_launch():

    launch_data = None
    launch_file = "{}/.vscode/launch.json".format(ZEPHYR_BASE)

    if os.path.exists(launch_file):
        try:
            launch_data = loadJson(launch_file)
        except json.decoder.JSONDecodeError as err:
            print("json load file '{}' Error: {}".format(launch_file, err))

    if launch_data == None:
        launch_data = json.loads(LAUNCH_TEMPLATE)

    cortex_debug_configuration = json.loads(LAUNCH_CORTEX_DEBUG)

    if len(launch_data) == 0:
        launch_data['configurations'] = [cortex_debug_configuration]

    need_append = True

    for configuration in launch_data['configurations']:
        if configuration.get("name") == cortex_debug_configuration["name"]:
            configuration.clear()
            configuration.update(cortex_debug_configuration)
            need_append = False
            break

    if need_append:
        launch_data['configurations'].append(cortex_debug_configuration)

    with open(launch_file, 'w') as f:
        json.dump(launch_data, f, indent=4, ensure_ascii=False)


def update():

    vscode_dir = "{}/.vscode".format(WORKSPACE)

    if not os.path.exists(vscode_dir):
        os.makedirs(vscode_dir)

    update_launch()

    update_settings()

    update_c_cpp_properties()


def config():

    global args

    if args.generator:

        if args.generator == 'Eclipse':
            generator = 'Eclipse CDT4 - Ninja'
        else:
            generator = args.generator
    else:
        generator = GENERATOR

    PROJECT_HOME = '{}/{}'.format(WORKSPACE, PROJECT)

    cmd = 'cmake --debug -H{} -B{} -G"{}" -DCMAKE_BUILD_TYPE=Debug -DBOARD={} -DCMAKE_EXPORT_COMPILE_COMMANDS=1'
    # -DCMAKE_VERBOSE_MAKEFILE=ON
    cmd = cmd.format(PROJECT_HOME, OUTPUT, generator, BOARD)

    boards_dir = os.path.join(PROJECT_HOME, 'boards')
    soc_dir = os.path.join(PROJECT_HOME, 'soc')

    if os.path.exists(boards_dir):
        cmd = cmd + ' -DBOARD_ROOT=' + PROJECT_HOME

    if os.path.exists(soc_dir):
        cmd = cmd + ' -DSOC_ROOT=' + PROJECT_HOME


    print("cmd: " + cmd)

    return os.system(cmd)

def menuconfig():
    cmd = 'ninja -C {} menuconfig'.format(OUTPUT)
    print("cmd: " + cmd)
    return os.system(cmd)

def build():
    cmd = 'ninja -v' if VERBOSE else "ninja"
    cmd = cmd + " -C {}".format(OUTPUT)
    print("cmd: " + cmd)
    return os.system(cmd)


def clean():
    cmd = 'rm -rf {}'.format(OUTPUT)
    print("cmd: " + cmd)
    return os.system(cmd)


def flash():
    cmd = 'ninja -C {} flash'.format(OUTPUT)
    print("cmd: " + cmd)
    return os.system(cmd)

def open_serial():
    cmd = ['putty', '-load', 'stm32f7']
    p = sp.Popen(cmd)
    print(p)
    return 0


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config', action='store_true', help="config project")
    parser.add_argument('-b', '--build', action='store_true', help="build project")
    parser.add_argument('-C', "--clean", action='store_true', help="clean project")
    parser.add_argument('-f', '--flash', action='store_true', help="flash")
    parser.add_argument('-v', '--verbose', action='store_true', help="verbose log")
    parser.add_argument('-m', '--menuconfig', action='store_true', help="menuconfig")
    parser.add_argument('-i', '--init', action='store_true', help="init environment")
    parser.add_argument('-g', '--generator', type=str, help="specify a cmake generator")
    parser.add_argument('-s', '--open_serial', type=str, help="open putty after building seccessfully")

    return parser


def _main():

    global VERBOSE
    global args

    for key in Env:
        os.environ[key] = Env[key]

    parser = parse_arguments()

    args = parser.parse_args()

    print(args)

    if args.verbose:
        VERBOSE = args.verbose

    if args.init:
        initEnv()
        os._exit(-1)

    if args.clean:
        clean()

    if args.config:
        if config():
            os._exit(-1)
        update()

    if args.menuconfig:
        if menuconfig():
            os._exit(-1)
        update()

    if args.build:
        if build():
            os._exit(-1)

    if args.flash:
        if flash():
            os._exit(-1)

    if args.open_serial:
        open_serial()


if __name__ == "__main__":

    _main()
