#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <time.h>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

default_random_engine engine(time(NULL));
uniform_int_distribution<int> dist(0, INT_MAX);
//https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
static unsigned long x=dist(engine), y=dist(engine), z=dist(engine);
unsigned long random() {          //period 2^96-1
	unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;
    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;
    return z;
}

//student: 2d list, (#student)x[lectures]
//teacher: 2d list, (#teacher)x[lectures, class_num]
//lecture: 1d list, (#lecture)x(class_num, class_time, consecutive)
vector<vector<int>> student;
vector<vector<int>> teacher;
vector<vector<int>> lecture;
int period;
vector<int> days;
vector<vector<int>> cont{
	{1, 1, 1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 0},
};
/*vector<vector<int>> cont{
	{   1, 1, 0, 1, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 1, 1, 1, 0},
	{1, 1, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 0},
	{1, 1, 1, 0, 1, 1, 0},
};*/
vector<int> flat;
map<int, int> pd;
int max_diff = 10;

void cal_periods() {
	int i, j;
	for(i=0; i<cont.size(); i++) {
		days.push_back(cont[i].size());
	}
	period = 0;
	for(i=0; i<days.size(); i++) {
		for(j=0; j<days[i]; j++) {
			if(cont[i][j]) {
				flat.push_back(period);
			}
			pd[period] = i;
			period++;
		}
	}
}

vector<vector<int>> read_generic_2d(const char* path) {
	int i, j;
	int n, m;
	int in;
	FILE* fp = fopen(path, "r");
	vector<vector<int>> list;
	fscanf(fp, "%d", &n);
	for(i=0; i<n; i++) {
		fscanf(fp, "%d", &m);
		list.push_back({});
		for(j=0; j<m; j++) {
			fscanf(fp, "%d", &in);
			list[i].push_back(in);
		}
	}
	fclose(fp);
	return list;
}

vector<vector<vector<int>>> read_generic_3d(const char* path) {
	int i, j, k;
	int n, m, p;
	int in;
	FILE* fp = fopen(path, "r");
	vector<vector<vector<int>>> list;
	fscanf(fp, "%d", &n);
	for(i=0; i<n; i++) {
		fscanf(fp, "%d", &m);
		list.push_back({});
		for(j=0; j<m; j++) {
			fscanf(fp, "%d", &p);
			list[i].push_back({});
			for(k=0; k<p; k++) {
				fscanf(fp, "%d", &in);
				list[i][j].push_back(in);
			}
		}
	}
	fclose(fp);
	return list;
}

void write_generic_2d(vector<vector<int>> a, const char* path) {
	int i, j;
	FILE* fp = fopen(path, "w");
	fprintf(fp, "%d\n", a.size());
	for(i=0; i<a.size(); i++) {
		fprintf(fp, "%d\n", a[i].size());
		for(j=0; j<a[i].size(); j++) {
			fprintf(fp, "%d ", a[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void write_generic_3d(vector<vector<vector<int>>> a, const char* path) {
	int i, j, k;
	FILE* fp = fopen(path, "w");
	fprintf(fp, "%d\n", a.size());
	for(i=0; i<a.size(); i++) {
		fprintf(fp, "%d\n", a[i].size());
		for(j=0; j<a[i].size(); j++) {
			fprintf(fp, "%d\n", a[i][j].size());
			for(k=0; k<a[i][j].size(); k++) {
				fprintf(fp, "%d ", a[i][j][k]);
			}
			fprintf(fp, "\n");
		}
	}
	fclose(fp);
}

void read_files(const char* folder) {
	char top_folder[20] = "CSV_files/";
	char path[100];
	strcpy(path, top_folder);
	strcat(path, folder);
	strcat(path, "/students_c.txt");
	student = read_generic_2d(path);
	strcpy(path, top_folder);
	strcat(path, folder);
	strcat(path, "/teachers_c.txt");
	teacher = read_generic_2d(path);
	strcpy(path, top_folder);
	strcat(path, folder);
	strcat(path, "/lectures_c.txt");
	lecture = read_generic_2d(path);
}

class Optimizer {
	public:
        //s: 2d list, (#student)x([class])
        //t: 3d list, (#teacher)x(#classes for teacher)x([classes])
        //l: 3d list, (#lecture)x(#classes for lecture)x([class time])
        //lim: 1d list, (#lecture)x([min, max #students in one class])
        //cr: 3d list, (#lecture)x(#classes for lecture)x([students])
        //rev: 3d list, (#period)x(#student)x([lecture, class])
        //revc: 4d list, (#day)x(#lecture)x(#class)x([indexes])
        //coll: set, {[id, period, student](id=0) or [id, day, lecture](id=1) or [id, lecture, class](id=2)}
        //push: same as coll
        //change: 2d list, (#child)x([change vector])
		vector<vector<int>> s;
		vector<vector<vector<int>>> t;
		vector<vector<vector<int>>> l;
		vector<vector<int>> lim;
		vector<vector<vector<int>>> cr;
		vector<vector<vector<vector<int>>>> rev;
		vector<vector<vector<vector<int>>>> revc;
		set<vector<int>> coll;
		set<vector<int>> push;
		vector<vector<vector<int>>> change;
		int loss;
		
		Optimizer() {
			int i, j, k;
			for(i=0; i<student.size(); i++) {
				s.push_back({});
				for(j=0; j<student[i].size(); j++) {
					int lec = student[i][j];
					s[i].push_back(random()%lecture[lec][0]);
				}
			}
			int idx[lecture.size()] = {};
			for(i=0; i<teacher.size(); i++) {
				t.push_back({});
				for(j=0; 2*j<teacher[i].size(); j++) {
					t[i].push_back({});
					int c = teacher[i][2*j];
					int n = teacher[i][2*j+1];
					for(k=0; k<n; k++) {
						t[i][j].push_back(idx[c]+k);
					}
					idx[c] += n;
				}
			}
			for(i=0; i<lecture.size(); i++) {
				l.push_back({});
				for(j=0; j<lecture[i][0]; j++) {
					l[i].push_back({});
					if(lecture[i][2]) {
						l[i][j].push_back(flat[random()%flat.size()]);
					}
					for(k=0; k<lecture[i][1]-2*lecture[i][2]; k++) {
						l[i][j].push_back(random()%period);
					}
				}
			}
			recalculate();
		}
		
		Optimizer(const char* folder) {
			char path[100] = "save/";
			strcat(path, folder);
			char file[100];
			strcpy(file, path);
			strcat(file, "/s_c.txt");
			s = read_generic_2d(file);
			strcpy(file, path);
			strcat(file, "/t_c.txt");
			t = read_generic_3d(file);
			strcpy(file, path);
			strcat(file, "/l_c.txt");
			l = read_generic_3d(file);
			recalculate();
		}
		
		void insert_coll(vector<int> v) {
			coll.insert(v);
			push.insert(v);
		}
		
		void erase_coll(vector<int> v) {
			coll.erase(v);
			push.erase(v);
		}
		
		void insert_rev(int p, int st, int lec, int cl) {
			int prev = rev[p][st].size();
			loss += prev;
			if(prev == 1) {
				insert_coll({0, p, st});
			}
			rev[p][st].push_back({lec, cl});
		}
		
		void delete_rev(int p, int st, int lec, int cl) {
			int prev = rev[p][st].size();
			loss -= prev-1;
			if(prev == 2) {
				erase_coll({0, p, st});
			}
			rev[p][st].erase(find(rev[p][st].begin(), rev[p][st].end(), vector<int>{lec, cl}));
		}
		
		void insert_revc(int day, int lec, int cl, int idx) {
			int prev = revc[day][lec][cl].size();
			loss += prev;
			if(prev == 1) {
				insert_coll({1, day, lec, cl});
			}
			revc[day][lec][cl].push_back(idx);
		}
		
		void delete_revc(int day, int lec, int cl, int idx) {
			int prev = revc[day][lec][cl].size();
			loss -= prev-1;
			if(prev == 2) {
				erase_coll({1, day, lec, cl});
			}
			revc[day][lec][cl].erase(find(revc[day][lec][cl].begin(), revc[day][lec][cl].end(), idx));
		}
		
		void insert_cr(int lec, int cl, int st) {
			if(cr[lec][cl].size() == lim[lec][0]-1) {
				erase_coll({2, lec, cl});
			} else if(cr[lec][cl].size() == lim[lec][1]) {
				insert_coll({2, lec, cl});
			}
			loss -= class_loss(lec, cl);
			cr[lec][cl].push_back(st);
			loss += class_loss(lec, cl);
		}
		
		void delete_cr(int lec, int cl, int st) {
			if(cr[lec][cl].size() == lim[lec][0]) {
				insert_coll({2, lec, cl});
			} else if(cr[lec][cl].size() == lim[lec][1]+1) {
				erase_coll({2, lec, cl});
			}
			loss -= class_loss(lec, cl);
			cr[lec][cl].erase(find(cr[lec][cl].begin(), cr[lec][cl].end(), st));
			loss += class_loss(lec, cl);
		}
		
		void change_s(int st, int idx, int val) {
			int i, j;
			int lec = student[st][idx];
			int cl = s[st][idx];
			delete_cr(lec, cl, st);
			insert_cr(lec, val, st);
			for(i=0; i<l[lec][cl].size(); i++) {
				int p = l[lec][cl][i];
				if(lecture[lec][2] && i == 0) {
					delete_rev(p+1, st, lec, cl);
				}
				delete_rev(p, st, lec, cl);
			}
			for(i=0; i<l[lec][val].size(); i++) {
				int p = l[lec][val][i];
				if(lecture[lec][2] && i == 0) {
					insert_rev(p+1, st, lec, val);
				}
				insert_rev(p, st, lec, val);
			}
			s[st][idx] = val;
		}
		
		void change_l(int lec, int cl, int idx, int val) {
			int i, j;
			int p = l[lec][cl][idx];
			for(int st:cr[lec][cl]) {
				if(lecture[lec][2] && idx == 0) {
					delete_rev(p+1, st, lec, cl);
					insert_rev(val+1, st, lec, cl);
				}
				delete_rev(p, st, lec, cl);
				insert_rev(val, st, lec, cl);
			}
			delete_revc(pd[p], lec, cl, idx);
			insert_revc(pd[val], lec, cl, idx);
			l[lec][cl][idx] = val;
		}
		
		void update_s(int st, int idx, int val, int child) {
			if(change.size() < child+1) {
				change.resize(child+1);
			}
			change[child].push_back({0, st, idx, s[st][idx], val});
			change_s(st, idx, val);
		}
		
		void update_l(int lec, int cl, int idx, int val, int child) {
			if(change.size() < child+1) {
				change.resize(child+1);
			}
			change[child].push_back({1, lec, cl, idx, l[lec][cl][idx], val});
			change_l(lec, cl, idx, val);
		}
		
		void revert_change(vector<int>& ch) {
			if(ch[0] == 0) {
				change_s(ch[1], ch[2], ch[3]);
			} else{
				change_l(ch[1], ch[2], ch[3], ch[4]);
			}
		}
		
		void revert_branch(int child) {
			int i;
			for(i=(int)change[child].size()-1; i>=0; i--) {
				revert_change(change[child][i]);
			}
		}
		
		void apply_change(vector<int>& ch) {
			if(ch[0] == 0) {
				change_s(ch[1], ch[2], ch[4]);
			} else{
				change_l(ch[1], ch[2], ch[3], ch[5]);
			}
		}
		
		void apply_branch(int child) {
			int i;
			for(i=0; i<change[child].size(); i++) {
				apply_change(change[child][i]);
			}
		}
		
		void ranked_update(int child) {
			int i;
			int r;
			int val;
			set<vector<int>>::iterator it;
			if(push.empty()) {
				it = coll.begin();
				r = random()%coll.size();
			} else {
				it = push.begin();
				r = random()%push.size();
			}
			for(i=0; i<r; i++) {
				++it;
			}
			vector<int> v = *it;
			if(v[0] == 0) {
				int p = v[1];
				int st = v[2];
				v = rev[p][st][random()%rev[p][st].size()];
				int lec = v[0];
				int c = v[1];
				if(random()%2 == 0 && lecture[lec][0] != 1) {
					for(i=0; i<student[st].size(); i++) {
						if(student[st][i] == lec) {
							break;
						}
					}
					val = random()%(lecture[lec][0]-1);
					if(val >= s[st][i]) {
						val++;
					}
					update_s(st, i, val, child);
				} else {
					for(i=0; i<l[lec][c].size(); i++) {
						if(l[lec][c][i] == p) {
							break;
						}
						if(lecture[lec][2] && i == 0 && l[lec][c][i]+1 == p) {
							break;
						}
					}
					if(lecture[lec][2] && i == 0) {
						val = random()%(flat.size()-1);
						if(flat[val] >= l[lec][c][0]) {
							val++;
						}
						val = flat[val];
					} else {
						val = random()%(period-1);
						if(val >= l[lec][c][i]) {
							val++;
						}
					}
					update_l(lec, c, i, val, child);
				}
			} else if(v[0] == 1) {
				int day = v[1];
				int lec = v[2];
				int cl = v[3];
				int idx = revc[day][lec][cl][random()%revc[day][lec][cl].size()];
				if(lecture[lec][2] && idx == 0) {
					val = random()%(flat.size()-1);
					if(flat[val] >= l[lec][cl][0]) {
						val++;
					}
					val = flat[val];
				} else {
					val = random()%(period-1);
					if(val >= l[lec][cl][i]) {
						val++;
					}
				}
				update_l(lec, cl, idx, val, child);
			} else if(v[0] == 2) {
				int lec = v[1];
				int cl = v[2];
				int count = cr[lec][cl].size();
				if(count < lim[lec][0]) {
					int idx = random()%(lecture[lec][0]-1);
					if(idx >= cl) {
						idx++;
					}
					int st = cr[lec][idx][random()%cr[lec][idx].size()];
					for(i=0; i<student[st].size(); i++) {
						if(student[st][i] == lec) {
							break;
						}
					}
					update_s(st, i, cl, child);
				} else if(count > lim[lec][1]) {
					int idx = random()%(lecture[lec][0]-1);
					if(idx >= cl) {
						idx++;
					}
					int st = cr[lec][cl][random()%cr[lec][cl].size()];
					for(i=0; i<student[st].size(); i++) {
						if(student[st][i] == lec) {
							break;
						}
					}
					update_s(st, i, idx, child);
				} else {
					printf("error");
				}
			}
		}
		
		void random_update(int child) {
			if(random()%2 == 0) {
				int st = random()%s.size();
				int idx = random()%s[st].size();
				int lec = student[st][idx];
				int val = random()%lecture[lec][0];
				update_s(st, idx, val, child);
			} else {
				int lec = random()%l.size();
				int cl = random()%l[lec].size();
				int idx = random()%l[lec][cl].size();
				int val = random()%period;
				update_l(lec, cl, idx, val, child);
			}
		}
		
		bool no_change(vector<vector<int>> branch, int depth) {
			int i, j, k;
			int info_len[2] = {3, 4};
			sort(branch.begin(), branch.begin()+depth);
			bool status = true;
			for(i=0; i<depth; i++) {
				for(j=i+1; j<depth; j++) {
					for(k=0; k<info_len[branch[i][0]]; k++) {
						if(branch[i][k] != branch[j][k]) {
							goto OUT2;
						}
					}
				}
OUT2:;
				j--;
				multiset<int> con;
				for(k=i; k<=j; k++) {
					con.insert(branch[k][info_len[branch[k][0]]]);
				}
				for(k=i; k<=j; k++) {
					auto it = con.find(branch[k][info_len[branch[k][0]]+1]);
					if(it != con.end()) {
						con.erase(it);
					}
				}
				if(!con.empty()) {
					status = false;
					break;
				}
				i = j;
			}
			return status;
		}
		
		void iterate(int childs, int depth) {
			int i, j, k, l;
			change.resize(childs);
			int idx = -1;
			int d = -1;
			int min_loss;
			int ini_loss = loss;
			for(i=0; i<childs; i++) {
				push.clear();
				for(j=0; j<depth; j++) {
					ranked_update(i);loggin
					if(!no_change(change[i], j+1) && (idx == -1 || loss < min_loss || (loss == min_loss && j > d))) {
						min_loss = loss;
						idx = i;
						d = j+1;
						if(min_loss == 0) {
							goto OUT;
						}
						if(min_loss < ini_loss) {
							goto OUT;
						}
					}
				}
				revert_branch(i);
			}
			for(i=0; i<d; i++) {
				apply_change(change[idx][i]);
			}
OUT:;
			change.clear();
		}
		
		void recalculate() {
			int i, j, k;
			lim.clear();
			int lec_count[lecture.size()] = {};
			for(i=0; i<student.size(); i++) {
				for(j=0; j<student[i].size(); j++) {
					lec_count[student[i][j]]++;
				}
			}
			for(i=0; i<lecture.size(); i++) {
				int mid = lec_count[i]/lecture[i][0];
				int rad = max_diff/2;
				if(max_diff%2 == 0) {
					lim.push_back({mid-rad, mid+rad});
				} else {
					lim.push_back({mid-rad, mid+rad+1});
				}
			}
			cr.clear();
			for(i=0; i<lecture.size(); i++) {
				cr.push_back({});
				for(j=0; j<lecture[i][0]; j++) {
					cr[i].push_back({});
				}
			}
			for(i=0; i<s.size(); i++) {
				for(j=0; j<s[i].size(); j++) {
					int c = student[i][j];
					int lec = s[i][j];
					cr[c][lec].push_back(i);
				}
			}
			rev.clear();
			for(i=0; i<period; i++) {
				rev.push_back({});
				for(j=0; j<student.size(); j++) {
					rev[i].push_back({});
				}
			}
			for(i=0; i<s.size(); i++) {
				for(j=0; j<s[i].size(); j++) {
					int lec = student[i][j];
					int c = s[i][j];
					for(k=0; k<l[lec][c].size(); k++) {
						int p = l[lec][c][k];
						rev[p][i].push_back({lec, c});
						if(lecture[lec][2] && k == 0) {
							rev[p+1][i].push_back({lec, c});
						}
					}
				}
			}
			revc.clear();
			for(i=0; i<days.size(); i++) {
				revc.push_back({});
				for(j=0; j<lecture.size(); j++) {
					revc[i].push_back({});
					for(k=0; k<lecture[j][0]; k++) {
						revc[i][j].push_back({});
					}
				}
			}
			for(i=0; i<l.size(); i++) {
				for(j=0; j<l[i].size(); j++) {
					for(k=0; k<l[i][j].size(); k++) {
						revc[pd[l[i][j][k]]][i][j].push_back(k);
					}
				}
			}
			coll.clear();
			for(i=0; i<student.size(); i++) {
				for(j=0; j<student[i].size(); j++) {
					int lec = student[i][j];
				}
			}
			loss = 0;
			for(i=0; i<period; i++) {
				for(j=0; j<student.size(); j++) {
					int n = rev[i][j].size();
					if(n >= 2) {
						insert_coll({0, i, j});
					}
					loss += (n-1)*n/2;
				}
			}
			for(i=0; i<days.size(); i++) {
				for(j=0; j<lecture.size(); j++) {
					for(k=0; k<lecture[j][0]; k++) {
						int n = revc[i][j][k].size();
						if(n >= 2) {
							insert_coll({1, i, j, k});
						}
						loss += (n-1)*n/2;
					}
				}
			}
			for(i=0; i<lecture.size(); i++) {
				for(j=0; j<lecture[i][0]; j++) {
					int add = class_loss(i, j);
					loss += add;
					if(add > 0) {
						insert_coll({2, i, j});
					}
				}
			}
		}
		
		int class_loss(int lec, int cl) {
			int num = cr[lec][cl].size();
			return max(0, max(lim[lec][0]-num, num-lim[lec][1]));
		}
		
		bool found() {
			return coll.empty();
		}
		
		void save(const char* folder) {
			char path[100] = "save/";
			strcat(path, folder);
			mkdir(path);
			char file[100];
			strcpy(file, path);
			strcat(file, "/s_c.txt");
			write_generic_2d(s, file);
			strcpy(file, path);
			strcat(file, "/t_c.txt");
			write_generic_3d(t, file);
			strcpy(file, path);
			strcat(file, "/l_c.txt");
			write_generic_3d(l, file);
		}
		
		void print(bool detail=false) {
			int i, j, k, m;
			printf("%d students\n", s.size());
			if(detail) {
				for(i=0; i<s.size(); i++) {
					printf("%d: ", i);
					for(j=0; j<s[i].size(); j++) {
						printf("%d:%d ", student[i][j], s[i][j]);
					}
					printf("\n");
				}
				printf("\n");
			}
			printf("%d teachers\n", t.size());
			if(detail) {
				for(i=0; i<t.size(); i++) {
					printf("%d: ", i);
					for(j=0; j<t[i].size(); j++) {
						printf("%d:", teacher[i][2*j]);
						for(k=0; k<t[i][j].size(); k++) {
							if(k > 0) {
								printf("/");
							}
							printf("%d", t[i][j][k]);
						}
						printf(" ");
					}
					printf("\n");
				}
				printf("\n");
			}
			printf("%d lectures\n", l.size());
			if(detail) {
				for(i=0; i<l.size(); i++) {
					printf(lecture[i][2]?"O ":"X ");
					printf("%d: ", i);
					for(j=0; j<l[i].size(); j++) {
						printf("%d:", j);
						for(k=0; k<l[i][j].size(); k++) {
							if(k > 0) {
								printf("/");
							}
							printf("%d", l[i][j][k]);
						}
						printf(" ");
					}
					printf("[%d,%d]", lim[i][0], lim[i][1]);
					printf("\n");
				}
				printf("\n");
			}
			printf("%d periods\n", period);
			if(detail) {
				printf("Class to Students\n");
				for(i=0; i<cr.size(); i++) {
					printf("%d\n", i);
					for(j=0; j<cr[i].size(); j++) {
						printf("%d:", j);
						for(auto pt=cr[i][j].begin(); pt!=cr[i][j].end(); pt++) {
							if(pt != cr[i][j].begin()) {
								printf("/");
							}
							printf("%d", *pt);
						}
						printf("\n");
					}
					printf("\n");
				}
				printf("\n");
				printf("Period-Student to Classes\n");
				for(i=0; i<rev.size(); i++) {
					printf("%d: ", i);
					for(j=0; j<rev[i].size(); j++) {
						if(!rev[i][j].empty()) {
							printf("%d: ", j);
							for(auto pair:rev[i][j]) {
								printf("%d,%d ", pair[0], pair[1]);
							}
							printf(" ");
						}
					}
					printf("\n");
				}
				printf("\n");
				printf("Day-Lecture to Indexes\n");
				for(i=0; i<revc.size(); i++) {
					printf("%d: ", i);
					for(j=0; j<revc[i].size(); j++) {
						printf("%d: ", j);
						for(k=0; k<revc[i][j].size(); k++) {
							if(!revc[i][j][k].empty()) {
								printf("%d:", k);
								for(int idx:revc[i][j][k]) {
									printf("%d,", idx);
								}
								printf(" ");
							}
						}
						printf(" ");
					}
					printf("\n");
				}
				printf("\n");
				printf("Collisions\n");
				for(auto v:coll) {
					for(i=0; i<v.size(); i++) {
						printf("%d", v[i]);
						if(i == 0) {
							printf(":");
						} else if (i+1 != v.size()) {
							printf("/");
						}
					}
					printf(" ");
				}
				printf("\n");
			}
			printf("Loss: %d\n", loss);
		}
};

int graph_size = 90;
void print_bar(int i, int loss) {
	int j;
	printf("i: %06d ", i);
	int bar = loss;
	if(bar > graph_size) {
		bar = graph_size;
	}
	for(j=0; j<bar; j++) {
		printf("*");
	}
	printf(" ");
	printf("Loss: %d\n", loss);
}

void record(const char* folder, int breadth, int depth, int times) {
	int cl;
	int i;
	read_files(folder);
	FILE* fp = fopen("log.txt", "a");
	fprintf(fp, "%s\n", folder);
	fprintf(fp, "Repeated %d times with Breadth %d Depth %d\n", times, breadth, depth);
	for(cl=0; cl<times; cl++) {
		clock_t start = clock();
		Optimizer op;
		for(i=0; ; i++) {
			if(i%1000 == 0) {
				print_bar(i, op.loss);
			}
			if(cl != 0) {
				printf("!");
			}
			op.iterate(breadth, depth);
			if(cl != 0) {
				printf("!");
			}
			if(op.found()) {
				printf("Solved at %d iterations\n", i);
				clock_t end = clock();
				double duration = (double)(end-start)/CLOCKS_PER_SEC;
				fprintf(fp, "Solved at %d iterations %f seconds\n", i, duration);
				op.save("solution");
				break;
			}
		}
	}
	fclose(fp);
}

int main() {
	int i, j;
	int n, m;
	cal_periods();
	record("2017-1", 12, 5, 1);
}
