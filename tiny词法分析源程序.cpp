#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <map>
#include <algorithm>

using namespace std;

// Token类型编码
enum TokenType {
    // 关键字 (1-10)
    TK_IF = 1,
    TK_THEN = 2,
    TK_ELSE = 3,
    TK_END = 4,
    TK_REPEAT = 5,
    TK_UNTIL = 6,
    TK_READ = 7,
    TK_WRITE = 8,
    
    // 运算符和符号 (20-40)
    TK_PLUS = 20,      // +
    TK_MINUS = 21,     // -
    TK_MULT = 22,      // *
    TK_DIV = 23,       // /
    TK_MOD = 24,       // %
    TK_POWER = 25,     // ^
    TK_ASSIGN = 26,    // :=
    TK_EQ = 27,        // =
    TK_LT = 28,        // <
    TK_GT = 29,        // >
    TK_LE = 30,        // <=
    TK_GE = 31,        // >=
    TK_NE = 32,        // <>
    TK_LPAREN = 33,    // (
    TK_RPAREN = 34,    // )
    TK_SEMI = 35,      // ;
    
    // 标识符和数字 (100+)
    TK_ID = 100,       // 标识符
    TK_NUM = 101,      // 数字
    
    // 特殊
    TK_ERROR = -1,
    TK_COMMENT = -2
};

// 关键字映射表 (不区分大小写)
map<string, TokenType> keywords = {
    {"if", TK_IF},
    {"then", TK_THEN},
    {"else", TK_ELSE},
    {"end", TK_END},
    {"repeat", TK_REPEAT},
    {"until", TK_UNTIL},
    {"read", TK_READ},
    {"write", TK_WRITE}
};

// 将字符串转为小写
string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// 检查是否是关键字
int checkKeyword(const string& token) {
    string lower = toLower(token);
    if (keywords.find(lower) != keywords.end()) {
        return keywords[lower];
    }
    return TK_ID;
}

// 获取token类型名称
string getTokenName(int type) {
    switch(type) {
        case TK_IF: return "IF";
        case TK_THEN: return "THEN";
        case TK_ELSE: return "ELSE";
        case TK_END: return "END";
        case TK_REPEAT: return "REPEAT";
        case TK_UNTIL: return "UNTIL";
        case TK_READ: return "READ";
        case TK_WRITE: return "WRITE";
        case TK_PLUS: return "PLUS";
        case TK_MINUS: return "MINUS";
        case TK_MULT: return "MULT";
        case TK_DIV: return "DIV";
        case TK_MOD: return "MOD";
        case TK_POWER: return "POWER";
        case TK_ASSIGN: return "ASSIGN";
        case TK_EQ: return "EQ";
        case TK_LT: return "LT";
        case TK_GT: return "GT";
        case TK_LE: return "LE";
        case TK_GE: return "GE";
        case TK_NE: return "NE";
        case TK_LPAREN: return "LPAREN";
        case TK_RPAREN: return "RPAREN";
        case TK_SEMI: return "SEMI";
        case TK_ID: return "ID";
        case TK_NUM: return "NUM";
        default: return "UNKNOWN";
    }
}

class Lexer {
private:
    string input;
    size_t pos;
    
public:
    Lexer(const string& code) : input(code), pos(0) {}
    
    // 跳过空白字符
    void skipWhitespace() {
        while (pos < input.size() && isspace(input[pos])) {
            pos++;
        }
    }
    
    // 跳过注释 { ... }
    bool skipComment() {
        if (pos < input.size() && input[pos] == '{') {
            pos++;
            while (pos < input.size() && input[pos] != '}') {
                pos++;
            }
            if (pos < input.size() && input[pos] == '}') {
                pos++;
                return true;
            }
            // 未闭合的注释
            cout << "Warning: Unclosed comment" << endl;
            return true;
        }
        return false;
    }
    
    // 读取数字
    pair<string, int> readNumber() {
        string num;
        while (pos < input.size() && isdigit(input[pos])) {
            num += input[pos++];
        }
        return {num, TK_NUM};
    }
    
    // 读取标识符或关键字
    pair<string, int> readIdentifier() {
        string id;
        while (pos < input.size() && (isalnum(input[pos]) || input[pos] == '_')) {
            id += input[pos++];
        }
        int type = checkKeyword(id);
        return {id, type};
    }
    
    // 获取下一个token
    pair<string, int> getNextToken() {
        skipWhitespace();
        
        // 跳过注释
        while (skipComment()) {
            skipWhitespace();
        }
        
        if (pos >= input.size()) {
            return {"", TK_ERROR};
        }
        
        char current = input[pos];
        
        // 数字
        if (isdigit(current)) {
            return readNumber();
        }
        
        // 标识符或关键字
        if (isalpha(current)) {
            return readIdentifier();
        }
        
        // 运算符和符号
        switch (current) {
            case '+':
                pos++;
                return {"+", TK_PLUS};
            case '-':
                pos++;
                return {"-", TK_MINUS};
            case '*':
                pos++;
                return {"*", TK_MULT};
            case '/':
                pos++;
                return {"/", TK_DIV};
            case '%':
                pos++;
                return {"%", TK_MOD};
            case '^':
                pos++;
                return {"^", TK_POWER};
            case '(':
                pos++;
                return {"(", TK_LPAREN};
            case ')':
                pos++;
                return {")", TK_RPAREN};
            case ';':
                pos++;
                return {";", TK_SEMI};
            case '=':
                pos++;
                return {"=", TK_EQ};
            case '<':
                pos++;
                if (pos < input.size()) {
                    if (input[pos] == '=') {
                        pos++;
                        return {"<=", TK_LE};
                    } else if (input[pos] == '>') {
                        pos++;
                        return {"<>", TK_NE};
                    }
                }
                return {"<", TK_LT};
            case '>':
                pos++;
                if (pos < input.size() && input[pos] == '=') {
                    pos++;
                    return {">=", TK_GE};
                }
                return {">", TK_GT};
            case ':':
                pos++;
                if (pos < input.size() && input[pos] == '=') {
                    pos++;
                    return {":=", TK_ASSIGN};
                }
                // 单独的 : 不是有效token
                return {":", TK_ERROR};
            default:
                pos++;
                return {string(1, current), TK_ERROR};
        }
    }
    
    // 分析所有token
    void tokenize() {
        while (pos < input.size()) {
            auto [lexeme, type] = getNextToken();
            if (lexeme.empty()) break;
            
            if (type == TK_ERROR) {
                cout << "Error: Invalid token '" << lexeme << "'" << endl;
            } else {
                cout << lexeme << " : " << type << " (" << getTokenName(type) << ")" << endl;
            }
        }
    }
};

int main() {
    cout << "Enter TINY code (Ctrl+Z on Windows or Ctrl+D on Unix to end):" << endl;
    cout << "-----------------------------------------------" << endl;
    
    string line, code;
    while (getline(cin, line)) {
        code += line + "\n";
    }
    
    cout << "\n=============== TOKEN ANALYSIS ===============" << endl;
    Lexer lexer(code);
    lexer.tokenize();
    cout << "=============================================" << endl;
    
    return 0;
}