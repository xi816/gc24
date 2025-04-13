#!/usr/bin/python3
import time

print("Welcome to BadOS 1.0,loading...")
time.sleep(.3)
print("Starting core....")
time.sleep(.5)
print("Loading strings.....")
time.sleep(.4)
print("System is ready to work!")
time.sleep(.1)
while (1):
  command = input("Enter your command: ")
  if command == "help":
      print("data,bdfetch")
  elif command == "bdfetch":
      print("CPU = Bdintel")
      print("GPU = Bdintel UHD graphics 620")
      print("RAM = 64GB")
      print("Hostname = user")
      print("Disk = 345GB left out of 1000GB")

