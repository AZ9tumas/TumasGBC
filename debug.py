# Debugger to reduce the pain when debugging the insane output given by the emulators ...
import os
import subprocess
import sys
import time

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def clear_terminal(): os.system("clear")

tumasOutput = open("log.txt", "r")
tumasLines = tumasOutput.readlines()

logOutput = open("log2.txt", "r")
logLines = logOutput.readlines()

print("Length of TUMASGBC:", len(tumasLines))
print("Length of DEBUGGER:", len(logLines))

print("\n")

leaveDebug = False

ignoreFlags = input("Ignore flags? (y/n) ") == 'y'
ignoreRegs = input("Ignore Registers? (y/n) ") == 'y'

if ignoreRegs == 'exit' or ignoreFlags == 'exit': leaveDebug = True

del logLines[0]
del logLines[0]

del tumasLines[len(tumasLines)-1]
del tumasLines[len(tumasLines)-1]

while True:

    if leaveDebug: break

    FlagErrors = []
    RegErrors = []
    errorFixes = []

    foundFlagmismatch = False
    foundRegmismatch = False
    
    try:
        i = int(input("Enter initial val: ")) * 2
        final = int(input("Enter final val: ")) * 2
    except:
        print("Error in figuring out the entered initial / final values")
        break

    clear_terminal()

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
                if foundRegmismatch: errorFixes.append(f"Register error at {(i-2)//2} was fixed at {i//2}")
                foundRegmismatch = False
            else:
                print("REGS: false")
                RegErrors.append(i//2)
                foundRegmismatch = True

        if not ignoreFlags:
            if tumasLines[i+1] == logLines[i+1]:
                print("FLAG & ADDRESS: true")
                if foundFlagmismatch: errorFixes.append(f"Flag error at {(i-2)//2} was fixed at {i//2}")
                foundFlagmismatch = False
            else:
                print("FLAG & ADDRESS: false")
                FlagErrors.append(i//2)
                foundFlagmismatch = True

        i+=2
    if not ignoreRegs:
        print("\nReg Errors: ", *RegErrors)
        
    if not ignoreFlags:
        print("\nFlag Errors: ", *FlagErrors)

    print("\nError fixes: ", *errorFixes, sep = "\n")

tumasOutput.close()
logOutput.close()
