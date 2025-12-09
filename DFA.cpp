#include "DFA.h"
#include <queue>
#include <iostream>
#include <cctype>
extern vector<string> names;

extern stack<int> st;//辅助转换
extern set<int>accepted;//记录终态
extern string buffer;//存正则表达式
extern map<int,int> acceptCodeMap; // NFA接受态到编码基（100/101/200）
//dfa处理前的清
void DFA::clear() {
    dfa_size = 0;
    dfa.clear();
}

//求空闭包数量
void DFA::epsilon_closure(int state, set<int>& epsi, vector<nst> &nfa) {
    for (unsigned int i = 0; i < nfa[state].a["eps"].size(); i++) {
        if (epsi.count(nfa[state].a["eps"][i]) == 0) {
            epsi.insert(nfa[state].a["eps"][i]);
            epsilon_closure(nfa[state].a["eps"][i], epsi, nfa);
        }
    }
}


//nfa转dfa
void DFA::nfa_to_dfa(vector<nst> &nfa) {
    int nfaSize = nfa.size();
    dfa.resize(nfaSize);
    int final_status = st.top();
    st.pop();
    int start_status = st.top();
    st.pop();
    nfa[final_status].out = 1;

    map<set<int>, int> flag; //标记出现过的状态,
    map<set<int>, int> index; //所有等价的status对应下标
    set<int>startSet; //起始状态集
    //求所有状态的空闭包，即所有状态的等价状态
    set<int>* eps = new set<int>[nfaSize];
    for (int i = 0; i < nfaSize; i++)
        epsilon_closure(i, eps[i], nfa);

    startSet = eps[start_status];
    startSet.insert(start_status); //至少得有一个起始状态

    //起始状态对应的数字
    index[startSet] = 0;
    sameStatus[0] = startSet;//记录等价起点的所有状态
    // 初始包含起始状态
    dfa_size = 1;
    start_index = 0;

    queue<set<int>> q;
    q.push(startSet);
    while (!q.empty()) {
        auto ss = q.front();
        q.pop();
        flag[ss]++;
        if (flag[ss] > 1)
            continue;
        //接下来算第一行：起始状态先读入一个对应字符再做eps闭包
        for (auto name : names) { //第几个字符
            set<int> status; //记录状态

            for (auto st : ss) { //对应起点
                //st通过一个i边能到达的集合
                for (auto j : nfa[st].a[name]) {
                    status.insert(j); //首先是自己
                    for (auto k : eps[j]) //加上j的空闭包
                        status.insert(k);
                }
            }
            if (status.size() && flag[status] == 0)
            {
                q.push(status);
            }


            if (status.size())
            {
                if (!index.count(status))
                {
                    index[status] = dfa_size;
                    sameStatus[dfa_size] = status;
                    dfa_size++;
                }
                dfa[index[ss]].a[name].push_back(index[status]);
                // 标记接受态与编码标签：若子集中包含某个正则的接受态
                for (auto j : status) {
                    if (accepted.count(j)) {
                        dfa[index[status]].end = 1;
                    }
                    auto itc = acceptCodeMap.find(j);
                    if (itc != acceptCodeMap.end()) {
                        if (find(dfa[index[status]].labels.begin(), dfa[index[status]].labels.end(), itc->second) == dfa[index[status]].labels.end())
                            dfa[index[status]].labels.push_back(itc->second);
                        dfa[index[status]].end = 1;
                    }
                }
            }
        }
    }
}

// 最小化DFA：使用分区细化算法（Hopcroft思想）对当前DFA进行状态等价合并
// 输入：当前对象内的DFA（dfa、dfa_size、names）
// 输出：以字符串形式打印最小化后的DFA，状态说明为合并后状态包含的NFA原始状态集合
QString DFA::minDFA()
{
    // 1. 收集可达状态（从0号起始状态）
    vector<int> reachable(dfa_size, 0);
    queue<int> rq;
    rq.push(0);
    reachable[0] = 1;
    while (!rq.empty()) {
        int u = rq.front(); rq.pop();
        for (auto &sym : names) {
            if (!dfa[u].a[sym].empty()) {
                int v = dfa[u].a[sym][0];
                if (v >= 0 && v < dfa_size && !reachable[v]) {
                    reachable[v] = 1;
                    rq.push(v);
                }
            }
        }
    }

    // 2. 初始分区：接受态与非接受态（仅考虑可达状态）
    vector<int> stateBlock(dfa_size, -1);
    vector<vector<int>> blocks;
    vector<int> accBlock, nonAccBlock;
    for (int i = 0; i < dfa_size; ++i) {
        if (!reachable[i]) continue;
        if (dfa[i].end) accBlock.push_back(i); else nonAccBlock.push_back(i);
    }
    if (!nonAccBlock.empty()) { blocks.push_back(nonAccBlock); for (int s : nonAccBlock) stateBlock[s] = 0; }
    if (!accBlock.empty()) { blocks.push_back(accBlock); for (int s : accBlock) stateBlock[s] = (int)blocks.size()-1; }

    // 若所有状态都不可达或只有一个分区，直接打印现有结构
    if (blocks.empty()) {
        // 构造空打印
        QString result = "";
        result += buffer + ":\n";
        result += "DFA最小化: \n";
        result += "K = {};\nΣ = {};\nI = {};\nZ = {};\n";
        return result;
    }

    // 3. 分区细化：按字母表对转移的目标分区进行区分
    bool changed = true;
    while (changed) {
        changed = false;
        vector<vector<int>> newBlocks;
        for (auto &blk : blocks) {
            if (blk.size() <= 1) { newBlocks.push_back(blk); continue; }

            // 将当前分区内的状态按“转移签名”分组
            map<vector<int>, vector<int>> groups;
            for (int s : blk) {
                vector<int> signature;
                signature.reserve(names.size());
                for (auto &sym : names) {
                    int t = -1;
                    if (!dfa[s].a[sym].empty()) t = stateBlock[dfa[s].a[sym][0]];
                    signature.push_back(t);
                }
                groups[signature].push_back(s);
            }

            // 如果分成多个组则发生细化
            if (groups.size() > 1) {
                changed = true;
                for (auto &kv : groups) newBlocks.push_back(kv.second);
            } else {
                newBlocks.push_back(blk);
            }
        }
        // 更新state->block映射
        blocks = newBlocks;
        for (int bi = 0; bi < (int)blocks.size(); ++bi) {
            for (int s : blocks[bi]) stateBlock[s] = bi;
        }
    }

    // 4. 构造最小化后的DFA结构
    int mSize = (int)blocks.size();
    vector<dst> mdfa(mSize);
    // 最小化后的状态说明：合并后的状态包含的所有NFA原状态（合并原dfa状态的sameStatus）
    map<int, set<int>> mSameStatus;
    for (int bi = 0; bi < mSize; ++bi) {
        set<int> unionSet;
        for (int s : blocks[bi]) {
            // 合并原DFA状态对应的NFA集合
            for (int nfaState : sameStatus[s]) unionSet.insert(nfaState);
        }
        mSameStatus[bi] = unionSet;

        // 合并后的接受标记与标签集合
        for (int s : blocks[bi]) if (dfa[s].end) { mdfa[bi].end = 1; break; }
        {
            set<int> labelSet;
            for (int s : blocks[bi]) {
                for (int lb : dfa[s].labels) labelSet.insert(lb);
            }
            for (int lb : labelSet) mdfa[bi].labels.push_back(lb);
        }

        // 转移：对分区内所有状态聚合每个符号的目标分区（等价类应一致，聚合更稳健）
        for (auto &sym : names) {
            int targetBi = -1;
            for (int s : blocks[bi]) {
                if (!dfa[s].a[sym].empty()) {
                    int to = dfa[s].a[sym][0];
                    int bi_to = stateBlock[to];
                    if (bi_to != -1) { targetBi = bi_to; break; }
                }
            }
            if (targetBi != -1) mdfa[bi].a[sym].push_back(targetBi);
        }
    }

    // 5. 打印最小化DFA
    QString result = "";
    result += buffer + ":\n";
    result += "DFA最小化: \n";
    result += "K = {";
    for (int i = 0; i < mSize; i++) {
        if (i) result += ", ";
        result += QString::number(i);
    }
    result += "};\n";

    // Σ
    result += "Σ = {";
    auto it = names.begin();
    result += *it, it++;
    for (; it != names.end(); it++) result += ", " + *it;
    result += "};\n";

    // 状态说明
    for (int i = 0; i < mSize; i++) {
        result += QString::number(i) + " : {";
        int f = 1;
        for (int j : mSameStatus[i]) {
            if (f) result += QString::number(j), f = 0;
            else result += ", " + QString::number(j);
        }
        result += "}\n";
    }

    // 转移
    set<int> in;
    int fst = 0;
    for (int i = 0; i < mSize; i++) {
        for (int j = 0; j < (int)names.size(); j++) {
            if (!mdfa[i].a[names[j]].empty()) {
                for (auto k : mdfa[i].a[names[j]]) {
                    in.insert(k);
                    if (fst) result += ", "; else fst = 1;
                    result += "f(" + QString::number(i) + ", " + QString::fromUtf8(names[j]) + ") = " + QString::number(k);
                }
            }
        }
    }
    result += ";\n";

    // 起点：无入度或包含原始起始状态0所在分区
    int startBi = stateBlock[0];
    result += "I = {" + QString::number(startBi) + "};\n";

    // 终态
    result += "Z = {";
    vector<int> finals;
    for (int i = 0; i < mSize; i++) if (mdfa[i].end) finals.push_back(i);
    for (int i = 0; i < (int)finals.size(); i++) {
        if (i) result += ", ";
        result += QString::number(finals[i]);
    }
    result += "};\n";

    // 更新到对象以便后续需要（可选）
    dfa = mdfa;
    dfa_size = mSize;
    sameStatus = mSameStatus;

    return result;
}
//打印dfa
QString DFA::print_dfa() {
    QString result = "";
    result += buffer + ":\n";
    result += "NFA 转 DFA: \n";
    result +=  "K = {";
    for (int i = 0; i < dfa_size; i++) {
        if (i)  result +=  ", ";
        result +=  QString::number(i);
    }
    result +=  "};\n";

    //打印状态
    result +=  "Σ = {";
    auto it = names.begin();
    result +=  *it, it++;
    for (; it != names.end(); it++) {
        result +=  ", " + *it;
    }
    result += "};\n";
    //打印状态说明：
    for (int i = 0; i < dfa_size; i++) {
        result += QString::number(i) +  " : {";
        int f = 1;
        for (int j : sameStatus[i])
        {
            if (f)
                result += QString::number(j), f = 0;
            else
                result += ", " + QString::number(j) ;
        }
        result += "}\n" ;

    }

    //打印状态转移
    set<int> in; //记录有入度的点
    int fst = 0; //是否为第一次输出
    for (int i = 0; i < dfa_size; i++) {
        for (int j = 0; j < (int)names.size(); j++) {
            if (dfa[i].a[names[j]].size() != 0)
            {
                for (auto k : dfa[i].a[names[j]]) {
                    in.insert(k);
                    if (fst)
                        result += ", ";
                    else
                        fst = 1;
                    result +=  "f(" + QString::number(i) + ", " + QString::fromUtf8(names[j]) + ") = " + QString::number(k);
                }
            }

        }
    }
    result += ";\n";

    //打印起点
    result += "I = {" + QString::number(start_index) + "};\n";

    //打印终态
    result += "Z = {";
    vector<int> final;
    for (int i = 0; i < dfa_size; i++) {
        if (dfa[i].end)
            final.push_back(i);
    }
    for (int i = 0; i < (int)final.size(); i++) {
        if (i)  result += ", ";
        result += QString::number(final[i]);
    }
    result +=  "};\n";

    return result;
}

// 字符到字母表符号的映射（支持两字符组合）
// 规则：优先匹配复合符号（"<="、"<<"），其次匹配单字符运算符；字母→"letter"，数字→"digit"
string DFA::mapSymbol(const string& input, size_t pos, size_t& advance) {
    advance = 1;
    if (pos >= input.size()) return "";
    char c = input[pos];
    char n = (pos + 1 < input.size()) ? input[pos + 1] : '\0';

    if (isalpha(static_cast<unsigned char>(c))) return "letter";
    if (isdigit(static_cast<unsigned char>(c))) return "digit";

    if (c == '<') {
        if (n == '=') { advance = 2; return "<="; }
        if (n == '<') { advance = 2; return "<<"; }
        return "<";
    }
    if (c == '+') return "\\+";
    if (c == '-') return "-";
    if (c == '*') return "\\*";
    if (c == '/') return "/";
    if (c == '=') return "=";
    if (c == '>') return ">";
    if (c == '&') return "&";
    if (c == '|') return "\\|";
    return string(1, c);
}

// 一次优先级扫描：
// 策略：
// - 若当前位置为'+'或'-'且后续紧接digit，则作为数字（包含符号）连续读入所有digit；
// - 否则若是'+'或'-'则作为单字符运算符返回；
// - '<'优先匹配"<="、"<<"，否则匹配"<"；
// - letter开头读入连续的letter/digit作为标识符；
// - digit开头读入连续digit作为数字；
DFA::Token DFA::lexOneWithPriority(const string& input, size_t& pos) {
    Token tk{ "", "" };
    size_t n = input.size();
    while (pos < n && isspace(static_cast<unsigned char>(input[pos]))) pos++;
    if (pos >= n) return tk;

    char c = input[pos];
    char nx = (pos + 1 < n) ? input[pos + 1] : '\0';

    auto feed = [&](const string& sym) {
        int cur = start_index;
        if (!dfa[cur].a[sym].empty()) cur = dfa[cur].a[sym][0];
        return cur;
    };

    if ((c == '+' || c == '-') && isdigit(static_cast<unsigned char>(nx))) {
        tk.type = "NUMBER";
        tk.lexeme.push_back(c);
        pos++;
        while (pos < n && isdigit(static_cast<unsigned char>(input[pos]))) {
            tk.lexeme.push_back(input[pos]);
            pos++;
        }
        return tk;
    }

    if (c == '+' || c == '-') {
        tk.type = "OPERATOR";
        tk.lexeme.push_back(c);
        pos++;
        return tk;
    }

    if (c == '<') {
        if (nx == '=') { tk.type = "OPERATOR"; tk.lexeme = "<="; pos += 2; return tk; }
        if (nx == '<') { tk.type = "OPERATOR"; tk.lexeme = "<<"; pos += 2; return tk; }
        tk.type = "OPERATOR"; tk.lexeme = "<"; pos += 1; return tk;
    }

    if (isalpha(static_cast<unsigned char>(c))) {
        tk.type = "IDENT";
        tk.lexeme.push_back(c);
        pos++;
        while (pos < n) {
            char cc = input[pos];
            if (isalpha(static_cast<unsigned char>(cc)) || isdigit(static_cast<unsigned char>(cc))) {
                tk.lexeme.push_back(cc);
                pos++;
            } else break;
        }
        return tk;
    }

    if (isdigit(static_cast<unsigned char>(c))) {
        tk.type = "NUMBER";
        tk.lexeme.push_back(c);
        pos++;
        while (pos < n && isdigit(static_cast<unsigned char>(input[pos]))) {
            tk.lexeme.push_back(input[pos]);
            pos++;
        }
        return tk;
    }

    // 其他单字符运算符
    if (c == '*' || c == '/' || c == '=' || c == '>' || c == '&' || c == '|') {
        tk.type = "OPERATOR";
        tk.lexeme.push_back(c);
        pos++;
        return tk;
    }

    // 默认：当作单字符返回
    tk.type = "UNKNOWN";
    tk.lexeme.push_back(c);
    pos++;
    return tk;
}

vector<DFA::Token> DFA::lexAllWithPriority(const string& input) {
    vector<Token> out;
    size_t pos = 0;
    while (pos < input.size()) {
        Token t = lexOneWithPriority(input, pos);
        if (!t.type.empty()) out.push_back(t);
    }
    return out;
}

// 生成双层switch的匹配源程序（C++）
// 生成双层switch的匹配源程序（C++）
QString DFA::generate_switch_case_source() {
    QString out;
    out += "// 自动生成：基于最小化DFA的词法匹配代码\n";
    out += "#include <iostream>\n#include <string>\n#include <vector>\n#include <cctype>\n\n";
    out += "using namespace std;\n\n";

    // 生成 Token 分类函数
    out += "int classifyToken(const std::string& token) {\n";
    out += QString("    int currentState = %1;\n").arg(start_index); // 通常是 0
    out += "    for (size_t i = 0; i < token.size(); ++i) {\n";
    out += "        char c = token[i];\n";
    out += "        switch (currentState) {\n";

    // 遍历所有 DFA 状态生成第一层 Switch
    for (int i = 0; i < dfa_size; ++i) {
        // 如果该状态没有任何出边，不需要生成 case
        bool hasTransition = false;
        for(auto& kv : dfa[i].a) if(!kv.second.empty()) hasTransition = true;

        if (!hasTransition) continue;

        out += QString("        case %1:\n").arg(i);
        out += "            switch (c) {\n";

        // 记录已经处理过的具体字符，防止与 letter/digit 冲突
        // 比如 'e' 有专门的转移，那么处理 "letter" 时就要跳过 'e'
        set<char> handledChars;

        // 1. 先处理具体的单字符转移 (如 '+', '*', 'i' 等)
        for (const auto& kv : dfa[i].a) {
            string symbol = kv.first;
            int targetState = kv.second.empty() ? -1 : kv.second[0];
            if (targetState == -1) continue;

            // 跳过抽象符号，稍后处理
            if (symbol == "letter" || symbol == "digit") continue;

            // 处理转义字符
            char ch = 0;
            if (symbol.size() == 1) ch = symbol[0];
            else if (symbol == "\\+") ch = '+';
            else if (symbol == "\\*") ch = '*';
            else if (symbol == "\\|") ch = '|';
            else if (symbol == "\\?") ch = '?';
            // 注意：你原来的代码把 "<=" 等复合符号也放在 names 里。
            // 在 NFA->DFA 过程中，如果是标准构造，复合符号应该已被拆解为序列。
            // 如果你的 DFA 边上仍然保留 "<=" 这种字符串，说明 NFA 构建或 Split 逻辑有特殊处理。
            // 正常 DFA 边上应该只有单字符。此处假设边上是单字符。
            // 如果你的 names 里包含 "<="，请确保 regexp_to_postfix 里将其拆解，或者 DFA 转移里只存单字符。
            // 根据你的 postfix_to_nfa 逻辑，如果 names 里有 "<="，它被当作由两个字符组成的路径处理还是单原子？
            // 看逻辑是单原子 signal("<<")。这在 DFA 只有一步跳转。
            // 如果是这样，输入字符串必须把 "<<" 当作一个 char 处理，这在实际 C++ char 中是不可能的。
            // **修正假设**：生成的 C++ 代码是读 char 的，所以 DFA 必须是单字符转移。
            // 如果你的 DFA 图里有 "<=" 这种边，生成的 switch (char) 是无法匹配的。
            // 现在的修复基于假设：DFA 边上的 symbol 最终对应 ASCII 字符。

            if (ch != 0) {
                out += QString("            case '%1': currentState = %2; break;\n").arg(QChar(ch)).arg(targetState);
                handledChars.insert(ch);
            }
        }

        // 2. 处理 "digit" 集合 (0-9)
        if (dfa[i].a.count("digit") && !dfa[i].a["digit"].empty()) {
            int target = dfa[i].a["digit"][0];
            out += "            case '0': case '1': case '2': case '3': case '4': \n";
            out += "            case '5': case '6': case '7': case '8': case '9': \n";
            out += QString("                currentState = %1; break;\n").arg(target);
            // 标记数字已被处理
            for(char c='0'; c<='9'; ++c) handledChars.insert(c);
        }

        // 3. 处理 "letter" 集合 (a-z, A-Z)
        if (dfa[i].a.count("letter") && !dfa[i].a["letter"].empty()) {
            int target = dfa[i].a["letter"][0];
            bool first = true;
            int count = 0;
            // 遍历所有字母，只有未被 specific transition 处理过的才生成 case
            string allLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            for (char c : allLetters) {
                if (handledChars.find(c) == handledChars.end()) {
                    if (count > 0 && count % 10 == 0) out += "\n            ";
                    out += QString("case '%1': ").arg(QChar(c));
                    count++;
                }
            }
            if (count > 0) {
                out += QString("currentState = %1; break;\n").arg(target);
            }
        }

        out += "            default: return -1; // 无效转移\n";
        out += "            }\n"; // end switch(c)
        out += "            break;\n";
    }

    out += "        default: return -1; // 无效状态\n";
    out += "        }\n"; // end switch(currentState)
    out += "    }\n\n"; // end loop

    // 根据最终状态返回编码
    out += "    // 根据终态返回对应的 Token 编码\n";
    out += "    switch (currentState) {\n";
    for (int i = 0; i < dfa_size; ++i) {
        if (dfa[i].end) {
            out += QString("    case %1:\n").arg(i);

            // 优先级判定策略：
            // 这里的 labels 包含了该状态代表的所有 NFA 终态的编码。
            // 比如状态 X 既是 ID (100) 又是 IF (300)。
            // 应该返回特定的关键字编码，而不是通用的 ID 编码。
            // 假设编码规则：关键字 > 200/300, ID = 100/101, Op = 200...
            // 简单逻辑：取最大值 (假设关键字编码较大) 或者 特殊值优先。
            // 此处代码假设 labels 里存的就是 int 编码。

            if (dfa[i].labels.empty()) {
                out += "        return 0; // 接受但无编码\n";
            } else {
                // 查找优先级最高的编码
                // 逻辑：如果有 >= 300 (Keywords) 返回最大值
                // 否则如果有 >= 200 (Operators) 返回对应值
                // 否则返回 100 (ID)
                // 这是一个启发式策略，具体需根据你的编码表调整
                out += "        { int code = -1; int maxCode = -1; \n";
                out += "          std::vector<int> labs = {";
                for(size_t k=0; k<dfa[i].labels.size(); ++k)
                    out += QString::number(dfa[i].labels[k]) + (k==dfa[i].labels.size()-1?"":",");
                out += "};\n";
                out += "          for(int c : labs) { if(c > maxCode) maxCode = c; }\n";
                out += "          return maxCode;\n";
                out += "        }\n";
            }
        }
    }
    out += "    default: return -1; // 非接受态\n";
    out += "    }\n";
    out += "}\n\n";

    // 主函数保持类似
    out += "int main() {\n";
    out += "    std::string line;\n";
    out += "    std::cout << \"Enter code (Ctrl+Z/D to end):\" << std::endl;\n";
    out += "    while (std::getline(std::cin, line)) {\n";
    out += "        size_t i = 0;\n";
    out += "        while (i < line.size()) {\n";
    out += "            while (i < line.size() && std::isspace(line[i])) ++i;\n";
    out += "            if (i >= line.size()) break;\n";
    out += "            // 贪婪匹配：尝试找到最长的匹配子串\n";
    out += "            int bestCode = -1;\n";
    out += "            size_t bestEnd = i;\n";
    out += "            std::string sub;\n";
    out += "            for (size_t j = i + 1; j <= line.size(); ++j) {\n";
    out += "                sub = line.substr(i, j - i);\n";
    out += "                int code = classifyToken(sub);\n";
    out += "                if (code != -1) {\n";
    out += "                    bestCode = code;\n";
    out += "                    bestEnd = j;\n";
    out += "                } else {\n";
    out += "                    // 如果当前前缀已经不能匹配，且DFA死胡同(根据你的实现classifyToken如果是前缀不匹配会直接-1)，可以break优化\n";
    out += "                    // 但你的classifyToken只要没走到死路循环完都算，这里简单处理不break\n";
    out += "                }\n";
    out += "            }\n";
    out += "            if (bestCode != -1) {\n";
    out += "                std::cout << line.substr(i, bestEnd - i) << \" : \" << bestCode << std::endl;\n";
    out += "                i = bestEnd;\n";
    out += "            } else {\n";
    out += "                std::cout << \"Error at: \" << line[i] << std::endl;\n";
    out += "                ++i;\n";
    out += "            }\n";
    out += "        }\n";
    out += "    }\n";
    out += "    return 0;\n";
    out += "}\n";

    return out;
}
