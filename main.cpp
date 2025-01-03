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
  PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

// 标识目前支持的关键字
typedef enum{
  INSERT,
  SELECT
} StatementType;

struct Statement{
  StatementType type;
};

// 处理 .开头的meta cmd 这些都不是sql语句
// 目前只能处理.exit
MetaCmdResult do_meta_cmd(string meta_cmd){
  if(meta_cmd == ".exit") exit(0);
  return META_CMD_UNRECOGNIZED;
}

// 检查input_buffer是否是合法的statement 并塞进statement
PrepareResult prepare_statment(string input_buffer, Statement* statement){
  const string insert_str = "insert";
  const string select_str = "select";
  if(input_buffer.substr(0,insert_str.size()) == insert_str) {
    statement->type = INSERT;
    return PREPARE_SUCCESS;
  }
  else if(input_buffer.substr(0,select_str.size()) == select_str) {
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
      case (PREPARE_UNRECOGNIZED_STATEMENT):{
        cout << "Unrecognized keyword at start of  " << input_buffer << '\n';
        continue;
      }
    }

    execute_statment(&statement);
    cout << "Executed\n";
  }
  return 0;
}