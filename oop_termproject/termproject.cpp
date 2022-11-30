#define _CRT_SECURE_NO_WARNINGS

#include<fstream>
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<windows.h>
#include<sstream>

#define MAXBYTE 75

using namespace std;

//�� ���� ��µ� ����
struct Buffer {
	char buf[152] = "";// �Է� �ִ� 75byte
};

vector<Buffer> lines;

int pageNumber = 1;
int searchedLine = -1;
string consolMessage = "";

//base interface
class Action {
public:
	//pure virtual function
	virtual void doAction() = 0;

	//buf ���� ���� 
	void clear_buffer(Buffer& buffer) {
		for (int i = 0; i <= 151; i++) {
			buffer.buf[i] = '\0';
		}
	}

	//���� ����
	void reWrite() {
		vector<Buffer> temp;
		string word, prev, sentance;
		int currentByte = 1;
		char buf[75] = "";
		Buffer buffer;

		word.clear();
		prev.clear();
		sentance.clear();

		temp.push_back(buffer);

		for (int i = 0; i < lines.size(); i++) {
			sentance = lines[i].buf;
			istringstream ss(sentance);

			while (getline(ss, word, ' ')) {
				if (!prev.empty()) {
					for (int i = 0; i < prev.length(); i++) {
						buffer.buf[i] = prev.at(i);
					}
					currentByte += prev.length();
					prev.clear();
				}

				word += ' ';
				currentByte += word.length();

				if (currentByte / MAXBYTE >= 1) {
					prev = word;
					currentByte = 1;
					temp.push_back(buffer);
					clear_buffer(buffer);
					word.clear();
					continue;
				}

				int  j = 0;

				for (int i = currentByte - word.length() - 1; i < currentByte - 1; i++) {
					buffer.buf[i] = word.at(j);
					j++;
				}

				word.clear();

			}
			word.clear();
			sentance.clear();
		}

		temp.push_back(buffer);

		lines.clear();
		lines.assign(temp.begin(), temp.end());
	}
};
//���� �������� �Ѿ��
class NextPage :public Action {
public:
	void doAction() override {
		// ��� �������� ������ 
		if (lines.size() / 20 < pageNumber) {
			consolMessage = "This is the last page!";
			return;
		}
		++pageNumber;
			
		
	}
};

//���� �������� ����
class PrevPage : public Action {
public:
	void doAction() override {
		// ���� �������� ���� �� ����ó��
		if (pageNumber - 1 < 1) {
			consolMessage = "This is the first page!";
			return;
		}
		--pageNumber;
	}
};

//�ܾ� ����
class Insert : public Action {
public:
	//a�� line b�� index c�� �߰��� �ܾ�
	Insert(int a, int b, string& s) : line((pageNumber - 1) * 20 + a), col(b), word(s) {
		//���� �������� ����
		if (a > 20) {
			consolMessage = "error: line out of range ( invalid line : 1<= line <= 20)";
			throw 0;
		}
	}
	~Insert() {};

	void doAction() override {
		
			//���ڿ��� ���ԵǴ� ��
			string temp = lines[line].buf;

			int index = 0;
			//���ڿ� ����
			temp.insert(col, word);
			strcpy(lines[line].buf, temp.c_str());

			// ���� ���� �� 75 ����Ʈ ũ�⸦ ���� �� �ִ�.
			reWrite();
		
		

	}
public:
	int line, col;
	string word;
};

class Delete : public Action {
public:

	Delete(int a, int b, int d) : line((pageNumber - 1) * 20 + a), col(b), bt(d) {
		if (a > 20) {
			// �� ���� �ʰ� 
			consolMessage = "error: line out of range ( invalid line : 1<= line <= 20)";
			throw 0;
		}
	}
	~Delete() {}

	void doAction() override {
		//delete�� ��
		string temp = lines[line].buf;
		//index ���� ����ó��
		if (col<0 || col >75) {
			consolMessage = "error: delete target number input out of range";
			throw 0;
		}
		//���� byte�� ����ó��
		if (bt < 0 || bt>75) {
			consolMessage = "error: delete byte number input out of range";
			throw 0;
		}
		
		//���ڿ� ����
			temp.erase(col, bt);
			strcpy(lines[line].buf, temp.c_str());
		
		

		reWrite();
	}

public:
	int line, col, bt;

};
//�ٸ� �ܾ�� �ٲٱ�
class Change : public Action {
public:
	Change(string oldWord_, string newWord_) : oldWord(oldWord_), newWord(newWord_) {}
	~Change() {}

	void doAction() {
		int lineStart = (pageNumber - 1) * 20 + 1;
		int lineEnd = lineStart + 19;
		string temp;

		for (int i = lineStart; i <= lineEnd; i++) {
			temp = lines[i].buf;
			temp = replaceAll(temp, oldWord, newWord);
			strcpy(lines[i].buf, temp.c_str());
		}

		reWrite();
	}
private:
	string replaceAll(string& str, const string& from, const string& to) {
		size_t start_pos = 0; //stringó������ �˻�
		while ((start_pos = str.find(from, start_pos)) != string::npos)  //from�� ã�� �� ���� ������
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // �ߺ��˻縦 ���ϰ� from.length() > to.length()�� ��츦 ���ؼ�
		}
		return str;
	}

public:
	string oldWord, newWord;
};

class Search : public Action {
public:
	Search(string s_) {
		target = s_;
	}
	~Search() {}
	void doAction() {
		string s;

		for (int i = 0; i < lines.size(); i++) {
			s = lines[i].buf;

			if (s.find(target) <= 75) {
				searchedLine = i;
				break;
			}
		}

	}
public:
	string target;
};

class SaveQuit : public Action {
public:
	void doAction() {
		ofstream write("test.txt");

		for (int i = 0; i < lines.size(); i++) {
			string temp = lines[i].buf;
			write << temp;
		}
	}

};

class Term {
protected:
	Action* action;
	string filename;

public:
	Term(string filename_) : filename(filename_) {
		text_to_vector();
	}

	//vector�� ����� ���� index��ȣ�� �˸°� ���
	void show_page(int page) {
		int start;
		int count = 1;
				
		if (searchedLine != -1) {
			start = searchedLine;
		}
		else {
			start = 20 * (page - 1) + 1;
		}

		int end = start + 19;
		int lineNum;
		//������ ������
		if (end > lines.size()) {
			end = lines.size() - 1;
			for (int i = end-19;i <=end;i++) {
				lineNum = i - 20 * (page - 1);

				if (count < 10) {
					
					cout << " " << count << "| " << lines[i].buf << endl;
					++count;
				}
				
				else {
					
					cout << count << "| " << lines[i].buf << endl;
					++count;

				}
			}
		}
		else {
			for (int i = start; i <= end; i++) {
				lineNum = i - 20 * (page - 1);
				
				if (count < 10) {
					
					cout << " " << count << "| " << lines[i].buf << endl;
					++count;
				}
				else {
					
					cout << count << "| " << lines[i].buf << endl;
					++count;

				}
			}
		}

		searchedLine = -1;
	}

	string get_command() {
		string command;
		
			cin >> command;

			return command;
		
		
	}

	void show_interface() {
		string command;
		show_page(pageNumber);

		cout << "------------------------------------------------------------------------------------------" << endl;
		cout << "n:next page , p:previous page, i:insert, d:delete, c:change, s:search, t:save and quit " << endl;
		cout << "------------------------------------------------------------------------------------------" << endl;
		cout << "(consol message) " << consolMessage << endl;
		cout << "------------------------------------------------------------------------------------------" << endl;
		cout << "input:";
	}
	//Strategy ���� ����
	void setAction(Action* action_) {
		action = action_;
	}

	void doAction() {
		action->doAction();
	}

	void text_to_vector() {
		ifstream file;
		file.open(filename);
		string word, prev;
		int currentByte = 1;
		char buf[MAXBYTE] = "";
		Buffer buffer;

		lines.push_back(buffer);	// �� ���� ���� ����

		word.clear();
		prev.clear();

		while (file >> word) {

			if (!prev.empty()) {
				// ���� �ٿ��� 75 ����Ʈ�� �Ѿ �ܾ ���� ó�� 
				for (int i = 0; i < prev.length(); i++) {
					buffer.buf[i] = prev.at(i);
				}
				currentByte += prev.length();
				prev.clear();
			}

			word += ' ';
			currentByte += word.length();

			if (currentByte / MAXBYTE >= 1) {
				prev = word;				// 75 BYTE�� �ѱ�� ��� ���� �ٿ� �ѱ�� ���� ����
				currentByte = 1;
				lines.push_back(buffer);
				clear_buffer(buffer);
				word.clear();
				continue;
			}

			int  j = 0;

			for (int i = currentByte - word.length() - 1; i < currentByte - 1; i++) {
				// �ܾ ���� �ٿ� ����
				buffer.buf[i] = word.at(j);
				j++;
			}
			word.clear();
		}
		lines.push_back(buffer);	// ������ �ٿ� ���� ó�� 
		file.close();
	}

	void clear_buffer(Buffer& buffer) {
		for (int i = 0; i <= 151; i++) {
			buffer.buf[i] = '\0';
		}
	}
};

//�Է¿��� �����͸� �����ϱ� ���� ���� �Լ�
void string_data_handling(string command, vector<string>& splitedData) {
	
		string data = command.substr(2, command.length() - 3);
		string stringBuf;
		istringstream ss(data);

		try {
			while (getline(ss, stringBuf, ',')) {
			if (stringBuf.length() > MAXBYTE) {
				consolMessage = "string parameter must be under 75 byte";
				throw 0;
			}

			for (int i = 0; i < stringBuf.length(); i++) {
				if (stringBuf[i] == ' ') {
					throw 0;
				}
			}

			splitedData.push_back(stringBuf);
		}
	}
	catch (...) {
		consolMessage = "invaild input";
	}

	}



//�Է¿� ���� ��� ����
void main() {
	string command;
	string data;	// ����� �Է°� ������ �����
	string stringBuf;
	vector<string> splitedData;
	Term* t = new Term("test.txt");
	Action* next = new NextPage;
	Action* prev = new PrevPage;;
	Action* saveQuit = new SaveQuit;
	int a, b, d;
	string test;

	do {
		try{
		splitedData.clear();
		t->show_interface();
		command = t->get_command();

		consolMessage.clear();
		string N = "n";
		string P = "p";
		
			if (command.compare(N) == 0) {
				t->setAction(next);
				t->doAction();
			}
			
			else if (command.compare(P) == 0) {
				t->setAction(prev);
				t->doAction();
			}
			else if (command.at(0) == 'i') {
				// insert ����

				string_data_handling(command, splitedData);
				test = splitedData.at(0);
				test = splitedData.at(1);
				stringstream(splitedData[0]) >> a;
				stringstream(splitedData[1]) >> b;

				Insert* insert = new Insert(a, b, splitedData[2]);

				t->setAction(insert);
				t->doAction();

				delete(insert);

			}
			else if (command.at(0) == 'd') {
				// delete ����			
				
					string_data_handling(command, splitedData);
					test = splitedData.at(0);
					test = splitedData.at(1);
					test = splitedData.at(2);
					stringstream(splitedData[0]) >> a;
					stringstream(splitedData[1]) >> b;
					stringstream(splitedData[2]) >> d;
					Delete* del = new Delete(a, b, d);



					t->setAction(del);
					t->doAction();

					delete del;
				
				
			}
			else if (command.at(0) == 's') {
				// �ؽ�Ʈ ���� ó������ Ž��, ù��° ��� â�� ù��° ���ο� ��ġ�ϵ��� ���
				string_data_handling(command, splitedData);
				Search* search = new Search(splitedData[0]);
				t->setAction(search);
				t->doAction();

				delete search;
			}
			else if (command.at(0) == 'c') {
				// �����ϰ� ���� �ܾ� ��� �ٸ� �ܾ�� ����
				string_data_handling(command, splitedData);

				Change* change = new Change(splitedData[0], splitedData[1]);
				t->setAction(change);
				t->doAction();
				delete(change);
			}
			else if (!command.compare("t")) {

				t->setAction(saveQuit);
				t->doAction();
			}
			else {
				consolMessage = "error: invalid command";
			}
		}
		catch (...) {
			if (consolMessage.empty()) {
				consolMessage = "error: invalid input, check blank, invalid input type, etc...";
			}
		}
		cout << "------------------------------------------------------------------------------------------" << endl;
		Sleep(100);

		system("cls");
	} while (command.compare("t"));

	delete next;
	delete prev;
	delete saveQuit;
}