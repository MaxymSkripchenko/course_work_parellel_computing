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
#include <mutex>

using namespace std;
namespace fs = std::filesystem;
mutex m;

void remove_html(wstring & s) {
	size_t left = 0, right; // posiitions of "<"(left) and ">"(right)
	while ((left = s.find(L"<", left)) != wstring::npos) { //seeking "<" until eof
		if ((right = s.find(L">", left)) != wstring::npos) { //sekking ">" until eof
			s.erase(left, right - left + 1); //erasing tag
			left = right;
		}
		else { break; }
	}
}

void inverted_index(vector<fs::path> &f, int lower, int upper, map<wstring, set<wstring>>& index) {
	wstring buffer; // buffer to store string from file
	wstring temp; // buffer to store every word of file
	for (int i = lower; i < upper; i++) {
		wifstream file(f[i]); // opening file with wifstream allows us to read wide char, like Unicode
		//if a file doesn't open prinitng error message and continuing loop
		if (!file.is_open()) {
			cerr << "To bad" << endl;
			continue;
		}
		//to read whole file into string we place carriage in file to it's end with seekg
		//them with tellg we getting position of end of file and storing it in size
		//them returning carriage to the start of the file
		file.seekg(0, ios_base::end);
		auto size = file.tellg();
		file.seekg(0, ios_base::beg);
		//resizing our buffer string so it can store whole string from a file
		buffer.resize(size);
		//reading whole string from a file to buffer
		file.read(buffer.data(), size);
		//calling return_html function to delete all html tags from file
		remove_html(buffer);
		//removing everything that is not number, letter of space-like symbols
		auto it = remove_if(buffer.begin(), buffer.end(), [](wchar_t c) {return !iswalnum(c) && !iswspace(c); });
		buffer.erase(it, buffer.end());
		//lowering all letters in buffer
		transform(buffer.begin(), buffer.end(), buffer.begin(), [](wchar_t c) -> wchar_t {return towlower(c); });
		//creating streamstring ws, that stores our buffer so we can read every word from our buffer separately
		wistringstream ws(buffer);
		while (ws >> temp) {
			//locking mutex to prevent simultaneous access to index
			lock_guard<mutex> lock(m);
			//pushing path of file into index if key temp already in it
			//if not creating new set with key temp and pushing path of file in it
			index[temp].insert(f[i]);
		}
	}
}


int main() {
	map<wstring, set<wstring>> index;//resulting inverted index
	int thread_count; //number of threads
	vector<int> chunks; // vector, that stores index of chunks of files vector, to share it between threads
	int chunk; // basic chunk, dependant on thread_count
	vector<fs::path> files; // vector, that stores pathes to files
	vector<thread> threads; // vector of threads
	vector<fs::path> paths = { "D:/Coursework/Set1/", "D:/Coursework/Set2/","D:/Coursework/Set3/","D:/Coursework/Set4/","D:/Coursework/Set5/" };//pathes to directory of data set
	//next loop find all files in vector paths, and stores them in vector files 
	for (const auto& path : paths) {
		for (const auto& file : fs::directory_iterator(path)) {
			if(file.is_regular_file()){
				files.push_back(file.path());
			}
		}
	}

	cout << "How many threads do you want?: ";
	cin >> thread_count;
	cout << files.size() << endl;
	chunk = files.size() / thread_count;
	
	//loop, that calculates indexes of chunks of vector file
	if (files.size() % thread_count == 0) {
		for (int i = 0; i < thread_count + 1; i++) {
			chunks.push_back(chunk*i);
		}
	}
	else {
		int size = 0;
		int temp = 0;
		for (int i = 0; i < thread_count; i++) {
			chunks.push_back(chunk*i);
		}
		size = files.size() - temp;
		chunks.push_back(size);
	}

//	for (int i = 0; i < chunks.size(); i++) {
//		cout << chunks[i] << endl;
//	}

	auto start = std::chrono::system_clock::now();
	// creating threads, that will calculate inverted index
	for (int i = 0; i < thread_count; i++) {
		thread t{inverted_index, ref(files), chunks[i], chunks[i+1], ref(index)};
		threads.push_back(move(t));
	}

	//waiting for threads to stop and "killing" them  
	for (int i = 0; i < thread_count; i++) {
		threads[i].join();
	}

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> result = end - start;
	
	//printing full index
	//for (auto it = index.begin(); it != index.end(); ++it) {
	//	wcout << it->first << ": (";
	//	for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
	//		wcout << *it2 << ", ";
	//	cout << ")" << endl;
	//}

	cout <<"Index size: " << index.size() << endl;
	cout << "Time: " << result.count() << endl;
	system("pause");
	return 0;
}