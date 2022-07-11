import csv

def read_data(folder):
    lec_dict = dict()
    teach_dict = dict()
    with open(f'{folder}/lectures.csv', newline='', encoding='utf8') as lecture_file:
        lecture = list(csv.reader(lecture_file))
        for i in range(len(lecture)):
            lec_dict[lecture[i][0]] = i
            lecture[i] = lecture[i][1:]
            lecture[i][0] = int(lecture[i][0])
            lecture[i][1] = int(lecture[i][1])
            lecture[i][2] = (lecture[i][2] == "True")
    with open(f'{folder}/students.csv', newline='', encoding='utf8') as student_file:
        student = list(csv.reader(student_file))
        for i in range(len(student)):
            student[i] = student[i][2:]
            for j in range(len(student[i])):
                student[i][j] = lec_dict[student[i][j]]
    with open(f'{folder}/teachers.csv', newline='', encoding='utf8') as teacher_file:
        teacher = list(csv.reader(teacher_file))
        for i in range(len(teacher)):
            teacher[i] = teacher[i][1:]
            for j in range(0, len(teacher[i]), 2):
                teacher[i][j] = lec_dict[teacher[i][j]]
                teacher[i][j+1] = int(teacher[i][j+1])
    return lecture, student, teacher
