import torch
from torch import nn

from shared import read_data

def dot_product_pair(m):
    m = torch.stack(m)
    m = torch.matmul(m, torch.t(m))
    return (torch.sum(m)-torch.sum(torch.diagonal(m)))/2

def loss(student, teacher, lecture):
    """
    Student: list of 2d tensor, (#student)x(#lectures)x(#classes for lecture)
    Probability distribution for each class
    Teacher: 3d, (#teacher)x(#lectures)x(#classes for lecture)
    0 or 1, whether taught or not
    Lecture: list of 3d tensor, (#lecture)x(#classes for lecture)x(class count for class)x(#period)
    Probability distribution for each period
    """
    l = torch.tensor(0, dtype=torch.float)
    for i in range(len(student)):
        matrix = []
        for j in range(len(student[i])):
            matrix.extend(list(torch.matmul(student[i][j], lecture[j])))
        l += dot_product_pair(matrix)
    for i in range(len(teacher)):
        matrix = []
        for j in range(len(teacher[i])):
            for k in range(len(teacher[i][j])):
                if teacher[i][j][k]:
                    matrix.extend(list(lecture[j][k]))
        l += dot_product_pair(matrix)
    return l

def max_pick(student, teacher, lecture):
    student = [x.clone().detach() for x in student]
    lecture = [x.clone().detach() for x in lecture]
    for i in range(len(student)):
        for j in range(len(student[i])):
            if sum(student[i][j]) > 0.5:
                idx = torch.argmax(student[i][j])
                for k in range(len(student[i][j])):
                    if k == idx:
                        student[i][j][k] = 1
                    else:
                        student[i][j][k] = 0
    for i in range(len(lecture)):
        for j in range(len(lecture[i])):
            for k in range(len(lecture[i][j])):
                idx = torch.argmax(lecture[i][j][k])
                for l in range(len(lecture[i][j][k])):
                    if l == idx:
                        lecture[i][j][k][l] = 1
                    else:
                        lecture[i][j][k][l] = 0
    return student, teacher, lecture

def collision(student, teacher, lecture):
    return loss(*max_pick(student, teacher, lecture))

class Model(nn.Module):
    def __init__(self, student_lecture, teacher_num, class_num, lecture_num, period_num):
        super(Model, self).__init__()
        self.student_lecture = student_lecture
        self.teacher_num = teacher_num
        self.class_num = class_num
        self.lecture_num = lecture_num
        self.period_num = period_num
        self.size = 10
        self.result_sizes = []
        for i in range(len(self.student_lecture)):
            for s in self.student_lecture[i]:
                self.result_sizes.append(self.class_num[s])
        for i in range(len(self.class_num)):
            for j in range(self.class_num[i]):
                for k in range(self.lecture_num[i]):
                    self.result_sizes.append(self.period_num)
        self.flatten = nn.Flatten(0)
        self.linear_stack = nn.Sequential(
            nn.Linear((len(self.student_lecture)+self.teacher_num)*sum(self.class_num), self.size),
            nn.LeakyReLU(0.01),
            nn.Linear(self.size, self.size),
            nn.LeakyReLU(0.01),
            nn.Linear(self.size, self.size),
            nn.LeakyReLU(0.01),
            nn.Linear(self.size, sum(self.result_sizes)),
        )
        self.softmax = nn.Softmax(0)

    def forward(self, x):
        x = self.flatten(x)
        x = self.linear_stack(x)
        student = []
        lecture = []
        idx = 0
        for i in range(len(self.student_lecture)):
            student.append([])
            for j in range(len(self.class_num)):
                if j in self.student_lecture[i]:
                    student[-1].append(self.softmax(x[idx:idx+self.class_num[j]]))
                    idx += self.class_num[j]
                else:
                    student[-1].append(torch.zeros(self.class_num[j]))
            student[-1] = torch.stack(student[-1], 0)
        for i in range(len(self.class_num)):
            temp = []
            for j in range(self.class_num[i]):
                temp.append([])
                for k in range(self.lecture_num[i]):
                    temp[-1].append(self.softmax(x[idx:idx+self.period_num]))
                    idx += self.period_num
                temp[-1] = torch.stack(temp[-1], 0)
            lecture.append(torch.stack(temp, 0))
        return student, lecture

#lecture, student, teacher = read_data('CSV_files/2022-1')
lecture = [
    [1, 1, True],
    [1, 1, True],
    [1, 1, True],
]
student = [
    [0, 1, 2],
]
teacher = [
    [0, 1, 1, 1, 2, 1],
]
class_num = [x[0] for x in lecture]
lecture_num = [x[1] for x in lecture]
model = Model(student, len(teacher), class_num, lecture_num, 3)
grid = torch.rand((len(student)+len(teacher)), sum(class_num))
lecture_go = []
for i in range(len(lecture)):
    lecture_go.append([])
for i in range(len(teacher)):
    for j in range(0, len(teacher[i]), 2):
        for k in range(teacher[i][j+1]):
            lecture_go[teacher[i][j]].append(i)
t = []
for i in range(len(teacher)):
    t.append([])
    for j in range(len(lecture_go)):
        t[i].append([])
        for k in range(len(lecture_go[j])):
            if lecture_go[j][k] == i:
                t[i][j].append(1)
            else:
                t[i][j].append(0)
        t[i][j] = torch.tensor(t[i][j])
optimizer = torch.optim.Adam(model.parameters(), lr=1, betas=(0.9, 0.9))
for i in range(10):
    s, l = model(grid)
    loss_val = loss(s, t, l)
    print(loss_val)
    print(collision(s, t, l))
    print(s)
    print(l)
    optimizer.zero_grad()
    loss_val.backward()
    #print(model.linear_stack[0].weight)
    #print(model.linear_stack[0].weight.grad)
    optimizer.step()
