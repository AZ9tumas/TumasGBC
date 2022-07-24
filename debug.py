# Debugger to reduce the pain when debugging the insane output given by the emulators ...
import os
import subprocess
import sys
import time

tumasOutput = open("log.txt", "r")
tumasLines = tumasOutput.readlines()

logOutput = open("log2.txt", "r")
logLines = logOutput.readlines()

print("Length of TUMASGBC:", len(tumasLines))
print("Length of DEBUGGER:", len(logLines))

print("\n")

ignoreFlags = input("Ignore flags? (y/n)") == 'y'
ignoreRegs = input("Ignore Registers? (y/n)") == 'y'

del logLines[0]
del logLines[0]

del tumasLines[len(tumasLines)-1]
del tumasLines[len(tumasLines)-1]

while True:

    FlagErrors = []
    RegErrors = []

    i = int(input("Enter initial val: ")) * 2
    final = int(input("Enter final val: ")) * 2

    while i + 1 < len(tumasLines) and i < final:
        print(f"\n<== DISPATCH NO {i//2} ==>\n")
        # TUMASGBC
        print("Tumas ::")
        print(tumasLines[i],end='')
        print(tumasLines[i+1])

        #DEBUGGER
        print("Debug ::")
        print(logLines[i],end='')
        print(logLines[i+1])

        if not ignoreRegs:
            if tumasLines[i] == logLines[i]:
                print("REGS: true")
            else:
                print("REGS: false")
                RegErrors.append(i//2)

        if not ignoreFlags:
            if tumasLines[i+1] == logLines[i+1]:
                print("FLAG & ADDRESS: true")
            else:
                print("FLAG & ADDRESS: false")
                FlagErrors.append(i//2)

        i+=2
    if not ignoreRegs:
        print("Reg Errors: ", *RegErrors)
    if not ignoreFlags:
        print("Flag Errors: ", *FlagErrors)

tumasOutput.close()
logOutput.close()
