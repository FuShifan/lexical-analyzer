#ifndef NFA_H
#define NFA_H
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <QString>
class Dialog;
using namespace std;

//产生式定义
struct production {
    int start;//起点
    string status;//状态
    int accept;//终点
};
//nfa dfa结构定义
struct nst {
    map<string, vector<int>> a;
    bool out = 0; //=1为有出度
};
//NFA类
class NFA {
public:
    vector<nst> nfa;

    int nfa_size = 0;
    nst init_nfa_state;//初始状态




    //判断是否有非法输入
    bool check(string postfix);
    //每组处理前的清空
    void clear();

    /***************************** regex to nfa ****************************/

    // 辅助函数：获取最长状态名长度
    size_t get_max_state_len(const vector<string>& names);

    //对正则表达式预处理
    string insert_concat(string regexp);

    //处理命名
    void namer(string name);
    //处理符号
    void signal(string ch);

    // 处理正闭包+（A+ = A·A*）
    // 构造规则：复用闭包，连接A和A*
    void plus_nfa();

    // 可选操作?（A? = A | ε）
    // 构造规则：新增起始状态s，接受状态e
    // s --ε--> A.start，s --ε--> e
    // A.accept --ε--> e
    void optional_nfa();

    //处理'|'
    void union_();

    //处理'.'
    void concatenation();
    //处理'*'
    void kleene_star() ;

    //后缀转nfa
    void postfix_to_nfa(string postfix) ;

    //出入栈优先级
    int priority(char c) ;

    //正则式转后缀表达式
    string regexp_to_postfix(string regexp) ;

    //输出nfa
    QString print_nfa() ;
    //处理函数
    void processed(Dialog *);

    // 合并所有正则表达式生成的子图为统一的NFA图（Thompson构造）
    // 输入：每条正则对应的起始状态和接受状态对
    // 输出：在现有nfa末尾新增统一起始状态S与统一接受状态E，并建立ε转移：S→每个子图start，子图accept→E
    // 兼容：保持原有接口，最终将(S,E)压栈以供后续DFA转换使用
    void unify_all_regex(const vector<pair<int,int>>& start_accept_pairs);
};

// 接受态到编码基的映射（例如 100, 101, 200）
extern std::map<int,int> acceptCodeMap;

#endif // NFA_H
