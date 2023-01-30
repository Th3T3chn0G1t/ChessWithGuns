import matplotlib.pyplot as plt
from csv import *
import math

f = open('game.csv', 'r')
r = reader(f)

black_health = []
black_damage = []
white_health = []
white_damage = []
dist = []

mean_black_health = 0
mean_black_damage = 0
mean_dist = 0
n = 0

for row in r:
    print(row)
    try:
        black_health.append(float(row[0]))
        black_damage.append(float(row[1]))
        white_health.append(float(row[2]))
        white_damage.append(float(row[3]))
        n += 1
        mean_black_health += float(row[0])
        mean_black_damage += float(row[1])
        if math.isfinite(float(row[4])):
            mean_dist += float(row[4])
            dist.append(float(row[4]) * 10)
        else:
            dist.append(0)
    except Exception as e:
        print('bad', e)
        pass

print(f'Mean Black Damage (100 - White Health): {round(mean_black_damage / n)}')
print(f'Mean Black Health (100 - White Damage): {round(mean_black_health / n)}')
print(f'Mean Player Distance: {round(mean_dist / n)}')

plt.plot(black_health)
plt.plot(black_damage)
plt.plot(dist)
plt.show()
