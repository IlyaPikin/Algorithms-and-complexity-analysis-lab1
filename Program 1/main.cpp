#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <random>
#include <locale>
#include <omp.h>

#define uint unsigned

using namespace std;

const uint BIG_NUM = 1'000'000'000;
bool MARKS[1'000'000];
uint DIST1[1'000'000];
uint DIST2[1'000'000];
uint UP1[1'000'000];
uint UP2[1'000'000];

// Класс, необходимый для вывода вещественных чисел с символом запятой
class comma : public numpunct<char> {
public:
	comma() : numpunct<char>() {}
protected:
	char do_decimal_point() const {
		return ',';
	}
};

class vtype
{
public:
	uint name;
	uint w;
	vtype* next;

	vtype() { name = 0; w = 0; next = nullptr; }
	vtype(uint name_, uint w_, vtype* next_) : name(name_), w(w_), next(next_) {}
	vtype& operator=(const vtype& other)
	{
		if (this != &other)
		{
			name = other.name;
			w = other.w;
			next = nullptr;
		}
		return *this;
	}
	~vtype() {}
};

class Graph
{
public:
//private

	uint n = 0;						// число вершин в графе
	uint m = 0;						// число рёбер в графе
	vtype** ADJ = nullptr;			// списки смежности графа

public:
	Graph() : n(0), m(0) { ADJ = nullptr; }
	Graph(string fileName)
	{
		generate_from_file(fileName);
	}
	Graph(uint n_, uint q_, uint r_)
	{
		generate_full_graph(n_, q_, r_);
	}
	Graph(uint n_, uint m_, uint q_, uint r_)
	{
		generate_rand_graph(n_, m_, q_, r_);
	}
	Graph(const Graph& other)
	{
		n = other.n;
		m = other.m;

		ADJ = new vtype * [n];

		for (uint i = 0; i < n; i++)
		{
			vtype* other_el = other.ADJ[i];
			ADJ[i] = nullptr;
			vtype** prev_ptr = &ADJ[i];

			while (other_el != nullptr)
			{
				vtype* new_edge = new vtype(other_el->name, other_el->w, nullptr);
				*prev_ptr = new_edge;
				prev_ptr = &(new_edge->next);
				other_el = other_el->next;
			}
		}
	}

	~Graph()
	{
		clear();
	}

	void generate_from_file(string fileName)
	{
		clear();
		// чтение данных из файла
		ifstream file(fileName);
		file >> n >> m;
		string str;
		getline(file, str);

		ADJ = new vtype * [n];

		// заполнение списка смежностей
		for (uint i = 0; i < n; i++)
		{
			ADJ[i] = nullptr;
			vtype** prev_ptr = &ADJ[i];

			string new_str;
			getline(file, new_str);

			size_t pos = 0;
			// парсинг строки
			while ((pos = new_str.find('(')) != std::string::npos)
			{
				new_str.erase(0, pos + 1);
				uint name = stoi(new_str, &pos); // получение номера смежной вершины

				new_str.erase(0, pos + 1); // удаление запятой

				uint w = stoi(new_str, &pos); // получение веса ребра

				vtype* new_edge = new vtype(name, w, nullptr);

				*prev_ptr = new_edge; // добавляем адрес нового элемента в старый

				prev_ptr = &(new_edge->next); // обновляем адрес указателя на следующий элемент

			}

		}
	}
	void generate_rand_graph(uint n_, uint m_, uint q, uint r)
	{
		if (m_ >= n_ * (n_ - 1))
		{
			cout << "Too many edges!";
			generate_full_graph(n_, q, r);
			return;
		}

		clear();
		n = n_;
		m = m_;

		ADJ = new vtype * [n];
		for (uint i = 0; i < n; i++)
			ADJ[i] = nullptr;

		// Проверяем корректность аргументов
		if (q > r) swap(q, r);

		// Генератор псевдослучайных чисел
		mt19937 engine;
		// Создаём псевдо-устройство для получения случайного зерна.
		std::random_device device;
		// Получаем случайное зерно последовательности
		engine.seed(device());
		// Создаём равномерное распределение в диапозоне [q,r] для генерации веса ребра
		uniform_int_distribution<uint> weight_distribution(q, r);
		// Создаём равномерное распределение в диапозоне [0,n-1] для генерации индекса вершины
		uniform_int_distribution<uint> index_distribution(0, n - 1);

		uint count = 0;
		while (count != m)
		{
			// Вычисляем псевдослучайное число: вызовем распределение как функцию,
			// передав генератор произвольных целых чисел как аргумент.
			uint weight = weight_distribution(engine);
			uint index = index_distribution(engine);
			uint name = 0;
			do
			{
				name = index_distribution(engine);
			} while (name == index);

			vtype** prev_ptr = &ADJ[index];

			// Вставка в список смежности с проверкой кратности ребра 
			while (true)
			{
				vtype* curr_el = *prev_ptr;

				if (curr_el == nullptr)
				{
					vtype* new_edge = new vtype(name + 1, weight, nullptr);
					*prev_ptr = new_edge; // добавляем адрес нового элемента в старый
					count++;
					break;
				}
				else
				{
					if (curr_el->name == name + 1)
					{
						break;
					}
					else if (curr_el->name < name + 1)
					{
						prev_ptr = &(curr_el->next); // обновляем адрес указателя на следующий элемент

					}
					else // curr_el->name > name + 1
					{
						vtype* new_edge = new vtype(name + 1, weight, curr_el);
						*prev_ptr = new_edge; // добавляем адрес нового элемента в предыдущий
						count++;
						break;
					}
				}
			}
		}
	}
	void generate_full_graph(uint n_, uint q, uint r)
	{
		clear();
		n = n_;
		m = n * (n - 1);

		ADJ = new vtype * [n];
		for (uint i = 0; i < n; i++)
			ADJ[i] = nullptr;

		// Проверяем корректность аргументов
		if (q > r) swap(q, r);

		// Генератор псевдослучайных чисел
		mt19937 engine;
		// Создаём псевдо-устройство для получения случайного зерна.
		std::random_device device;
		// Получаем случайное зерно последовательности
		engine.seed(device());
		// Создаём равномерное распределение в диапозоне [q,r] для генерации веса ребра
		uniform_int_distribution<uint> weight_distribution(q, r);


		for (uint index = 0; index < n; index++)
		{
			vtype** prev_ptr = &ADJ[index];
			// Вставка в список смежности
			for (uint j = 0; j < n; j++)
			{
				// Вычисляем псевдослучайное число: вызовем распределение как функцию,
				// передав генератор произвольных целых чисел как аргумент.
				uint weight = weight_distribution(engine);

				if (j != index)
				{
					vtype* new_edge = new vtype(j + 1, weight, nullptr);
					*prev_ptr = new_edge; // добавляем адрес нового элемента в старый
					prev_ptr = &(new_edge->next); // обновляем адрес указателя на следующий элемент
				}
			}
		}
	}

	void clear()
	{
		if (ADJ != nullptr)
		{
			
			for (uint i = 0; i < n; i++)
			{
				 vtype* current = ADJ[i];
				 vtype* next = nullptr;
				 while (current != nullptr)
				 {
					 next = current->next;
					 delete current;
					 current = next;
				 }
			}
			delete[] ADJ;
			ADJ = nullptr;
		}
		n = 0;
		m = 0;
	}
	void print_to_console()
	{
		cout << "n = " << n << ", m = " << m << endl;
		for (uint i = 0; i < n; i++)
		{
			cout << i + 1 << ": ";
			vtype* curr_el = ADJ[i];
			while (curr_el)
			{
				cout << "(" << curr_el->name << "," << curr_el->w << ") ";
				curr_el = curr_el->next;
			}
			cout << endl;
		}
		cout << endl;
	}
	void print_to_file(string fileName)
	{
		ofstream file(fileName);
		file << n << " " << m << endl;
		for (uint i = 0; i < n; i++)
		{
			file << i + 1 << ": ";
			vtype* curr_el = ADJ[i];
			while (curr_el)
			{
				file << "(" << curr_el->name << "," << curr_el->w << ") ";
				curr_el = curr_el->next;
			}
			file << endl;
		}
		file << endl;
	}
	
};

// Сложность O(n^2)
void ldg_dijkstra_mark(uint* dist, uint* up, const Graph& graph, uint start)
{
	// Вывыд названия запущенного алогоритма
	cout << "The Dijkstra's algorithm with marks has been started!" << endl;
	start--;
	bool* h = &MARKS[0]; // Массив меток
	
	// Инициализация массивов
	for (uint i = 0; i < graph.n; i++)
	{
		dist[i] = BIG_NUM;
		up[i] = i;			// Родителями вершин, не имеющих путь в start, будут сами эти вершины 
		h[i] = false;
	}

	dist[start] = 0;
	uint nq = graph.n; // Количество вершин, для которых не найден кротчайший путь до start

	while (nq > 0)
	{
		// Вывод состояния алгоритма
		cout << graph.n - nq + 1 << "/" << graph.n << " iteration of cycle of Dijkstra's algorithm." << endl;
		// Поиск первой попавшейся вершины в массиве h, для которой не найден кр. путь до start
		uint c = 0;
		while (h[c] != false)
			c++;
		
		// Поиск вершины с минимальной текущей длиной пути до start
		uint i = c;
		for (uint k = c + 1; k < graph.n; k++)
		{
			if (h[k] == false)
			{
				if (dist[i] > dist[k])
				{
					i = k;
				}
			}
		}

		// Текущая длина пути от вершины i до start минимальна
		h[i] = true;
		nq--;

		// Просматриваем окрестность вершины i
		vtype* curr_el = graph.ADJ[i];
		while (curr_el != nullptr)
		{
			uint j = curr_el->name - 1;
			if (h[j] == false)
			{
				if (dist[j] > dist[i] + curr_el->w) // Релаксация ребра
				{
					dist[j] = dist[i] + curr_el->w;
					up[j] = i;
				}
			}
			curr_el = curr_el->next;
		}
	}
	cout << "The end of the Dijkstra's algorithm with marks." << endl;
}

// Сложность O(n*m)
void ldg_ford_bellman(uint* dist, uint* up, const Graph& graph, uint start)
{
	// Вывыд названия запущенного алогоритма
	cout << "The Ford-Bellman's algorithm has been started!" << endl;
	start--;
	// Инициализация массивов
	for (uint i = 0; i < graph.n; i++)
	{
		dist[i] = BIG_NUM;
		up[i] = i;
	}

	dist[start] = 0;
	
	for (uint k = 0; k < graph.n - 1; k++)
	{
		// Вывод состояния алгоритма
		cout << k << "/" << graph.n-1 << " iteration of cycle of Ford-Bellman's algorithm." << endl;
		// Просматриваем все ребра каждой вершины
		for (uint i = 0; i < graph.n; i++)
		{
			vtype* curr_el = graph.ADJ[i];
			while (curr_el != nullptr)
			{
				uint j = curr_el->name - 1;
				if (j != start)
				{
					if (dist[j] > dist[i] + curr_el->w) // Релаксация ребра
					{
						dist[j] = dist[i] + curr_el->w;
						up[j] = i;
					}
				}
				curr_el = curr_el->next;
			}
		}
	}
	cout << "The end of the Ford-Bellman's algorithm." << endl;
}

bool compare(uint n, uint* dist1, uint* dist2, uint* up1, uint* up2)
{
	for (uint i = 0; i < n; i++)
	{
		if ((dist1[i] != dist2[i]))
		{
			cout << "Distances are not equel!" << endl;
			cout << i + 1 << ": " << dist1[i] << " " << dist2[i] << " parent = " << up1[i] + 1 << " " << up2[i] + 1 << endl;
			return false;
		}
	}
	cout << "Correct!" << endl;
	return true;
}

void ex1()
{
	// ---------------- 1st experiment ----------------

	ofstream ex1("experiments\\ex1\\ex1.csv");
	ex1 << fixed;

	for (uint i = 1; i <= 1501; i += 100)
	{
		uint n = i;
		uint m = n * (n - 1);
		uint q = 1;
		uint r = 1000;

		ex1 << n << ";" << m << ";" << q << ";" << r;

		Graph graph(n, q, r); // generate full graph

		// Dijkstra's algorithm with marks
		uint* dist1 = &DIST1[0];
		uint* up1 = &UP1[0];

		auto start_d = std::chrono::steady_clock::now();
		ldg_dijkstra_mark(dist1, up1, graph, 1);
		auto end_d = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_d = end_d - start_d;

		ex1 << ";" << elapsed_seconds_d.count();
		//ex1 << ";" << elapsed_seconds_d.count() << endl;


		// Ford-Bellman's algorithm
		uint* dist2 = &DIST2[0];
		uint* up2 = &UP2[0];

		auto start_fb = std::chrono::steady_clock::now();
		ldg_ford_bellman(dist2, up2, graph, 1);
		auto end_fb = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_fb = end_fb - start_fb;

		ex1 << ";" << elapsed_seconds_fb.count() << endl;

		//if (!compare(n, dist1, dist2, up1, up2))
		//{
		//	system("pause");
		//}
	}

	ex1.close();
}
void ex2()
{
	// ---------------- 2nd experiment ----------------

	ofstream ex2("experiments\\ex2\\ex2.csv");
	ex2 << fixed;

	for (uint i = 101; i <= 2501; i += 100)
	{
		uint n = i;
		uint m = n * 100;
		uint q = 1;
		uint r = 1000;

		ex2 << n << ";" << m << ";" << q << ";" << r;

		Graph graph(n, m, q, r);

		// Dijkstra's algorithm with marks
		uint* dist1 = &DIST1[0];
		uint* up1 = &UP1[0];


		auto start_d = std::chrono::steady_clock::now();
		ldg_dijkstra_mark(dist1, up1, graph, 1);
		auto end_d = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_d = end_d - start_d;

		ex2 << ";" << elapsed_seconds_d.count();
		//ex2 << ";" << elapsed_seconds_d.count() << endl;


		// Ford-Bellman's algorithm
		uint* dist2 = &DIST2[0];
		uint* up2 = &UP2[0];

		auto start_fb = std::chrono::steady_clock::now();
		ldg_ford_bellman(dist2, up2, graph, 1);
		auto end_fb = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_fb = end_fb - start_fb;

		ex2 << ";" << elapsed_seconds_fb.count() << endl;

		//if (!compare(n, dist1, dist2, up1, up2))
		//{
		//	system("pause");
		//}
	}

	ex2.close();
}
void ex3()
{
	// ---------------- 3rd experiment ----------------

	ofstream ex3("experiments\\ex3\\ex3.csv");
	ex3 << fixed;

	for (uint i = 0; i <= 500'000; i += 10'000)
	{
		uint n = 1001;
		uint m = i;
		uint q = 1;
		uint r = 1000;

		ex3 << n << ";" << m << ";" << q << ";" << r;

		Graph graph(n, m, q, r); 

		// Dijkstra's algorithm with marks
		uint* dist1 = &DIST1[0];
		uint* up1 = &UP1[0];

		auto start_d = std::chrono::steady_clock::now();
		ldg_dijkstra_mark(dist1, up1, graph, 1);
		auto end_d = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_d = end_d - start_d;

		ex3 << ";" << elapsed_seconds_d.count();

		// Ford-Bellman's algorithm
		uint* dist2 = &DIST2[0];
		uint* up2 = &UP2[0];

		auto start_fb = std::chrono::steady_clock::now();
		ldg_ford_bellman(dist2, up2, graph, 1);
		auto end_fb = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_fb = end_fb - start_fb;

		ex3 << ";" << elapsed_seconds_fb.count() << endl;

		//if (!compare(n, dist1, dist2, up1, up2))
		//{
		//	system("pause");
		//}
	}

	ex3.close();
}
void ex4()
{
	// ---------------- 4th experiment ----------------

	ofstream ex4("experiments\\ex4\\ex4.csv");
	ex4 << fixed;

	for (uint i = 1; i <= 136; i += 1)
	{
		uint n = 1001;
		uint m = n*(n-1);
		uint q = 1;
		uint r = i;

		ex4 << n << ";" << m << ";" << q << ";" << r;

		Graph graph(n, m, q, r); // generate full graph

		// Dijkstra's algorithm with marks
		uint* dist1 = &DIST1[0];
		uint* up1 = &UP1[0];

		auto start_d = std::chrono::steady_clock::now();
		ldg_dijkstra_mark(dist1, up1, graph, 1);
		auto end_d = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_d = end_d - start_d;

		ex4 << ";" << elapsed_seconds_d.count();

		// Ford-Bellman's algorithm
		uint* dist2 = &DIST2[0];
		uint* up2 = &UP2[0];

		auto start_fb = std::chrono::steady_clock::now();
		ldg_ford_bellman(dist2, up2, graph, 1);
		auto end_fb = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_fb = end_fb - start_fb;

		ex4 << ";" << elapsed_seconds_fb.count() << endl;

		//if (!compare(n, dist1, dist2, up1, up2))
		//{
		//	system("pause");
		//}
	}

	ex4.close();
}

int main(int argc, char* argv[])
{
	// Настройка вывода вещественный чисел с символом запятой
	locale loccomma(cout.getloc(), new comma);
	locale::global(loccomma);

	// Серия экспериментов

	ex1();
	 
	ex2();

	ex3();

	ex4();

	return 0;
}