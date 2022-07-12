#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <time.h>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

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

//student: 2d list, (#student)x[lectures]
//teacher: 2d list, (#teacher)x[lectures, class_num]
//lecture: 1d list, (#lecture)x(class_num, class_time, consecutive)
vector<vector<int>> student;
vector<vector<int>> teacher;
vector<vector<int>> lecture;
int period = 45;

class Optimizer {
	public:
        //s: 2d list, (#student)x([class])
        //t: 3d list, (#teacher)x(#classes for teacher)x([classes])
        //l: 3d list, (#lecture)x(#classes for lecture)x([class time])
        //pr: 2d list, (#period)x([lecture, class])
        //cr: 3d list, (#lecture)x(#classes for lecture)x([students])
        //rank: 2d list, (0~max-count)x([period, student])
        //count: 2d list, (#period)x([students])
        //change: 2d list, (#child)x([change vector])
		vector<vector<int>> s;
		vector<vector<vector<int>>> t;
		vector<vector<vector<int>>> l;	
		vector<vector<vector<int>>> cr;
		vector<vector<vector<vector<int>>>> rev;
		vector<set<vector<int>>> rank;
		int maxr;
		vector<vector<vector<int>>> change;
		int loss;
		
		Optimizer() {
			int i, j, k;
			for(i=0; i<student.size(); i++) {
				s.push_back({});
				for(j=0; j<student[i].size(); j++) {
					int lec = student[i][j];
					s[i].push_back(rand()%lecture[lec][0]);
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
					for(k=0; k<lecture[i][1]; k++) {
						l[i][j].push_back(rand()%period);
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
		
		void insert_rank(int r, vector<int> v) {
			if(r >= 2) {
				rank[r].insert(v);
			}
		}
		
		void erase_rank(int r, vector<int> v) {
			if(r >= 2) {
				rank[r].erase(v);
			}
		}
		
		void insert_rev(int p, int st, int lec, int c) {
			int prev = rev[p][st].size();
			loss += prev;
			//printf("INS %d %d %d %d %d\n", p, st, lec, c, prev);
			erase_rank(prev, {p, st});
			insert_rank(prev+1, {p, st});
			if(prev+1 > maxr) {
				maxr = prev+1;
			}
			rev[p][st].push_back({lec, c});
		}
		
		void delete_rev(int p, int st, int lec, int c) {
			int prev = rev[p][st].size();
			loss -= prev-1;
			//printf("DEL %d %d %d %d %d\n", p, st, lec, c, prev-1);
			erase_rank(prev, {p, st});
			insert_rank(prev-1, {p, st});
			if(prev == maxr && rank[prev].empty()) {
				maxr = cal_maxr();
			}
			rev[p][st].erase(find(rev[p][st].begin(), rev[p][st].end(), vector<int>{lec, c}));
		}
		
		void change_s(int st, int idx, int val) {
			int i, j;
			int lec = student[st][idx];
			int c = s[st][idx];
			cr[lec][c].erase(find(cr[lec][c].begin(), cr[lec][c].end(), st));
			cr[lec][val].push_back(st);
			for(i=0; i<l[lec][c].size(); i++) {
				int p = l[lec][c][i];
				delete_rev(p, st, lec, c);
			}
			for(i=0; i<l[lec][val].size(); i++) {
				int p = l[lec][val][i];
				insert_rev(p, st, lec, val);
			}
			s[st][idx] = val;
		}
		
		void change_l(int lec, int cl, int idx, int val) {
			int i, j;
			int p = l[lec][cl][idx];
			for(int st:cr[lec][cl]) {
				delete_rev(p, st, lec, cl);
				insert_rev(val, st, lec, cl);
			}
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
			auto it = rank[maxr].begin();
			int r = rand()%rank[maxr].size();
			for(i=0; i<r; i++) {
				++it;
			}
			vector<int> v = *it;
			int p = v[0];
			int st = v[1];
			v = rev[p][st][rand()%rev[p][st].size()];
			int lec = v[0];
			int c = v[1];
			if(rand()%2 == 0 && lecture[lec][0] != 1) {
				for(i=0; i<student[st].size(); i++) {
					if(student[st][i] == lec) {
						break;
					}
				}
				int val = rand()%(lecture[lec][0]-1);
				if(val >= s[st][i]) {
					val++;
				}
				update_s(st, i, val, child);
			} else {
				for(i=0; i<l[lec][c].size(); i++) {
					if(l[lec][c][i] == p) {
						break;
					}
				}
				int val = rand()%(period-1);
				if(val >= l[lec][c][i]) {
					val++;
				}
				update_l(lec, c, i, val, child);
			}
		}
		
		void random_update(int child) {
			if(rand()%2 == 0) {
				int st = rand()%s.size();
				int idx = rand()%s[st].size();
				int lec = student[st][idx];
				int val = rand()%lecture[lec][0];
				update_s(st, idx, val, child);
			} else {
				int lec = rand()%l.size();
				int cl = rand()%l[lec].size();
				int idx = rand()%l[lec][cl].size();
				int val = rand()%period;
				update_l(lec, cl, idx, val, child);
			}
		}
		
		// sorts branch for performance, be careful
		bool no_change(vector<vector<int>>& branch, int depth) {
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
			int i, j;
			change.resize(childs);
			int idx = -1;
			int d = -1;
			int min_loss;
			for(i=0; i<childs; i++) {
				for(j=0; j<depth; j++) {
					ranked_update(i);
					if(idx == -1 || loss < min_loss || (loss == min_loss && j > d)) {
						min_loss = loss;
						idx = i;
						d = j+1;
						if(min_loss == 0) {
							goto OUT;
						}
					}
				}
				revert_branch(i);
			}
OUT:;
			for(i=0; i<d; i++) {
				apply_change(change[idx][i]);
			}
			if(rand()%10 == 0) {
				for(i=0; i<d; i++) {
					for(j=0; j<change[idx][i].size(); j++) {
						printf("%d ", change[idx][i][j]);
					}
					printf("\n");
				}
				if(no_change(change[idx], d)) {
					printf("No Change\n");
				}
				printf("\n");
			}
			change.clear();
		}
		
		int cal_maxr() {
			int i;
			for(i=rank.size()-1; i>=0 && rank[i].empty(); i--);
			return i;
		}
		
		void recalculate() {
			int i, j, k;
			cr.clear();
			for(i=0; i<lecture.size(); i++) {
				cr.push_back({});
				for(j=0; j<lecture[i][0]; j++) {
					cr[i].push_back(vector<int>{});
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
					rev[i].push_back(vector<vector<int>>{});
				}
			}
			for(i=0; i<s.size(); i++) {
				for(j=0; j<s[i].size(); j++) {
					int lec = student[i][j];
					int c = s[i][j];
					for(k=0; k<l[lec][c].size(); k++) {
						int p = l[lec][c][k];
						rev[p][i].push_back({lec, c});
					}
				}
			}
			rank.clear();
			int max_rank = 0;
			for(i=0; i<student.size(); i++) {
				int a = 0;
				for(j=0; j<student[i].size(); j++) {
					int lec = student[i][j];
					a += lecture[lec][1];
				}
				max_rank = max(max_rank, a);
			}
			rank.resize(max_rank+1);
			loss = 0;
			for(i=0; i<period; i++) {
				for(j=0; j<student.size(); j++) {
					int n = rev[i][j].size();
					insert_rank(n, {i, j});
					loss += (n-1)*n/2;
				}
			}
			maxr = cal_maxr();
		}
		
		bool found() {
			return (maxr < 2);
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
			int i, j, k;
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
					printf("\n");
				}
				printf("\n");
			}
			printf("%d periods\n", period);
			if(detail) {
				printf("Class Reverse\n");
				for(i=0; i<cr.size(); i++) {
					printf("%d: ", i);
					for(j=0; j<cr[i].size(); j++) {
						printf("%d:", j);
						for(auto pt=cr[i][j].begin(); pt!=cr[i][j].end(); pt++) {
							if(pt != cr[i][j].begin()) {
								printf("/");
							}
							printf("%d", *pt);
						}
						printf(" ");
					}
					printf("\n");
				}
				printf("\n");
				printf("Reverse\n");
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
				printf("Rank\n");
				for(i=0; i<rank.size(); i++) {
					printf("%d: ", i);
					for(auto pair:rank[i]) {
						printf("%d:%d ", pair[0], pair[1]);
					}
					printf("\n");
				}
				printf("\n");
				printf("Max Rank: %d\n", maxr);
				printf("\n");
			}
			printf("Rank Summary\n");
			for(i=2; i<=maxr; i++) {
				printf("%d: %d\n", i, rank[i].size());
			}
			printf("Loss: %d\n", loss);
		}
};

int main() {
	srand(time(NULL));
	int i, j;
	int n, m;
	lecture = read_generic_2d("CSV_files/2016-1/lectures_c.txt");
	student = read_generic_2d("CSV_files/2016-1/students_c.txt");
	teacher = read_generic_2d("CSV_files/2016-1/teachers_c.txt");
	Optimizer op;
	for(i=0; i<=100000; i++) {
		if(i%1000 == 0) {
			printf("i: %d\n", i);
			op.print();
		}
		int prev = op.loss;
		op.iterate(100, 5);
		if(op.found()) {
			printf("Solved at %d iterations", i);
			op.save("solution");
			break;
		}
	}
}
