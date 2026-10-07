import numpy

a = numpy.loadtxt("matrix_a.txt")
b = numpy.loadtxt("matrix_b.txt")
cpp_result = numpy.loadtxt("result.txt")

python_result = a @ b

if numpy.allclose(cpp_result, python_result, atol=1e-10):
    print("Проверка пройдена.")
else:
    print("Проверка не пройдена.")