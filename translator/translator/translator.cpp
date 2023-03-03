#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
using namespace std;


struct Token {
   // table id
   int table;
   // hash, using for ordering
   int hash;
   // index of element inside hash array
   int index;
   Token(int table, int hash, int index) :table(table), hash(hash), index(index) {}
   Token() :table(0), hash(0), index(0) {}
};

struct VariableTableElem {
   VariableTableElem(string name, string type, bool isInit) :name(name), type(type), isInit(isInit) {}
   VariableTableElem() : name(""), type(""), isInit(0) {}

   // type attribute
   string type;
   // name attribute
   string name;
   // init attribute
   bool isInit;

   // using for compare elements in find function
   bool operator == (string b)
   {
      return name == b;
   }
};
struct buffer
{
   string str;
   // 1=костанта
   bool type;
};


class ValueTable
{
public:
   const int TABLE_SIZE = 19;
   
   ValueTable(int table_id)
   {
      this->table_id = table_id;
      table.resize(TABLE_SIZE);
   };

   // return token of founded or created element
   Token findOrCreate(string value)
   {
      int temp = stoi(value);
      Token token;
      int hash = getHash(value);
      token.table = table_id;
      token.hash = hash;

      // if there is no elements in hash array
      if (table[hash].size()) {
         auto res = find(table[hash].begin(), table[hash].end(), temp);

         // if element was not found
         if (res == table[hash].end())
         {
            table[hash].push_back(temp);
            token.index = table[hash].size() - 1;
         }
         // if elem exists, return index of this elem
         else token.index = res - table[hash].begin();
      }
      else {
         // if element with such hash was not init
         table[hash].push_back(temp);
         token.index = 0;
      }
      return token;
   }

   // get elem by token
   int getElem(Token key) {
      return table[key.hash][key.index];
   }
private:
   vector<vector<int>> table;
   int table_id;
   // get hash, order by first digit
   int getHash(string h) {
      if (h[0] == '-')
         return (h[1] - '0' + 9);
      else
         return(h[0] - '0');
      return -1;
   }
};



class ConstTable
{
public:
   ConstTable(string fileName, int table_id)
   {
      this->table_id = table_id;
      readTable(fileName);
   };

   // search elem by name, if it wont be found, token with index = -1
   Token search(string name)
   {
      auto res = find(table.begin(), table.end(), name);
      if (res == table.end())
         return Token(table_id, 0, -1);
      else
         return Token(table_id, 0, res - table.begin());
   }

   // get elem by token
   string get(Token token) {
      return table[token.index];
   }
private:
   vector<string> table;
   int table_id;
   void readTable(string fileName)
   {
      string temp;
      fstream in(fileName);
      while (!(in.eof())) {
         in >> temp;
         table.push_back(temp);
      }
      in.close();
   }
};

const string KeyWordsFile = "words.txt";
const string OperationsFile = "operations.txt";
const string DelimitersFile = "delimeters.txt";



class VariableTable
{
public:
   const int MAX_ELEM_COUNT = 53;

   VariableTable(int table_id)
   {
      this->table_id = table_id;
      table.resize(MAX_ELEM_COUNT);
   };

   // return token with existing element or created element
   Token findOrCreate(string name)
   {
      Token token;
      int hash = getHash(name);
      token.table = table_id;
      token.hash = hash;

      // if there is at least one elem in table with such hash
      if (table[hash].size()) {

         auto res = find(table[hash].begin(), table[hash].end(), name);

         // if there is no element with such name, add this elem
         if (res == table[hash].end())
         {
            table[hash].push_back(VariableTableElem(name, "", 0));
            token.index = table[hash].size() - 1;
         }
         // if elem exists, set index of this elem to token
         else token.index = res - table[hash].begin();
      }
      // if there is no elem with this hash
      else {
         table[hash].push_back(VariableTableElem(name, "", 0));
         token.index = 0;
      }

      return token;
   }

   // set init status by token
   void setInit(Token token, bool isInit) {
      table[token.hash][token.index].isInit = isInit;
   }

   // set status type by token
   void setType(Token token, string type) {
      table[token.hash][token.index].type = type;
   }

   // get init status
   bool getInit(Token token)
   {
      return table[token.hash][token.index].isInit;
   }

   // get status type
   string getType(Token token)
   {
      return table[token.hash][token.index].type;
   }

   // get elem by token
   VariableTableElem getElem(Token token) {
      return table[token.hash][token.index];
   }
private:
   vector <vector<VariableTableElem>> table;
   int table_id;
   // return hash as first symbol code
   int getHash(string h) {

      if (h[0] >= 'A' && h[0] <= 'Z')
         return (h[0] - 'A');
      if (h[0] >= 'a' && h[0] <= 'z')
         return (h[0] - 'a' + 26);
      if (h[0] == '_')
         return 52;
      return -1;
   }
};

class Lexeme
{
public:

   static const int KEY_WORDS_TABLE_ID = 0;
   static const int OPERATIONS_TABLE_ID = 1;
   static const int DELIMETERS_TABLE_ID = 2;
   static const int VALUES_TABLE_ID = 3;
   static const int VARIABLE_TABLE_ID = 4;

   ConstTable keyWords = ConstTable(KeyWordsFile, KEY_WORDS_TABLE_ID);
   ConstTable operations = ConstTable(OperationsFile, OPERATIONS_TABLE_ID);
   ConstTable delimiters = ConstTable(DelimitersFile, DELIMETERS_TABLE_ID);
   ValueTable valueTable = ValueTable(VALUES_TABLE_ID);
   VariableTable variableTable = VariableTable(VARIABLE_TABLE_ID);
   string str;
   int value;
   VariableTableElem var;
   void getElemByToken(Token token) {
      switch (token.table) {
      case KEY_WORDS_TABLE_ID: str = keyWords.get(token); break;
      case OPERATIONS_TABLE_ID: str = operations.get(token); break;
      case DELIMETERS_TABLE_ID: str = delimiters.get(token); break;
      case VALUES_TABLE_ID: value = valueTable.getElem(token); break;
      case VARIABLE_TABLE_ID: var = variableTable.getElem(token); break;
      }
   }

   void Parse(ifstream &fin, ofstream &fout)
   {
      char tmp;
      bool is_line_comment = 0;
      bool is_long_comment = 0;
      buffer lexeme;
      lexeme.str = "";
      
      Token delim_token;
      string operation_buffer = "";
      bool is_operation = 0;
      while (fin.get(tmp))
      {
         lexeme.type = 0;
         bool is_delim = 0;
         if (is_line_comment)
         {
            if (tmp == '\n')
            {
               is_line_comment = 0;
               lexeme.str = "";
            }
            continue;
         }

         if (is_long_comment)
         {
            if (tmp == '*')
            {
               lexeme.str = "*";
               continue;
            }
            if (lexeme.str == "*")
            {
               if (tmp == '/')
                  is_long_comment = 0;
               lexeme.str = "";
               continue;
            }
            continue;
         }

         //commentary start
         if ((tmp == '/') && (!is_line_comment) && (!is_long_comment))
         {
            if (lexeme.str == "/")
            {
               is_line_comment = 1;
               lexeme.str == "";
               continue;
            }
            lexeme.str = '/';
            continue;
         }

         if (lexeme.str == "/")
         {
            if (tmp == '*')
            {
               is_long_comment = 1;
               lexeme.str == "";
               continue;
            }
         }

         if ((tmp == ' ') || (tmp == '\n'))
            continue;

         Token token;
         if (is_operation)
         {
            if ((token=operations.search(operation_buffer + tmp)).index != -1)
            {
               PrintToken(fout, token);
               operation_buffer = "";
               is_operation = 0;
               continue;
            }
            else if ((token=operations.search(operation_buffer)).index != -1)
            {
               PrintToken(fout, token);
               operation_buffer = "";
               is_operation = 0;
            }
         }
         //main part

         if ((token=delimiters.search(string(1, tmp))).index != -1)
         {
            PrintToken(fout, token);
            continue;
         }


         do
         {
            if ((delim_token=delimiters.search(string(1, tmp))).index != -1)
            {
               is_delim = 1;
               break;
            }

            if ((token = operations.search(string(1, tmp))).index != -1)
            {
               operation_buffer = tmp;
               is_operation = 1;
               break;
            }

            if ((tmp >= '0') && (tmp <= '9'))
            {
               lexeme.type = 1;
            }
            else
               if (lexeme.type == 1)
               {
                  fout << "unexpected const";
                  exit(0);
               }

            lexeme.str += tmp;
         } while ((fin.get(tmp)) && (tmp != ' '));

         if (lexeme.type == 1)
         {
            token = valueTable.findOrCreate(lexeme.str);
            PrintToken(fout, token);
            if (is_delim) PrintToken(fout, delim_token);
            lexeme.str = "";
            continue;
         }

         if ((token=keyWords.search(lexeme.str)).index != -1)
         {
            PrintToken(fout, token);
            if (is_delim) PrintToken(fout, delim_token);
            lexeme.str = "";
            continue;
         }
         token = variableTable.findOrCreate(lexeme.str);
         PrintToken(fout, token);

         if (is_delim) PrintToken(fout, delim_token);

         lexeme.str = "";
         continue;
      }
      if (is_long_comment)
         fout << "commentary is not closed";
      if (is_operation)
      {
         Token token;
         if ((token=operations.search(operation_buffer)).index != -1)
            PrintToken(fout, token);
      }
   } 

private:
   void PrintToken(ofstream &fout, Token token)
   {
      fout << token.table << " " << token.hash << " " << token.index << endl;
   }
};


int main()
{
   auto lexeme = Lexeme();
   ifstream fin("code.txt");
   ofstream fout("token_file.txt");
   lexeme.Parse(fin, fout);
   return 0;
}

