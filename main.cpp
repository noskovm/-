#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>
#include <list>

using namespace std;

class HashTableEntry{

public:
	string key_;
	string value_;

	HashTableEntry() 
	{
	}

	HashTableEntry(string key, string value) 
	{
		this->key_ = key;
		this->value_ = value;
	}
};

class HashTable
{
private:
	vector<list<HashTableEntry>> table_;
	int table_capacity_;	//мощность хеш-таблицы
	int HashFunction(string& key);
	int Conversion(int base, string s);	//перевод из base-системы счисления в 10-ричную

public:
	HashTable(int count_of_key_value_couple)
	{
		table_capacity_ = count_of_key_value_couple << 1; //задаем мощность по формуле 2^n(n-кол-во элементов)
		table_.resize(table_capacity_);
	}

	void Put(HashTableEntry& new_entry);

	string Get(string& key); //возвращает value 
};

int HashTable::HashFunction(string& key)
{
	int key_index = Conversion(62, key);
	return key_index % table_capacity_;
}

int HashTable::Conversion(int base, string s)   //перевод из заданной в десятичную систему счисления
{
	int answer = 0;
	int power = (int)(s.length() - 1);

	for (int i = 0; i < s.length(); ++i)
	{
		if (isdigit(s[i]))
			answer += (int)((s[i] - 48) * pow(base, power));		//смещение 48, чтобы брался не аски-код, а цифра
		else
			answer += (int)((s[i] - 55) * pow(base, power));		//смещение, чтобы бралась цифра от буквы(A = 10, B = 11...)

		power--;
	}

	return answer;
}

void HashTable::Put(HashTableEntry& new_entry)
{
	int key_index = HashFunction(new_entry.key_);
	list<HashTableEntry>::iterator inner_list_iterator = table_[key_index].begin();

	while (inner_list_iterator != table_[key_index].end())
		inner_list_iterator++;  //идем по списку пока не найдем свободную ячейку

	table_[key_index].insert(inner_list_iterator, new_entry);
}

string HashTable::Get(string& key)
{
	vector<list<HashTableEntry>>::iterator table_iterator;
	list<HashTableEntry>::iterator inner_list_iterator;

	table_iterator = table_.begin();
	table_iterator += HashFunction(key);
	for (inner_list_iterator = table_iterator->begin(); inner_list_iterator != table_iterator->end(); inner_list_iterator++)
		if (inner_list_iterator->key_ == key)
			return inner_list_iterator->value_;

	string warning = "В таблице нет такой записи";
	return warning;
}

vector<HashTableEntry> GetKeyValuePair(char* argv[]) ///доделать покрасивше
{
	fstream data;
	data.open(argv[1]); //файл с исходными данными

	char symbol;
	vector<HashTableEntry> key_value_pair;
	HashTableEntry new_entry;

	while (data.get(symbol))  //считывает весь файл до конца после чтения define
	{
		if (symbol == '<')		//key начинается после '<'
		{
			data.get(symbol);	//считываем лишний '<'
			for (int i = 0; symbol != '>'; i++)
			{
				new_entry.key_ += symbol;
				data.get(symbol);				}

			data.get(symbol); //считываем лишний '>'
			data.get(symbol); //считываем лишний '<'

			for (int i = 0; symbol != '>'; i++)
			{
				new_entry.value_ += symbol;
				data.get(symbol);
			}

			key_value_pair.push_back(new_entry); //если сработало условие на value, значит buffer.key записан и пара сформирована
			new_entry.key_ = "";
			new_entry.value_ = "";

			data.get(symbol);  //считываем символ '\n'
			data.get(symbol);  //если после value не идет #, то директивы #define закончились
			if (symbol != '#')
			{
				data.close();
				return key_value_pair;
			}		
		}
	}
	return key_value_pair;
}

void Menu(char* argv[])
{
	string warning[4] = { "Директив не найдено", "Ошибка открытия файла для записи", 
					   "Ошибка открытия исходного файла", "Редактирование отменено" };

	cout << "Программа для организации макрозамен в тексте" << endl;
	cout << "Для начала редактирования файла нажите Y" << endl;
	cout << "Ваш ответ >";
	char answer;
	cin >> answer;

	if (answer == 'Y' || answer == 'y')
	{
		fstream data;
		data.open(argv[1]); //файл с исходными данными подаеттся через аргумент командной строки

		if (data.is_open())
		{
			vector<HashTableEntry> key_value_couple = GetKeyValuePair(argv);
			if (key_value_couple.size() == 0)
			{
				cout << warning[0];
			}
			else
			{
				HashTable entries((int)key_value_couple.size());;

				for (int i = 0; i < key_value_couple.size(); ++i)
				{
					entries.Put(key_value_couple[i]);
				}

				fstream out;
				out.open("changed.txt");

				if (out.is_open())
				{
					while (!data.eof())
					{
						string current_word;
						char symbol;
						data >> current_word;
						data.get(symbol); //чтобы отловить знак переноса строки

						int i = 0;
						string buffer = current_word;
						string punct_buffer = "";
						current_word = "";
						for (int i = 0; i < buffer.size(); i++)
						{
							if (isalpha(buffer[i]) || isdigit(buffer[i]))
							{
								current_word += buffer[i];
							}
							else
							{
								punct_buffer += buffer[i];
							}
						}
						
						int couple_of_define = 0;
						for (int i = 0; i < key_value_couple.size(); ++i, ++couple_of_define)
						{
							if (current_word == key_value_couple[i].key_)
							{
								current_word = entries.Get(current_word);
								out << current_word;
								out << punct_buffer;
								current_word = "";
								punct_buffer = "";
							}
						}
						out << current_word;
						out << punct_buffer;

						if (symbol == '\n')
							out << '\n';
						else
							out << " ";
					}
					out.close();
				}
				else
					cout << warning[1] << endl;
			}
			data.close();
		}
		else
			cout << warning[2] << endl;
	}
	else
		cout << warning[3] << endl;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");
	
	Menu(argv);

	return 0;
}