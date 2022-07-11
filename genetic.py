from random import randrange, choice
from _collections import defaultdict

from shared import read_data

class Timetable:
    def __init__(self, parents=None):
        """
        s: 2d list of pairs of int, (#student)x([lecture,class])
        t: 2d list of pairs of int and list, (#teacher)x([lecture,[classes]])
        l: 3d list, (#lecture)x(#classes for lecture)x(class count for class)

        If has parents, created as a crossover
        If not, completely random
        """
        self.s = [[] for _ in range(len(student))]
        for i in range(len(student)):
            for j in range(len(student[i])):
                lec = student[i][j]
                self.s[i].append([lec, choice(parents).s[i][j][1] if parents else randrange(lecture[lec][0])])
        lecture_go = []
        for i in range(len(lecture)):
            lecture_go.append([])
        for i in range(len(teacher)):
            for j in range(0, len(teacher[i]), 2):
                for k in range(teacher[i][j+1]):
                    lecture_go[teacher[i][j]].append(i)
        self.t = [defaultdict(list) for _ in range(len(teacher))]
        for i in range(len(lecture_go)):
            for j in range(len(lecture_go[i])):
                self.t[lecture_go[i][j]][i].append(j)
        self.l = [[[choice(parents).l[i][j][k] if parents else randrange(period)
                    for k in range(lecture[i][1])]
                   for j in range(lecture[i][0])]
                  for i in range(len(lecture))]

    def __str__(self):
        s = ''
        s += f'Student: {self.s}\n'
        s += f'Teacher: {self.t}\n'
        s += f'Lecture: {self.l}\n'
        s += f'Loss: {self.loss()}\n'
        return s

    def loss(self):
        l = 0
        for i in range(len(self.s)):
            count = [0]*period
            for j in range(len(self.s[i])):
                lec = self.s[i][j][0]
                cl = self.s[i][j][1]
                for time in self.l[lec][cl]:
                    count[time] += 1
            for coll in count:
                l += coll*(coll-1)
        for i in range(len(self.t)):
            count = [0]*period
            for lec in self.t[i]:
                for cl in self.t[i][lec]:
                    for time in self.l[lec][cl]:
                        count[time] += 1
            for coll in count:
                l += coll*(coll-1)
        return l

    def student_random(self, idx, lec):
        self.s[idx][lec][1] = randrange(lecture[student[idx][lec]][0])

    def lecture_random(self, idx, cls, time):
        self.l[idx][cls][time] = randrange(period)

    def mutate(self):
        while True:
            if randrange(2) == 0:
                idx = randrange(len(self.s))
                lec = randrange(len(self.s[idx]))
                self.student_random(idx, lec)
            else:
                idx = randrange(len(self.l))
                cls = randrange(len(self.l[idx]))
                time = randrange(len(self.l[idx][cls]))
                self.lecture_random(idx, cls, time)
            if randrange(2) == 0:
                break

class Optimizer:
    def __init__(self):
        self.table = Timetable()

    def __str__(self):
        return str(self.table)

    def create_mutation(self, offspring):
        choices = [Timetable(parents=[self.table]) for _ in range(offspring)]
        for child in choices:
            child.mutate()
        choices.append(self.table)
        new_loss = [[child.loss(), child] for child in choices]
        new_loss.sort(key=lambda x:x[0])
        return new_loss

    def cross_mutate(self, offspring, parent):
        choices = self.create_mutation(offspring)
        parents = [choices[i][1] for i in range(parent)]
        self.table = Timetable(parents=parents)

lecture, student, teacher = read_data('CSV_files/2022-1')
period = 35
optim = Optimizer()
i = 0
while True:
    if i%100 == 0:
        print(f'Epoch: {i}')
        print(optim)
    optim.cross_mutate(1, 1)
    i += 1
