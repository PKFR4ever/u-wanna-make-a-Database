// #include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// using namespace std;

// 类型名用的驼峰
// 变量名/函数名用的下划线分隔
// 宏定义用全大写 + 下划线分隔

#define ID_SIZE 4  // id字段 4 byte
#define USERNAME_SIZE 32 // username字段 32 byte
#define EMAIL_SIZE 256 // email字段 256 byte
#define ID_OFFSET 0 // id字段起始偏移
#define USERNAME_OFFSET ID_SIZE // Iusername字段起始偏移
#define EMAIL_OFFSET (ID_SIZE+USERNAME_SIZE) // email字段起始偏移

#define PAGE_SIZE 4096 // 4kB 一页
#define ROW_SIZE (ID_SIZE+USERNAME_SIZE+EMAIL_SIZE) // 291 byte
#define TABLE_MAX_PAGES 100 // 暂时支持100页大小
#define PAGE_MAX_ROWS (PAGE_SIZE / ROW_SIZE) // 一页多少行 14
#define TABLE_MAX_ROWS (TABLE_MAX_PAGES * PAGE_MAX_ROWS) // 整个table支持的总行数

// 标识meta_cmd是否合法
typedef enum{
  META_CMD_SUCCESS,
  META_CMD_UNRECOGNIZED
} MetaCmdResult;

// 标识statement是否合法
typedef enum{
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR,
  PREPARE_INVALID_ARGS
} PrepareResult;

// 标识execute的结果
typedef enum{
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
  EXECUTE_UNRECOGNIZED_STATEMENT
} ExecuteResult;

// 标识目前支持的关键字
typedef enum{
  INSERT,
  SELECT
} StatementType;

// 行数据结构
// | id | username | email | 结构的小型数据库
struct Row{ // total 291 byte
  int id;  // 4 byte
  char username[USERNAME_SIZE]; // 最多31字符
  char email[EMAIL_SIZE];    // 最多255字符
};

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}


struct Pager{
  int file_descriptor;
  int file_length;
  char* pages[TABLE_MAX_PAGES];

  Pager(const char* filename){
    int fd = open(filename,
                  O_RDWR | O_CREAT,   // r+w, create if not exist
                  S_IWUSR | S_IRUSR); // 允许 r+w
    if(fd == -1){
      std::cout << "Unable to open file\n";
      exit(EXIT_FAILURE);
    }
    auto file_length = lseek(fd, 0, SEEK_END);
    file_descriptor = fd;
    file_length = file_length;
    for(int i=0;i<TABLE_MAX_PAGES;i++){
      pages[i] = nullptr;
    }
  }

  void pager_flush(int page_idx, int size){
    if(pages[page_idx] == nullptr){
      std::cout << "Tried to flush null page\n";
      exit(EXIT_FAILURE);
    }
    auto offset = lseek(file_descriptor, page_idx * PAGE_SIZE, SEEK_SET);
    if(offset == -1){
      std::cout << "Error seeking: " << errno << '\n';
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(file_descriptor, pages[page_idx], size);
    if(bytes_written == -1){
      std::cout << "Error writing " << errno << '\n';
      exit(EXIT_FAILURE);
    }
  }
};

// 通过pager获取table中的page_idx
char* get_page(Pager* pager, int page_idx){
  if(page_idx > TABLE_MAX_PAGES){
    std::cout << "Tried to fetch page number out of bounds.\n";
    exit(EXIT_FAILURE);
  }
  // 分页器中第page_idx页不存在
  if(pager->pages[page_idx] == nullptr){
    // cache miss 分配新page
    char* page = new char[PAGE_SIZE];
    // num_pages: 该pager对应的file中有几个pages
    int num_pages = pager->file_length / PAGE_SIZE;
    if(pager->file_length % PAGE_SIZE) num_pages++;

    // page_idx是在这个pager对应的file中的
    if(page_idx <= num_pages){ 
      lseek(pager->file_descriptor, page_idx * PAGE_SIZE, SEEK_SET);
      auto bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
      if(bytes_read == -1){
        std::cout << "Error reading file: " << errno << '\n';
        exit(EXIT_FAILURE);
      }
    }

    // 把新page放进pager的page_idx位置
    pager->pages[page_idx] = page;
  }
  return pager->pages[page_idx];; 
}

struct Table{
  int num_rows;
  Pager* pager;

  Table(const char* filename){
    pager = new Pager(filename);
    num_rows = pager->file_length / ROW_SIZE;
  }

  void db_close(){
    // table 中多少个满page
    int num_full_pages = num_rows / PAGE_MAX_ROWS;
    for(int i=0;i<num_full_pages;i++){
      if(pager->pages[i] == nullptr) continue;
      pager->pager_flush(i, PAGE_SIZE);
      delete pager->pages[i];
      pager->pages[i] = nullptr;
    }

    int more_rows = num_rows % PAGE_MAX_ROWS;
    if(more_rows > 0){
      int page_idx = num_full_pages;
      if(pager->pages[page_idx]!=nullptr){
        pager->pager_flush(page_idx, PAGE_SIZE);
        delete pager->pages[page_idx];
        pager->pages[page_idx] = nullptr;
      }
    }

    int res = close(pager->file_descriptor);
    if(res == -1){
      std::cout << "Error closing db file.\n";
      exit(EXIT_FAILURE);
    }
    for(int i=0;i<TABLE_MAX_PAGES;i++){
      char* page = pager->pages[i];
      if(page){
        delete page;
        pager->pages[i] = nullptr;
      }
    }
    delete pager;
  }
};

// 返回table中第idx行的开始位置
char* row_slot(Table* table, int idx){
  int page_idx = idx / PAGE_MAX_ROWS;
  char* page = get_page(table->pager, page_idx);
  int row_idx = idx % PAGE_MAX_ROWS;
  char* ret = (char*)page + row_idx * ROW_SIZE;
  return ret;
}

// 把row指向的行放到dest位置
void put_row_to_table(Row* row, char* dest){
  memcpy((char*)dest + ID_OFFSET, &(row->id), ID_SIZE);
  memcpy((char*)dest + USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
  memcpy((char*)dest + EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
  return;
}

// 去除source位置的一行放进row指向的位置
void get_row_from_table(Row* row, char* source){
  memcpy(&(row->id), (char*)source + ID_OFFSET, ID_SIZE);
  memcpy(&(row->username), (char*)source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(row->email), (char*)source + EMAIL_OFFSET, EMAIL_SIZE);
  return;
}

struct Statement{
  StatementType type;
  Row row_to_insert; // only use in insert statement
};

// 处理 .开头的meta cmd 这些都不是sql语句
// 目前只能处理.exit
MetaCmdResult do_meta_cmd(std::string meta_cmd, Table* table){
  if(meta_cmd == ".exit"){
    table->db_close();
    delete table;
    exit(EXIT_SUCCESS);
  }
  return META_CMD_UNRECOGNIZED;
}

// 检查input_buffer是否是合法的statement 并塞进statement
// 需要解析input_buffer的语法
PrepareResult prepare_statment(std::string input_buffer, Statement* statement){
  const std::string insert_str = "insert";
  const std::string select_str = "select";

  // 参数解析放外面
  std::istringstream iss(input_buffer);
  std::vector<std::string> args;
  std::string temp;
  while(iss >> temp){
    args.push_back(temp);
  }

  // 目前适配的语法：
  // insert id(int) username(string) email(string)
  if(args[0] == insert_str) {
    if(args.size() != 4) return PREPARE_SYNTAX_ERROR;

    // 处理参数过长
    if(args[2].size() >= USERNAME_SIZE) return PREPARE_INVALID_ARGS;
    if(args[3].size() >= EMAIL_SIZE) return PREPARE_INVALID_ARGS;

    statement->type = INSERT;
    try{ // 处理args[1]不可解析为int的情况
      statement->row_to_insert.id = stoi(args[1]);
    }
    catch (std::invalid_argument){
      return PREPARE_SYNTAX_ERROR;
    }
    if(statement->row_to_insert.id < 0) return PREPARE_INVALID_ARGS;
    strcpy(statement->row_to_insert.username, args[2].c_str());
    strcpy(statement->row_to_insert.email, args[3].c_str());

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

// 执行insert
ExecuteResult execute_insert(Statement* statement, Table* table){
  // 插入判满
  if(table->num_rows >= TABLE_MAX_ROWS){
    return EXECUTE_TABLE_FULL;
  }
      
  Row* row_to_insert = &(statement->row_to_insert);
  // 把arg0放进arg1返回的位置

  put_row_to_table(row_to_insert, row_slot(table, table->num_rows));

  table->num_rows++;
  return EXECUTE_SUCCESS;
}

// 执行select
ExecuteResult execute_select(Statement* statement, Table* table){
  Row row_now;
  std::cout << '\n';
  for(int i=0;i<table->num_rows;i++){
    get_row_from_table(&row_now, row_slot(table, i));
    print_row(&row_now);
  }
  return EXECUTE_SUCCESS;
}

// 执行statement
ExecuteResult execute_statment(Statement* statement, Table* table){
  ExecuteResult res = EXECUTE_UNRECOGNIZED_STATEMENT;
  switch (statement->type){
    case (INSERT):{
      res = execute_insert(statement, table);
      break;
    }
    case (SELECT):{
      res = execute_select(statement, table);
      break;
    }
  }
  return res;
}

int main(int argc,char **argv){
  if(argc < 2){
    std::cout << "Must supply a database filename.\n";
    exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
  Table* table = new Table(filename);

  std::string input_buffer;
  while(1){
    std::cout << "db > ";
    std::getline(std::cin, input_buffer);
    // 处理空行退出的bug
    if(input_buffer.size() == 0) continue;

    // 处理 .开头的meta cmd
    if(input_buffer[0] == '.'){
      switch (do_meta_cmd(input_buffer, table)){
        case (META_CMD_SUCCESS):{
          continue;
        }
        case (META_CMD_UNRECOGNIZED):{
          std::cout << "Unrecognized command " << input_buffer << '\n';
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
        std::cout << "Syntax error. Could not parse statement\n";
        continue;
      }
      case (PREPARE_UNRECOGNIZED_STATEMENT):{
        std::cout << "Unrecognized keyword at start of " << input_buffer << '\n';
        continue;
      }
      case (PREPARE_INVALID_ARGS):{
        std::cout << "Invalid_args: too long or wrong type or negative\n";
        continue;
      }
    }

    switch (execute_statment(&statement, table)) {
      case (EXECUTE_SUCCESS):{
        std::cout << "Executed\n";
        break;
      }
      case (EXECUTE_TABLE_FULL):{
        std::cout << "Error: Table full\n";
        continue;
      }
      case (EXECUTE_UNRECOGNIZED_STATEMENT):{
        std::cout << "Unrecognized keyword in statement\n";
        continue;
      }
    }
  }
  return 0;
}