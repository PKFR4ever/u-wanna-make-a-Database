#include <bits/stdc++.h>
using namespace std;

// 类型名用的驼峰
// 变量名/函数名用的下划线分隔

// 标识meta_cmd是否合法
typedef enum{
  META_CMD_SUCCESS,
  META_CMD_UNRECOGNIZED
} MetaCmdResult;

// 标识statement是否合法
typedef enum{
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR
} PrepareResult;

// 标识目前支持的关键字
typedef enum{
  INSERT,
  SELECT
} StatementType;

// 行数据结构
// | id | username | email | 结构的小型数据库
struct Row{
  int id;
  string username;
  string email;
};

struct Statement{
  StatementType type;
  Row row_to_insert; // only use in insert statement
};

// 处理 .开头的meta cmd 这些都不是sql语句
// 目前只能处理.exit
MetaCmdResult do_meta_cmd(string meta_cmd){
  if(meta_cmd == ".exit") exit(0);
  return META_CMD_UNRECOGNIZED;
}

// 检查input_buffer是否是合法的statement 并塞进statement
// 需要解析input_buffer的语法
PrepareResult prepare_statment(string input_buffer, Statement* statement){
  const string insert_str = "insert";
  const string select_str = "select";

  // 参数解析放外面
  istringstream iss(input_buffer);
  vector<string> args;
  string temp;
  while(iss >> temp){
    args.push_back(temp);
  }

  // 目前适配的语法：
  // insert id(int) username(string) email(string)
  if(args[0] == insert_str) {
    if(args.size() != 4) 

    statement->type = INSERT;
    try{ // 处理args[1]不可解析为int的情况
      statement->row_to_insert.id = stoi(args[1]);
    }
    catch (std::invalid_argument){
      return PREPARE_SYNTAX_ERROR;
    }
    statement->row_to_insert.username = args[2];
    statement->row_to_insert.email = args[3];

    return PREPARE_SUCCESS;
  }
  // 目前适配的语法
  // select 
  else if(args[0] == select_str) {
    if(args.size() != 1) return PREPARE_SYNTAX_ERROR;

    statement->type = SELECT;

    return PREPARE_SUCCESS;
  }
  return PREPARE_UNRECOGNIZED_STATEMENT;
}

// 执行statement
void execute_statment(Statement* statement){
  switch (statement->type){
    case (INSERT):{
      cout << "do insert here\n";
      break;
    }
    case (SELECT):{
      cout << "do select here\n";
      break;
    }
  }
}

int main(int argc,char **argv){
  string input_buffer;
  while(1){
    std::cout << "db > ";
    getline(cin, input_buffer);
    // 处理空行退出的bug
    if(input_buffer.size() == 0) continue;

    // 处理 .开头的meta cmd
    if(input_buffer[0] == '.'){
      switch (do_meta_cmd(input_buffer)){
        case (META_CMD_SUCCESS):{
          continue;
        }
        case (META_CMD_UNRECOGNIZED):{
          cout << "Unrecognized command " << input_buffer << '\n';
          continue;
        }
      }
    }
    
    // 处理sql语句
    Statement statement;
    switch (prepare_statment(input_buffer, &statement)) {
      case (PREPARE_SUCCESS):{
        break;
      }
      case (PREPARE_SYNTAX_ERROR):{
        cout << "Syntax error. Could not parse statement.\n";
        continue;
      }
      case (PREPARE_UNRECOGNIZED_STATEMENT):{
        cout << "Unrecognized keyword at start of " << input_buffer << '\n';
        continue;
      }
    }

    execute_statment(&statement);
    cout << "Executed\n";
  }
  return 0;
}