#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <set>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

void remove_html(wstring & s) {
	size_t left = 0, right;
	while ((left = s.find(L"<", left)) != wstring::npos) {
		if ((right = s.find(L">", left)) != wstring::npos) {
			s.erase(left, right - left + 1);
			left = right;
		}
		else { break; }
	}
}

void inverted_index(vector<fs::path> &f, int lower, int upper, map<wstring, set<wstring>>& index) {
	wstring buffer;
	wstring temp;
	for (int i = lower; i < upper; i++) {
		wifstream file(f[i]);
		if (!file.is_open()) {
			cerr << "To bad" << endl;
			continue;
		}
		file.seekg(0, ios_base::end);
		auto size = file.tellg();
		file.seekg(0, ios_base::beg);
		buffer.resize(size);
		file.read(buffer.data(), size);
		remove_html(buffer);
		auto it = remove_if(buffer.begin(), buffer.end(), [](wchar_t c) {return !iswalnum(c) && !iswspace(c); });
		buffer.erase(it, buffer.end());
		transform(buffer.begin(), buffer.end(), buffer.begin(), [](wchar_t c) -> wchar_t {return towlower(c); });
		wistringstream ws(buffer);
		while (ws >> temp) {
			index[temp].insert(f[i]);
		}
	}
}


int main() {
	map<wstring, set<wstring>> index;
	int thread_count;
	int chunk;
	vector<fs::path> files;
	vector<thread> threads;
	wstring temp;
	vector<fs::path> paths = { "D:/Coursework/Set1/", "D:/Coursework/Set2/","D:/Coursework/Set3/","D:/Coursework/Set4/","D:/Coursework/Set5/" };
	for (const auto& path : paths) {
		for (const auto& file : fs::directory_iterator(path)) {
			if (file.is_regular_file()) {
				files.push_back(file.path());
			}
		}
	}


	auto start = std::chrono::system_clock::now();
	inverted_index(ref(files), 0, files.size(), ref(index));

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> result = end - start;

	//for (auto it = index.begin(); it != index.end(); ++it) {
	//	wcout << it->first << ": (";
	//	for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
	//		wcout << *it2 << ", ";
	//	cout << ")" << endl;
	//}
	cout << "Index size: " << index.size() << endl;
	cout << "Time: " << result.count() << endl;
	system("pause");
	return 0;
}