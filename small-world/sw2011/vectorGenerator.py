import random
for i in range(100000):
    line = str(i) + "\t"
    for d in range(32):
        line += str(d) + "\t" + str(random.random()) + "\t"
    print(line)
