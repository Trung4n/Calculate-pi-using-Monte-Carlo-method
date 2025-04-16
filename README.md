
```markdown
# Pi Estimation with Multithreading and Synchronization Strategies

This project estimates the value of Pi using the Monte Carlo method with multithreading in C. It demonstrates the performance impact of different synchronization techniques for shared variables in a concurrent environment.

## 🧠 Project Structure

- `approach.c`: The main C source code implementing three approaches:
  - **Single-threaded**
  - **Multi-threaded (non-shared)**
  - **Multi-threaded with shared variable**
- `Makefile`: Supports different synchronization strategies.
- `plot.py`: Python script for plotting speedup and runtime comparisons.
- `multi.txt`, `shared.txt`: Output data files used for plotting.

---

## 🛠️ Compilation (Using `make`)

You can compile the program using different strategies by calling:

- `make`: Default build — compiles **Approach 3 (shared variable)** using `MUTEX` synchronization.
- `make threadlocal`: Uses `MUTEX` combined with **Thread-local Reduction** to minimize synchronization overhead.
- `make atomic`: Uses **C11 atomic operations** to update the shared variable without mutex locks.
- `make freelock`: Uses **lock-free technique** with `__sync_bool_compare_and_swap`.

These macros control which synchronization method is compiled into the binary. Do **not manually define macros in the code** — all are handled via the Makefile.

---

## 🚀 Running the Program

```bash
./approach <approach_type> <nThreads> <nPointsMultiplier>
```

### Parameters

- `<approach_type>`: One of the following:
  - `single`: Run the estimation with a single thread.
  - `multi`: Run with multiple threads **without shared variables**. Each thread keeps a private count.
  - `shared`: Run with multiple threads **using shared variables**. Synchronization depends on the build (mutex, atomic, etc.)
  
- `<nThreads>`: Number of threads (required for `multi` and `shared`).

- `<nPointsMultiplier>`: A multiplier applied to the base number of points (10,000,000). The loop runs from `TOTAL_POINTS` up to `TOTAL_POINTS * nPointsMultiplier`.

### Examples

```bash
make freelock
./approach shared 100 10
```

- Compiles the program using the **lock-free strategy**.
- Executes **Approach 3** with 100 threads and scales the total number of points from 10 million to 100 million (in increments).

```bash
make threadlocal
./approach shared 8 5
```

- Uses **thread-local storage** to reduce mutex contention.
- Each thread performs partial updates at intervals, reducing synchronization overhead.

---

## 📊 Output & Visualization

- The program writes performance results into `multi.txt` and `shared.txt`.
- `plot.py` is automatically invoked to generate comparative plots after each execution.

---

## 📘 Synchronization Strategies Explained

| Strategy              | Macro                 | Description                                                                 |
|-----------------------|-----------------------|-----------------------------------------------------------------------------|
| `Mutex`               | `-DMUTEX`             | Basic mutex lock for each update. Easy to implement but high contention.    |
| `Thread-local`        | `-DMUTEX -DREDUCE_THREAD_LOCAL` | Each thread accumulates counts locally and syncs only periodically. |
| `Atomic`              | `-DREDUCE_ATOMIC`     | Uses C11 atomic operations (`atomic_fetch_add`) for lock-free safety.       |
| `Lock-free` (CAS)     | `-DREDUCE_LOCK_FREE`  | Uses compare-and-swap (`__sync_bool_compare_and_swap`) for lock-free update.|

---

## 📎 Notes

- Adjust `TOTAL_POINTS` in the code for different baseline workloads.
- Requires `gcc` and `pthread` for compilation.
- Requires Python 3 for plotting.

---

## ✅ Example Use Case

Estimate Pi using 100 threads, comparing the overhead of mutex vs. lock-free:

```bash
make
./approach shared 100 10

make freelock
./approach shared 100 10
```

Use the resulting plots to evaluate how different synchronization techniques impact performance and scalability.

---
```
