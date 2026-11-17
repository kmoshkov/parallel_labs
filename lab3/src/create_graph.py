import csv
import matplotlib.pyplot as plt

data = {}
with open("results.csv", encoding="utf-8") as f:
    rows = list(csv.reader(f, delimiter=";"))[1:]

for r in rows:
    key = (int(r[0]), int(r[1]))
    data.setdefault(key, []).append((float(r[2]), float(r[4])))

sizes = sorted(set(k[0] for k in data))
cores = sorted(set(k[1] for k in data))

time = {}
gflops = {}
for key, v in data.items():
    time[key] = sum(x[0] for x in v) / len(v)
    gflops[key] = sum(x[1] for x in v) / len(v)

out = open("results_avg.csv", "w", encoding="utf-8")
out.write("Размер матрицы (NxN);Количество процессов;Время умножения (с);Количество операций (FLOP);Производительность (GFLOPS);Ускорение;Эффективность\n")
for n in sizes:
    for p in cores:
        s = time[(n, 1)] / time[(n, p)]
        out.write("%d;%d;%.6f;%d;%.4f;%.3f;%.3f\n" % (n, p, time[(n, p)], 2 * n ** 3, gflops[(n, p)], s, s / p))
out.close()

plt.figure()
for n in sizes:
    plt.plot(cores, [time[(n, 1)] / time[(n, p)] for p in cores], marker="o", label="N = " + str(n))
plt.plot(cores, cores, "k--", label="Идеальное ускорение")
plt.title("Ускорение")
plt.xlabel("Количество процессов")
plt.ylabel("Ускорение")
plt.legend()
plt.grid()
plt.savefig("speedup.png", dpi=200)

plt.figure()
for n in sizes:
    plt.plot(cores, [time[(n, 1)] / time[(n, p)] / p for p in cores], marker="o", label="N = " + str(n))
plt.title("Эффективность")
plt.xlabel("Количество процессов")
plt.ylabel("Эффективность")
plt.legend()
plt.grid()
plt.savefig("efficiency.png", dpi=200)

plt.figure()
for n in sizes:
    plt.plot(cores, [gflops[(n, p)] for p in cores], marker="o", label="N = " + str(n))
plt.title("Производительность")
plt.xlabel("Количество процессов")
plt.ylabel("Производительность (GFLOPS)")
plt.legend()
plt.grid()
plt.savefig("gflops.png", dpi=200)
