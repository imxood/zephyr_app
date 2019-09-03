from pyocd.core.helpers import ConnectHelper
from pyocd.flash.loader import FileProgrammer

import logging
logging.basicConfig(level=logging.INFO)

with ConnectHelper.session_with_chosen_probe() as session:

    board = session.board
    target = board.target
    flash = target.memory_map.get_boot_memory()

    # Load firmware into device.
    FileProgrammer(session).program("my_firmware.bin")

    # Reset, run.
    target.reset_and_halt()
    target.resume()

    # Read some registers.
    print("pc: 0x%X" % target.read_core_register("pc"))

    target.step()
    print("pc: 0x%X" % target.read_core_register("pc"))

    target.resume()
    target.halt()
    print("pc: 0x%X" % target.read_core_register("pc"))

    target.reset_and_halt()

    print("pc: 0x%X" % target.read_core_register("pc"))
