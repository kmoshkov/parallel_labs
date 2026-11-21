import csv
import matplotlib.pyplot as plt

data = {}
with open("results.csv", encoding="utf-8") as f:
    rows = list(csv.reader(f, delimiter=";"))[1:]

for r in rows:
    key = (int(r[0]), int(r[1]), int(r[2]))
    data.setdefault(key, []).append((float(r[3]), float(r[5])))

sizes = sorted(set(k[0] for k in data))
blocks = sorted(set((k[1], k[2]) for k in data), key=lambda b: (b[0] * b[1], b[0]))
names = ["%dx%d" % b for b in blocks]

time = {}
gflops = {}
for key, v in data.items():
    time[key] = sum(x[0] for x in v) / len(v)
    gflops[key] = sum(x[1] for x in v) / len(v)

out = open("results_avg.csv", "w", encoding="utf-8")
out.write("Размер матрицы (NxN);Блок (X x Y);Размер сетки;Время умножения (с);Количество операций (FLOP);Производительность (GFLOPS)\n")
for n in sizes:
    for b in blocks:
        grid = "%dx%d" % ((n + b[0] - 1) // b[0], (n + b[1] - 1) // b[1])
        out.write("%d;%dx%d;%s;%.6f;%d;%.4f\n" % (n, b[0], b[1], grid, time[(n, b[0], b[1])], 2 * n ** 3, gflops[(n, b[0], b[1])]))
out.close()

plt.figure(figsize=(9, 5))
for n in sizes:
    plt.plot(names, [time[(n, b[0], b[1])] for b in blocks], marker="o", label="N = " + str(n))
plt.title("Время умножения")
plt.xlabel("Размер блока (X x Y)")
plt.ylabel("Время (с)")
plt.yscale("log")
plt.legend()
plt.grid()
plt.savefig("time.png", dpi=200)

plt.figure(figsize=(9, 5))
for n in sizes:
    plt.plot(names, [gflops[(n, b[0], b[1])] for b in blocks], marker="o", label="N = " + str(n))
plt.title("Производительность")
plt.xlabel("Размер блока (X x Y)")
plt.ylabel("Производительность (GFLOPS)")
plt.legend()
plt.grid()
plt.savefig("gflops.png", dpi=200)

plt.figure(figsize=(9, 5))
for b, name in zip(blocks, names):
    plt.plot(sizes, [gflops[(n, b[0], b[1])] for n in sizes], marker="o", label=name)
plt.title("Производительность в зависимости от размера матрицы")
plt.xlabel("Размер матрицы (N)")
plt.ylabel("Производительность (GFLOPS)")
plt.legend()
plt.grid()
plt.savefig("gflops_by_size.png", dpi=200)
