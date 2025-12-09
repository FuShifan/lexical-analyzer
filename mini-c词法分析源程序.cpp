#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <map>
#include <set>

using namespace std;

// Token类型定义
enum TokenType {
    // 关键字 (1-10)
    KW_INT = 1, KW_FLOAT, KW_REAL, KW_VOID, KW_IF, 
    KW_ELSE, KW_WHILE, KW_FOR, KW_RETURN,
    
    // 运算符和符号 (20-49)
    OP_PLUS = 20, OP_MINUS, OP_MULT, OP_DIV, OP_MOD,
    OP_ASSIGN, OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_INC, OP_DEC, OP_NOT, OP_POWER,
    
    // 分隔符 (50-59)
    SEP_LPAREN = 50, SEP_RPAREN, SEP_LBRACKET, SEP_RBRACKET,
    SEP_LBRACE, SEP_RBRACE, SEP_SEMICOLON, SEP_COMMA, SEP_DOT,
    
    // 标识符和常量 (100-101)
    ID = 100, NUM = 101
};

// 关键字表
map<string, TokenType> keywords = {
    {"int", KW_INT}, {"float", KW_FLOAT}, {"real", KW_REAL}, 
    {"void", KW_VOID}, {"if", KW_IF}, {"else", KW_ELSE},
    {"while", KW_WHILE}, {"for", KW_FOR}, {"return", KW_RETURN}
};

// 判断是否为标识符字符
bool isIdChar(char c) {
    return isalnum(c) || c == '_';
}

// 词法分析主函数
struct Token {
    string lexeme;
    TokenType type;
    int code;
};

class Lexer {
private:
    string input;
    size_t pos;
    
public:
    Lexer(const string& str) : input(str), pos(0) {}
    
    // 跳过空白字符
    void skipWhitespace() {
        while (pos < input.size() && isspace(input[pos])) {
            pos++;
        }
    }
    
    // 跳过注释 - 修复版本
    bool skipComment() {
        // 检查是否是单行注释
        if (pos < input.size() - 1 && input[pos] == '/' && input[pos + 1] == '/') {
            // 跳过整行注释
            while (pos < input.size() && input[pos] != '\n') {
                pos++;
            }
            // 跳过换行符
            if (pos < input.size() && input[pos] == '\n') {
                pos++;
            }
            return true; // 表示跳过了注释
        }
        return false; // 没有注释
    }
    
    // 识别数字（整数或浮点数）
    Token scanNumber() {
        Token token;
        size_t start = pos;
        
        // 读取整数部分
        while (pos < input.size() && isdigit(input[pos])) {
            pos++;
        }
        
        // 检查是否有小数点
        if (pos < input.size() && input[pos] == '.') {
            pos++; // 跳过小数点
            // 读取小数部分
            while (pos < input.size() && isdigit(input[pos])) {
                pos++;
            }
        }
        
        token.lexeme = input.substr(start, pos - start);
        token.type = NUM;
        token.code = NUM;
        return token;
    }
    
    // 识别标识符或关键字
    Token scanIdentifier() {
        Token token;
        size_t start = pos;
        
        // 第一个字符可以是字母或下划线
        while (pos < input.size() && isIdChar(input[pos])) {
            pos++;
        }
        
        token.lexeme = input.substr(start, pos - start);
        
        // 检查是否为关键字
        if (keywords.find(token.lexeme) != keywords.end()) {
            token.type = keywords[token.lexeme];
            token.code = keywords[token.lexeme];
        } else {
            token.type = ID;
            token.code = ID;
        }
        
        return token;
    }
    
    // 识别运算符和符号 - 修改版本
    Token scanOperator() {
        Token token;
        char c = input[pos];
        
        // 特殊处理：在扫描运算符前再次检查是否是注释
        // 这是额外的安全检查
        if (c == '/' && pos < input.size() - 1 && input[pos + 1] == '/') {
            // 这种情况应该已经被skipComment()处理了
            // 但为了安全起见，这里也返回一个无效token
            token.code = -1;
            return token;
        }
        
        // 检查双字符运算符
        if (pos < input.size() - 1) {
            string twoChar = input.substr(pos, 2);
            
            if (twoChar == "++") {
                token.lexeme = "++";
                token.type = OP_INC;
                token.code = OP_INC;
                pos += 2;
                return token;
            } else if (twoChar == "--") {
                token.lexeme = "--";
                token.type = OP_DEC;
                token.code = OP_DEC;
                pos += 2;
                return token;
            } else if (twoChar == "==") {
                token.lexeme = "==";
                token.type = OP_EQ;
                token.code = OP_EQ;
                pos += 2;
                return token;
            } else if (twoChar == "!=") {
                token.lexeme = "!=";
                token.type = OP_NE;
                token.code = OP_NE;
                pos += 2;
                return token;
            } else if (twoChar == "<=") {
                token.lexeme = "<=";
                token.type = OP_LE;
                token.code = OP_LE;
                pos += 2;
                return token;
            } else if (twoChar == ">=") {
                token.lexeme = ">=";
                token.type = OP_GE;
                token.code = OP_GE;
                pos += 2;
                return token;
            }
        }
        
        // 单字符运算符和符号
        pos++;
        token.lexeme = c;
        
        switch (c) {
            case '+': token.type = OP_PLUS; token.code = OP_PLUS; break;
            case '-': token.type = OP_MINUS; token.code = OP_MINUS; break;
            case '*': token.type = OP_MULT; token.code = OP_MULT; break;
            case '/': token.type = OP_DIV; token.code = OP_DIV; break;
            case '%': token.type = OP_MOD; token.code = OP_MOD; break;
            case '=': token.type = OP_ASSIGN; token.code = OP_ASSIGN; break;
            case '<': token.type = OP_LT; token.code = OP_LT; break;
            case '>': token.type = OP_GT; token.code = OP_GT; break;
            case '!': token.type = OP_NOT; token.code = OP_NOT; break;
            case '^': token.type = OP_POWER; token.code = OP_POWER; break;
            case '(': token.type = SEP_LPAREN; token.code = SEP_LPAREN; break;
            case ')': token.type = SEP_RPAREN; token.code = SEP_RPAREN; break;
            case '[': token.type = SEP_LBRACKET; token.code = SEP_LBRACKET; break;
            case ']': token.type = SEP_RBRACKET; token.code = SEP_RBRACKET; break;
            case '{': token.type = SEP_LBRACE; token.code = SEP_LBRACE; break;
            case '}': token.type = SEP_RBRACE; token.code = SEP_RBRACE; break;
            case ';': token.type = SEP_SEMICOLON; token.code = SEP_SEMICOLON; break;
            case ',': token.type = SEP_COMMA; token.code = SEP_COMMA; break;
            case '.': token.type = SEP_DOT; token.code = SEP_DOT; break;
            default:
                token.code = -1; // 错误
        }
        
        return token;
    }
    
    // 获取下一个Token - 修复版本
    Token getNextToken() {
        // 循环跳过空白字符和注释
        while (true) {
            skipWhitespace();
            
            // 尝试跳过注释，如果跳过了注释，继续循环
            if (skipComment()) {
                continue;
            }
            
            // 没有更多空白字符和注释，退出循环
            break;
        }
        
        if (pos >= input.size()) {
            return {"", ID, -1}; // EOF
        }
        
        char c = input[pos];
        
        // 数字
        if (isdigit(c)) {
            return scanNumber();
        }
        
        // 标识符或关键字
        if (isalpha(c) || c == '_') {
            return scanIdentifier();
        }
        
        // 运算符和符号
        return scanOperator();
    }
    
    // 分析所有Token
    vector<Token> tokenize() {
        vector<Token> tokens;
        
        while (pos < input.size()) {
            Token token = getNextToken();
            if (token.code == -1 && pos >= input.size()) {
                break; // EOF
            }
            if (token.code == -1) {
                cout << "Error at position " << pos << ": " << input[pos] << endl;
                pos++;
                continue;
            }
            tokens.push_back(token);
        }
        
        return tokens;
    }
};

// Token类型名称映射
string getTokenTypeName(TokenType type) {
    static map<TokenType, string> typeNames = {
        {KW_INT, "KEYWORD"}, {KW_FLOAT, "KEYWORD"}, {KW_REAL, "KEYWORD"},
        {KW_VOID, "KEYWORD"}, {KW_IF, "KEYWORD"}, {KW_ELSE, "KEYWORD"},
        {KW_WHILE, "KEYWORD"}, {KW_FOR, "KEYWORD"}, {KW_RETURN, "KEYWORD"},
        {OP_PLUS, "OPERATOR"}, {OP_MINUS, "OPERATOR"}, {OP_MULT, "OPERATOR"},
        {OP_DIV, "OPERATOR"}, {OP_MOD, "OPERATOR"}, {OP_ASSIGN, "OPERATOR"},
        {OP_EQ, "OPERATOR"}, {OP_NE, "OPERATOR"}, {OP_LT, "OPERATOR"},
        {OP_LE, "OPERATOR"}, {OP_GT, "OPERATOR"}, {OP_GE, "OPERATOR"},
        {OP_INC, "OPERATOR"}, {OP_DEC, "OPERATOR"}, {OP_NOT, "OPERATOR"},
        {OP_POWER, "OPERATOR"},
        {SEP_LPAREN, "SEPARATOR"}, {SEP_RPAREN, "SEPARATOR"},
        {SEP_LBRACKET, "SEPARATOR"}, {SEP_RBRACKET, "SEPARATOR"},
        {SEP_LBRACE, "SEPARATOR"}, {SEP_RBRACE, "SEPARATOR"},
        {SEP_SEMICOLON, "SEPARATOR"}, {SEP_COMMA, "SEPARATOR"},
        {SEP_DOT, "SEPARATOR"},
        {ID, "IDENTIFIER"}, {NUM, "NUMBER"}
    };
    
    return typeNames[type];
}

int main() {
    string line;
    string allInput;
    cout << "Enter code (Ctrl+Z/D to end):" << endl;
    
    // 收集所有输入
    while (getline(cin, line)) {
        if (!allInput.empty()) {
            allInput += "\n";  // 在行之间添加换行符
        }
        allInput += line;
    }
    
    // 输入结束后，统一进行词法分析和输出
    if (!allInput.empty()) {
        Lexer lexer(allInput);
        vector<Token> tokens = lexer.tokenize();
        
        for (const Token& token : tokens) {
            cout << token.lexeme << " : " << token.code 
                 << " (" << getTokenTypeName(token.type) << ")" << endl;
        }
    }
    
    return 0;
}