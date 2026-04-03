Import("env")

import atexit
import os
import shutil


PROJECT_DIR = env.subst("$PROJECT_DIR")
FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoteensy")
CORE_DIR = os.path.join(FRAMEWORK_DIR, "cores", "teensy4")
OVERRIDE_DIR = os.path.join(PROJECT_DIR, "teensy_core_override", "teensy4")
BACKUP_DIR = os.path.join(PROJECT_DIR, ".pio", "teensy_core_backup", "teensy4")
FILES = ("usb_mouse.c", "usb_desc.c")


def _copy_file(source, destination):
    os.makedirs(os.path.dirname(destination), exist_ok=True)
    shutil.copy2(source, destination)


def _restore_core_files():
    for name in FILES:
        backup = os.path.join(BACKUP_DIR, name)
        target = os.path.join(CORE_DIR, name)
        if os.path.exists(backup):
            shutil.copy2(backup, target)


for name in FILES:
    source = os.path.join(OVERRIDE_DIR, name)
    target = os.path.join(CORE_DIR, name)
    backup = os.path.join(BACKUP_DIR, name)

    if not os.path.exists(source):
        raise RuntimeError("Missing Teensy override file: %s" % source)

    _copy_file(target, backup)
    _copy_file(source, target)


atexit.register(_restore_core_files)
