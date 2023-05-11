#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <map>
#include <stack>
#include <sstream>
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
struct tableParseElem
{
   vector<string> terminal_; // Терминалы
   int jump_;                // Переход
   int accept_;              // Принимать или нет
   int stack_;               // Cтек
   int return_;              // Возвращать или нет
   int error_;               // Oшибка
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
   string getElem(Token token) {
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
const string parser_output = "token_file.txt";
const string postfix_output = "postfix.txt";
const string errors = "errors.txt";
const string parse_table = "parse_table.txt";



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
      case KEY_WORDS_TABLE_ID: str = keyWords.getElem(token); break;
      case OPERATIONS_TABLE_ID: str = operations.getElem(token); break;
      case DELIMETERS_TABLE_ID: str = delimiters.getElem(token); break;
      case VALUES_TABLE_ID: value = valueTable.getElem(token); break;
      case VARIABLE_TABLE_ID: var = variableTable.getElem(token); break;
      }
   }
   string getElemByTokenString(Token token)
   {
      switch (token.table) {
      case 0: return keyWords.getElem(token); break;
      case 1: return operations.getElem(token); break;
      case 2: return delimiters.getElem(token); break;
      case 3: return to_string(valueTable.getElem(token)); break;
      case 4: return variableTable.getElem(token).name; break;
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
            if ((token = operations.search(operation_buffer + tmp)).index != -1)
            {
               PrintToken(fout, token);
               operation_buffer = "";
               is_operation = 0;
               continue;
            }
            else if ((token = operations.search(operation_buffer)).index != -1)
            {
               PrintToken(fout, token);
               operation_buffer = "";
               is_operation = 0;
            }
         }
         //main part

         if ((token = delimiters.search(string(1, tmp))).index != -1)
         {
            PrintToken(fout, token);
            continue;
         }


         do
         {
            if ((delim_token = delimiters.search(string(1, tmp))).index != -1)
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

         if ((token = keyWords.search(lexeme.str)).index != -1)
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
         if ((token = operations.search(operation_buffer)).index != -1)
            PrintToken(fout, token);
      }
   }

   void automatParse(ifstream &fin, ofstream &fout) {
      //init
      makeP_matrix();
      Token token;
      char tmp;
      int i = 0, j = 0, prev_automat = 0;
      tmp = read_char(fin, &j);

      while (j != 5)
      {
         string buff = "";
         while (true)
         {
            if (i != 0) tmp = read_char(fin, &j);
            prev_automat = i;
            i = P_matrix[i][j];
            //cout << "automat" << i << endl;
            if (i == 9)
            {
               switch (prev_automat)
               {
               case 1:
                  token = valueTable.findOrCreate(buff);
                  PrintToken(fout, token);
                  break;
               case 2:
                  if ((token = keyWords.search(buff)).index == -1)
                     token = variableTable.findOrCreate(buff);
                  PrintToken(fout, token);
                  break;
               case 3:
               case 4:
                  token = operations.search(buff);
                  PrintToken(fout, token);
                  break;
               case 7:
                  tmp = read_char(fin, &j); // чтобы слеш после коммента не учитывался
                  break;
               case 8:
                  token = delimiters.search(buff);
                  PrintToken(fout, token);

               default:
                  break;
               }
               break;
            }
            if (i == 10) {
               switch (j)
               {
               case 1:
                  fout << "unrecognized const error";
                  break;
               case 2:
               case 7:
               case 8:
                  fout << "oprator error";
                  break;
               case 5:
                  fout << "eof error";
                  break;
               case 6:
                  fout << "unrecognized symbol errror";
                  break;
               default:
                  fout << "unexpected error";
                  break;
               }
               exit(1);
            }
            buff += tmp;
         }
         i = 0;
      }

   }
   void parse_tokens(vector<tableParseElem> &table_)
   {
      ofstream postfix_stream(postfix_output);
      ofstream error_stream(errors);
      map<string, int> priority;
      priority["="] = 1;
      priority["<"] = 6;
      priority[">"] = 6;
      priority["!="] = 6;
      priority["=="] = 6;
      priority["+"] = 7;
      priority["-"] = 7;
      priority["*"] = 8;
      vector<string> initialized_vars;
      stack<int> stack_;
      ifstream in(parser_output);
      bool is_open;
      int index = 1;
      int table;
      int hash;
      int idx;
      in >> table;
      in >> hash;
      in >> idx;
      int depth = -1;
      string temp_var;
      vector<string> poland;
      stack<string> operands;
      Token token(table, hash, idx);
      bool is_switch = 0;
      int switch_depth = -1;
      int current_L = 0;
      int current_switch_end = 0;
      stack<int> switch_end;
      stack<int> l_index_stack;
      stack<bool> was_default;
      vector<vector<string>> poland_switch;
      //bool was_initialized = 0;
      string string_token;
      string_token = getElemByTokenString(token);
      while (index > 0)
      {
         cout << index << " " << string_token << " " << stack_.size() << endl;
         if (compare_terminals(table_[index - 1].terminal_, string_token, token))
         {
            //switch condition
            if (index == 23)
            {
               if (!is_switch)
               {
                  if (switch_depth == -1)
                  {
                     switch_depth = stack_.size();
                  }
                  is_switch = 1;
               }
               was_default.push(0);
               switch_end.push(current_switch_end);
               current_switch_end++;
            }
            //break
            if (index == 44 || index == 39)
            {
               postfix_stream << 'B' << switch_end.top() << ' ' << "UJ" << ' ' << endl;
            }
            //next case\default
            if ((index == 40 || index == 45) && (string_token != "}"))
            {
               if (string_token == "default")
               {
                  was_default.pop();
                  was_default.push(1);
               }
               postfix_stream << 'L' << l_index_stack.top() << ':' << ' ';
               l_index_stack.pop();
            }

            //case const
            if (index == 28)
            {
               for (int switch_poland_index = 0; switch_poland_index < poland_switch[stack_.size() - switch_depth].size(); switch_poland_index++)
               {
                  postfix_stream << poland_switch[stack_.size() - switch_depth][switch_poland_index] << ' ';
               }
               postfix_stream << string_token << ' ';
               postfix_stream << "==";
               postfix_stream << endl;
               postfix_stream << 'L' << current_L << ' ' << "CJ" << ' ' << endl;
               l_index_stack.push(current_L);
               current_L++;
            }
            // switch }
            if ((index == 33) || (index == 27))
            {
               if (switch_depth == stack_.size())
               {
                  poland_switch.resize(0);
               }

               postfix_stream << 'B' << switch_end.top() << ':' << ' ';
               switch_end.pop();

               if (!was_default.top())
               {
                  postfix_stream << 'L' << l_index_stack.top() << ':' << ' ';
                  l_index_stack.pop();
               }

            }

            //слагаемое выражения
            if (index == 10 || index == 14 || index == 8)
            {
               if (!is_initialized(initialized_vars, string_token) && token.table == VARIABLE_TABLE_ID)
               {
                  error_stream << "uninitialized variable";
                  exit(-1);
               }
               if (depth == -1 && index != 8)
               {
                  depth = stack_.size();
               }
               if (index != 8)
               {
                  poland.push_back(string_token);
               }
            }
            //скобки
            if (index == 11 || index == 15)
            {
               if (depth == -1)
               {
                  depth = stack_.size();
               }
               is_open = 1;
               stack_push(operands, string_token, poland, priority);
            }
            //операнд
            if (index == 12 || index == 16)
            {
               stack_push(operands, string_token, poland, priority);
            }
            //закрывающая скобка но не конечная
            if (index == 13)
            {
               if (depth != stack_.size())
               {
                  stack_push(operands, string_token, poland, priority);
               }
               depth = stack_.size();
            }
            //окончание выр-я
            if (index == 13 || index == 17)
            {
               if (depth == stack_.size())
               {
                  //was_initialized = 0;
                  while (!operands.empty())
                  {
                     poland.push_back(operands.top());
                     operands.pop();
                  }

                  if (is_switch)
                  {
                     poland_switch.push_back(poland);
                     is_switch = 0;
                  }
                  else
                  {
                     for (int poland_index = 0; poland_index < poland.size(); poland_index++)
                     {
                        postfix_stream << poland[poland_index] << ' ';
                     }
                     postfix_stream << endl;
                  }
                  poland.resize(0);
                  depth = -1;

               }
            }

            if (table_[index - 1].accept_ == 1)
            {
               in >> token.table;
               in >> token.hash;
               in >> token.index;
               string_token = getElemByTokenString(token);
               //изменение переменной
               if (index == 18 && token.table == VARIABLE_TABLE_ID)
               {
                  depth = stack_.size();
                  //poland.push_back(string_token);
                  if (!is_initialized(initialized_vars, string_token))
                  {
                     //	was_initialized = 1;
                     initialized_vars.push_back(string_token);
                  }
                  //operands.push("=");
                  temp_var = string_token;
               }
               if (index == 19)
               {
                  if (string_token == "=")
                  {
                     poland.push_back(temp_var);
                     stack_push(operands, string_token, poland, priority);
                  }
               }
               //если она не менялась на самом деле
               //	if (index == 22 && depth != -1)
               //	{
               //		was_initialized = 0;
               //		initialized_vars.pop_back();
               //		depth = -1;
               //		poland.resize(0);
               //		operands.empty();
               //	}
            }
            if (table_[index - 1].stack_ == 1)
            {
               stack_.push(index + 1);
            }
            if (table_[index - 1].return_ == 1)
            {
               index = stack_.top();
               stack_.pop();
            }
            else
            {
               index = table_[index - 1].jump_;
            }

         }
         else
         {
            // вставить ошибку
            if (table_[index - 1].error_ == 1)
            {
               error_stream << "Parsing error";
               exit(-2);
            }
            else
            {
               index++;
            }
         }
      }
   }

private:
   void PrintToken(ofstream &fout, Token token)
   {
      fout << token.table << " " << token.hash << " " << token.index << endl;
   }
   char read_char(ifstream &fin, int *automat_idx)
   {
      char tmp;
      fin.get(tmp);
      if (!fin.good())
      {
         *automat_idx = 5; // EOF
         return NULL;
      }
      if (tmp >= '0' && tmp <= '9') *automat_idx = 0; // цифра
      else if (tmp >= 'A' && tmp <= 'Z' || tmp >= 'a' && tmp <= 'z') *automat_idx = 1; // буква
      else if (tmp == '+' || tmp == '-' || tmp == '!' || tmp == '>' || tmp == '<') *automat_idx = 2; // оператор (исключая = и *)
      else if ((delimiters.search(string(1, tmp))).index != -1) *automat_idx = 3; // разделитель
      else if (tmp == '/') *automat_idx = 4; // слеш
      else if (tmp == '=') *automat_idx = 7;
      else if (tmp == '*') *automat_idx = 8;
      else if (tmp == '\n') *automat_idx = 9;
      else if (tmp == ' ' or tmp == '\t') *automat_idx = 10;
      else *automat_idx = 6;
      return tmp;
   }
   vector<vector<int>> P_matrix;
   void makeP_matrix()
   {
      P_matrix = {
         { 1,  2,  3,  8,  4,  9, 10,  3,  3, 11, 11},
         { 1, 10,  9,  9,  9,  9, 10,  9,  9,  9,  9},
         { 2,  2,  9,  9,  9,  9, 10,  9,  9,  9,  9},
         { 9,  9,  9,  9,  9,  9, 10,  3, 10,  9,  9},
         { 9,  9, 10,  9,  6, 10, 10,  3,  5,  9,  9},
         { 5,  5,  5,  5,  5, 10,  5,  5,  7,  5,  5},
         { 6,  6,  6,  6,  6,  9,  6,  6,  6,  9,  6},
         { 5,  5,  5,  5,  9, 10,  5,  5,  7,  5,  5},
         { 9,  9,  9,  9,  9,  9, 10,  9,  9,  9,  9},
         {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
         {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
         { 9,  9,  9,  9,  9,  9, 10,  9,  9,  9,  9},
      };

   }
   bool compare_terminals(vector<string> terminal_string, string string, Token token)
   {
      int size = terminal_string.size();
      if (token.table == VARIABLE_TABLE_ID)
         string = "var";
      if (token.table == VALUES_TABLE_ID)
         string = "const";
      for (int i = 0; i < size; i++)
      {
         if (terminal_string[i] == string)
         {
            return 1;
         }
      }
      return 0;
   }

   void stack_push(stack<string> &stack, string operand, vector<string> &poland, map<string, int> &priority)
   {
      if (stack.empty())
      {
         stack.push(operand);
         return;
      }

      if (operand == ")")
      {
         while (stack.top() != "(")
         {
            poland.push_back(stack.top());
            stack.pop();
            if (stack.empty())
               break;
         }
         if (!stack.empty())
            stack.pop();
         return;
      }
      if (operand == "(")
      {
         stack.push(operand);
         return;
      }
      if (!(priority[operand] < priority[stack.top()]))
      {
         stack.push(operand);
         return;
      }
      while (!stack.empty())
      {
         if (priority[operand] < priority[stack.top()])
         {
            poland.push_back(stack.top());
            stack.pop();
         }
         else
         {
            break;
         }
      }
      poland.push_back(operand);
      return;
   }

   bool is_initialized(vector<string> &init_vec, string var)
   {
      for (int i = 0; i < init_vec.size(); i++)
         if (init_vec[i] == var)
            return 1;
      return 0;
   }


};


int main()
{
   auto lexeme = Lexeme();
   ifstream fin("code.txt");
   ofstream fout("token_file.txt");
   //lexeme.Parse(fin, fout);
   lexeme.automatParse(fin, fout);


   // чтение таблицы
   vector<tableParseElem> table;
   ifstream tabl(parse_table);
   while (!tabl.eof())
   {
      tableParseElem tmp;
      string string_tmp;
      getline(tabl, string_tmp);
      istringstream ss(string_tmp);
      vector<string> v;
      string t;
      while (ss >> t)
      {
         v.push_back(t);
      }
      int string_size = v.size();
      int count = 0;
      while (string_size - 5 > 0)
      {
         tmp.terminal_.push_back(v[count]);
         count++;
         string_size--;
      }
      tmp.jump_ = stoi(v[count]);
      count++;
      tmp.accept_ = stoi(v[count]);
      count++;
      tmp.stack_ = stoi(v[count]);
      count++;
      tmp.return_ = stoi(v[count]);
      count++;
      tmp.error_ = stoi(v[count]);
      table.push_back(tmp);
      //std::cout << tmp.terminal_[0] << endl;
   }
   lexeme.parse_tokens(table);



   return 0;
}

