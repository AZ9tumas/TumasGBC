# Debugger to reduce the pain when debugging the insane output given by the emulators ...
import os
import subprocess
import sys

print("pwd:",os.getcwd())

TUMAS_FILE_NAME = "tumasdebug.txt"

tumasgbc_debug_file = open(TUMAS_FILE_NAME, "w+")

try:
    subprocess.call(["./gbc", sys.argv[1]], stdout = tumasgbc_debug_file, stderr = tumasgbc_debug_file, timeout = 5)
except:
    print("Failed to execute executable files")

tumasgbc_debug_file.close()

tumasOutput = open(TUMAS_FILE_NAME, "r")
tumasLines = tumasOutput.readlines()

tumasOutput.close()

#tumasLines.replace("\\n", "\n") 

print(*tumasLines)

os.remove(TUMAS_FILE_NAME)
