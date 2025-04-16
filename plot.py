import matplotlib.pyplot as plt
import csv
import sys

if len(sys.argv) != 2:
    print("Usage: python3 plot.py [multi|shared]")
    sys.exit(1)

command = sys.argv[1]

if command == "multi":
    threads = []
    speedups = []

    with open("multi.txt", "r") as file:
        reader = csv.DictReader(file)
        for row in reader:
            threads.append(int(row["Threads"]))
            speedups.append(float(row["SpeedUp"]))

    plt.figure(figsize=(10, 6))
    plt.plot(threads, speedups, marker='o', linestyle='-', color='green')
    plt.title("Speedup of Multi-threaded Monte Carlo Pi Estimation")
    plt.xlabel("Number of Threads")
    plt.ylabel("Speedup")
    plt.grid(True)
    plt.xticks(threads)
    plt.tight_layout()
    plt.show()

elif command == "shared":
    npoints = []
    times = []

    with open("shared.txt", "r") as file:
        reader = csv.DictReader(file)
        for row in reader:
            npoints.append(int(row["nPoints"]))
            times.append(float(row["Execution_Time"]))

    plt.figure(figsize=(10, 6))
    plt.plot(npoints, times, marker='o', linestyle='-', color='blue')
    plt.title("Execution Time of Shared Variable Monte Carlo Pi Estimation")
    plt.xlabel("Number of Points")
    plt.ylabel("Execution Time (seconds)")
    plt.grid(True)
    plt.xticks(npoints)
    plt.tight_layout()
    plt.show()

else:
    print("Invalid command. Use 'multi' or 'shared'.")
    sys.exit(1)
