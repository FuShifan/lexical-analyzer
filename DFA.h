#ifndef DFA_H
#define DFA_H
#include <set>
#include <vector>
#include "NFA.h"
using namespace std;

struct dst {
    map<string, vector<int>> a;
    bool end = 0; //1为可结束状态
    vector<int> labels; // 该DFA状态对应的词类编码基（如100,101,200）

};
//DFA类
class DFA {
public:
    vector<dst> dfa;
    int dfa_size = 0;
    dst init_dfa_state;
    map<int, set<int>> sameStatus;//记录所有相同状态
    int start_index = 0; //记录DFA起始状态下标

    //dfa处理前的清空
    void clear();

    //求空闭包数量
    void epsilon_closure(int state, set<int>& epsi, vector<nst> &nfa) ;

    //打印dfa
    QString print_dfa();

    //nfa转dfa
    void nfa_to_dfa(vector<nst> &nfa);

    QString minDFA();

    // 词法扫描优先级：实现“+/- 初读优先作为运算符，若后续跟digit则按数字处理”的策略
    // Token结构用于返回类型与词素
    struct Token { string type; string lexeme; };

    // 字符到字母表符号的映射（支持两字符组合，如"<="、"<<"）
    // 返回符号名（与names一致），并设置advance为消耗的字符数
    string mapSymbol(const string& input, size_t pos, size_t& advance);

    // 基于当前DFA进行一次优先级扫描，返回一个Token，并推进pos
    Token lexOneWithPriority(const string& input, size_t& pos);

    // 扫描整段文本，返回Token序列
    vector<Token> lexAllWithPriority(const string& input);

    // 生成基于当前DFA的双层switch匹配源程序（C++），用于词法分析
    QString generate_switch_case_source();
};
#endif // DFA_H
