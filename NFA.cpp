#include "NFA.h"
#include "DFA.h"
#include "dialog.h"
#include "qdir.h"
#include "ui_dialog.h"
#include <QTableWidget>
#include <QMessageBox>
#include <algorithm>
#include <iostream>

vector<string> names;//存储命名
QString fileName;
stack<int> st;//辅助转换
set<int>accepted;//记录终态
string buffer;//存正则表达式
map<int,int> acceptCodeMap; // 每条正则的接受态到编码基
//判断是否有非法输入
bool NFA::check(string postfix) {
    for (unsigned int i = 0; i < postfix.size(); i++) {
        char ch = postfix[i];
        if (ch <= 'z' && ch >= 'a') continue;
        else if (ch == '*')    continue;
        else if (ch == '.')    continue;
        else if (ch == '|')    continue;
        else if (ch == '+')    continue;
        else if (ch == '?')     continue;
        else if (ch == '\\')    continue;

        cout << "输入为非法字符！读入只能是: 小写英文字母, |, *, ()" << endl;
        return false; //检测到非法字符
    }
    return true;
}

//每组处理前的清空
void NFA::clear() {
    nfa_size = 0;
    while (!st.empty())
        st.pop();
    nfa.clear();
    accepted.clear();
}
void init()
{
    names.push_back("\\+");
    names.push_back("-");
    names.push_back("\\*");
    names.push_back("/");
    names.push_back("=");
    names.push_back("<");
    names.push_back(">");
    names.push_back("&");
    names.push_back("\\|");
    // 复合运算符支持：将"<="和"<<"作为单独词法符号，保证最长匹配
    names.push_back("<=");
    names.push_back("<<");

}
// 辅助函数：获取最长状态名长度
size_t NFA::get_max_state_len(const vector<string>& names) {
    if (names.empty()) return 0;
    return max_element(names.begin(), names.end(),
                       [](const string& a, const string& b) { return a.size() < b.size(); }
                       )->size();
}

//对正则表达式预处理
// 对正则表达式预处理，插入状态名连接符
string NFA::insert_concat(string regexp) {
    size_t i = 0;
    string ret;
    size_t n = regexp.size();
    size_t max_len = get_max_state_len(names);

    while (i < n) {
        // 1. 优先匹配最长状态名（从长到短尝试）
        size_t matched_len = 0;
        for (size_t len = min(max_len, n - i); len > 0; --len) {
            string candidate = regexp.substr(i, len);
            if (find(names.begin(), names.end(), candidate) != names.end()) {
                matched_len = len;
                break;
            }
        }

        if (matched_len > 0) { // 状态名匹配成功
            ret += regexp.substr(i, matched_len);
            i += matched_len;

            // 2. 状态名后处理：非特殊符号需加连接符（如digit→digit → digit.digit）
            if (i < n) {
                char next_char = regexp[i];

                if (next_char != ')' && next_char != '|' && next_char != '*' && next_char != '+' && next_char != '?' && next_char != ' ') {
                    ret += '.'; // 非特殊符号直接加.
                }
            }
        }
        else { // 处理普通字符
            ret += regexp[i]; // 保留原字符
            char current_char = regexp[i++];

            // 3. 普通字符后处理
            if (i < n) {
                char next_char = regexp[i];

                if (current_char != ' ' && next_char != ' ' && current_char != '|' && current_char != '(' && next_char != ')' && next_char != '|' && next_char != '*' && next_char != '+' && next_char != '?') {
                    ret += '.';
                }
            }
        }
    }
    return ret;
}

//处理命名
void NFA::namer(string name) {
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);
    nfa[nfa_size].a[name].push_back(nfa_size + 1);
    st.push(nfa_size);
    nfa_size++;
    st.push(nfa_size);
    nfa_size++;
}

//处理符号
void NFA::signal(string ch)
{
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);
    nfa[nfa_size].a[ch].push_back(nfa_size + 1);
    st.push(nfa_size);
    nfa_size++;
    st.push(nfa_size);
    nfa_size++;
}

// 处理正闭包+（A+ = A·A*）
// 构造规则：复用闭包，连接A和A*
void NFA::plus_nfa() {
    int accept = st.top();  // 弹出原NFA的接受状态（A的结束）
    st.pop();
    int start = st.top();   // 弹出原NFA的起始状态（A的开始）
    st.pop();

    // 新增正闭包专属的起始和接受状态（共2个新状态）
    nfa.push_back(init_nfa_state);  // 新接受状态e
    nfa.push_back(init_nfa_state);  // 新起始状态s

    int s = nfa_size++;
    int e = nfa_size++;

    // 核心连接逻辑：
    nfa[s].a["eps"].push_back(start);       // s必须经过A（禁止ε跳过）
    nfa[accept].a["eps"].push_back(start);  // 允许A多次循环（A*部分）
    nfa[accept].a["eps"].push_back(e);      // 结束在新接受状态

    st.push(s);  // 压入新起始状态
    st.push(e);  // 压入新接受状态
}

// 可选操作?（A? = A | ε）
// 构造规则：新增起始状态s，接受状态e
// s --ε--> A.start，s --ε--> e
// A.accept --ε--> e
void NFA::optional_nfa() {
    int accept = st.top();  // 弹出原NFA的接受状态（A的结束）
    st.pop();
    int start = st.top();   // 弹出原NFA的起始状态（A的开始）
    st.pop();

    // 新增可选专属的起始和接受状态（共2个新状态）
    nfa.push_back(init_nfa_state);  // 新接受状态e
    nfa.push_back(init_nfa_state);  // 新起始状态s
    int s = nfa_size++;
    int e = nfa_size++;

    // 核心连接逻辑：
    nfa[s].a["eps"].push_back(start);   // 分支1：执行A
    nfa[s].a["eps"].push_back(e);       // 分支2：ε直接跳过
    nfa[accept].a["eps"].push_back(e);  // A执行完后到终点

    st.push(s);  // 压入新起始状态
    st.push(e);  // 压入新接受状态
}

//处理'|'
void NFA::union_() {
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);

    int accept2 = st.top();
    st.pop();
    int start2 = st.top();
    st.pop();
    int accept1 = st.top();
    st.pop();
    int start1 = st.top();
    st.pop();

    nfa[nfa_size].a["eps"].push_back(start1);
    nfa[nfa_size].a["eps"].push_back(start2);
    st.push(nfa_size);
    nfa_size++;
    nfa[accept1].a["eps"].push_back(nfa_size);
    nfa[accept2].a["eps"].push_back(nfa_size);
    st.push(nfa_size);
    nfa_size++;
}

//处理'.'
void NFA::concatenation() {
    int acccept2 = st.top();
    st.pop();
    int start2 = st.top();
    st.pop();
    int accept1 = st.top();
    st.pop();
    int start1 = st.top();
    st.pop();

    nfa[accept1].a["eps"].push_back(start2);
    st.push(start1);
    st.push(acccept2);

}

//处理'*'
void NFA::kleene_star() {
    //取出前两个
    int accept = st.top();
    st.pop();
    int start = st.top();
    st.pop();

    //再加三条边
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);

    nfa[accept].a["eps"].push_back(start);
    nfa[nfa_size].a["eps"].push_back(start);
    nfa[nfa_size].a["eps"].push_back(nfa_size + 1);
    nfa[accept].a["eps"].push_back(nfa_size + 1);
    st.push(nfa_size);

    nfa_size++;
    st.push(nfa_size);

    nfa_size++;

}

//后缀转nfa
void NFA::postfix_to_nfa(string postfix) {
    for (unsigned int i = 0; i < postfix.size(); i++) {
        char ch = postfix[i];
        if (ch == ' ') continue;
        string cha;
        cha = ch;
        auto it = find(names.begin(), names.end(), cha);
        if ((ch <= 'z' && ch >= 'a') || it != names.end())
        {
            string name = "";
            size_t max_len = get_max_state_len(names);
            size_t matched_len = 0;
            for (size_t len = min(max_len, postfix.size() - i); len > 0; --len) {
                string candidate = postfix.substr(i, len);
                auto it = find(names.begin(), names.end(), candidate);
                if (it != names.end()) {
                    matched_len = len;
                    break;
                }
            }

            if (matched_len > 0) { // 匹配到状态名
                name += postfix.substr(i, matched_len);
                i += matched_len;
                i -= 1;
                namer(name);
            }
            else {
                namer(postfix.substr(i, 1));
            }

        }
        else if (ch == '"')
        {
            signal("\"");
        }
        else if (ch == '\\')
        {
            signal(postfix.substr(i, 2));
            i++;
        }
        else if (ch == '+')
            plus_nfa();
        else if (ch == '?')
            optional_nfa();
        else if (ch == '*')
            kleene_star();
        else if (ch == '.')
            concatenation();
        else if (ch == '|')
            union_();
        else {
            names.push_back(postfix.substr(i, 1));
            signal(postfix.substr(i, 1));
        }
    }
}

// 合并所有正则表达式生成的子图为统一的NFA图（Thompson构造）
// 输入：start_accept_pairs 中每个元素为一条正则的起始状态与接受状态
// 处理：在当前 nfa 后追加两个新状态 S (统一起点) 与 E (统一终点)，建立 ε 转移
//      S --eps--> 每个子图 start
//      每个子图 accept --eps--> E
// 结果：将 S、E 压入构造栈 st，保证后续 DFA 构造依赖的接口兼容
void NFA::unify_all_regex(const vector<pair<int,int>>& start_accept_pairs) {
    if (start_accept_pairs.empty()) return;

    // 新增统一起点与统一接受状态
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);
    int S = nfa_size++;
    int E = nfa_size++;

    // 建立 ε 转移
    for (const auto& pr : start_accept_pairs) {
        int start = pr.first;
        int accept = pr.second;
        nfa[S].a["eps"].push_back(start);
        nfa[accept].a["eps"].push_back(E);
    }

    // 接受态集合保留为每条正则的各自接受状态，便于DFA层辨别具体词类
    accepted.clear();
    for (const auto& pr : start_accept_pairs) {
        accepted.insert(pr.second);
    }

    // 压栈以保持与现有接口兼容（DFA构造依赖于栈顶为接受、次顶为起始）
    st.push(S);
    st.push(E);
}

//正则式转后缀表达式
string NFA::regexp_to_postfix(string regexp) {
    string postfix = "";
    stack<char> op;
    size_t i = 0;
    size_t n = regexp.size();
    size_t max_len = get_max_state_len(names);

    while (i < n) {
        char ch = regexp[i];

        if (ch == ' ') { i++; continue; }

        // 括号处理
        if (ch == '(') { op.push('('); i++; continue; }
        if (ch == ')') {
            while (!op.empty() && op.top() != '(') {
                postfix += op.top();
                op.pop();
            }
            if (!op.empty()) op.pop();
            i++; continue;
        }

        // 尝试最长匹配命名符号（包含复合符号，如"<="、"<<"、"digit"、"letter"、"\\+"等）
        size_t matched_len = 0;
        string matched_token;
        for (size_t len = min(max_len, n - i); len > 0; --len) {
            string candidate = regexp.substr(i, len);
            if (find(names.begin(), names.end(), candidate) != names.end()) {
                matched_len = len;
                matched_token = candidate;
                break;
            }
        }

        if (matched_len > 0) {
            postfix += matched_token;
            postfix += ' ';
            i += matched_len;
            continue;
        }

        // 普通字母作为原子
        if (ch >= 'a' && ch <= 'z') { postfix += ch; postfix += ' '; i++; continue; }
        if (ch == '"') { postfix += ch; postfix += ' '; i++; continue; }

        // 处理运算符 . | * + ?
        while (!op.empty() && priority(op.top()) > priority(ch)) {
            postfix += op.top();
            postfix += ' ';
            op.pop();
        }
        op.push(ch);
        i++;
    }

    while (!op.empty()) {
        postfix += op.top();
        postfix += ' ';
        op.pop();
    }
    return postfix;
}

//输出nfa
QString NFA::print_nfa() {
    QString result = "";
    result += buffer + ":\n";

    result += "正则式 转 NFA: \n";
    result += "K = {";
    for (int i = 0; i < nfa.size(); i++) {
        if (i)  result += ", ";
        result += QString::number(i);
    }
    result += "};\n";

    //打印状态
    result += "Σ = {";
    auto it = names.begin();
    result += QString::fromUtf8(*it), it++;
    for (; it != names.end(); it++) {
        result += ", " + *it;
    }
    result += "};\n";

    vector<production> answer;
    set<int> in; //记录有入度的点
    for (int i = 0; i < nfa.size(); i++) {

        for (int j = 0; j < names.size(); j++) {
            if (nfa[i].a[names[j]].size() != 0)
            {
                nfa[i].out = 1;//记录有出度
                for (auto k : nfa[i].a[names[j]])
                {
                    answer.push_back({ i, names[j], k });
                    in.insert(k);
                }

            }
        }

        if (nfa[i].a["eps"].size() != 0)
        {
            nfa[i].out = 1;//记录有出度
            for (auto j : nfa[i].a["eps"])
            {
                answer.push_back({ i, "ε", j });
                in.insert(j);

            }
        }
    }

    for (int i = 0; i < answer.size(); i++) {
        if (i)  result += ", ";
        result += "f(";
        result += QString::number(answer[i].start);
        result += ", ";
        result += QString::fromUtf8(answer[i].status);
        result += ") = ";
        result += QString::number(answer[i].accept);
    }
    result += ";\n";

    //没有入度就是起点
    for (int i = 0; i < nfa.size(); i++) {
        if (!in.count(i)) {
            result += "I = {" + QString::number(i) + "};\n";
            break;
        }
    }

    // 终态：使用构造栈的栈顶接受状态，避免误将无出度的中间状态标为终态
    result += "Z = {";
    if (!st.empty()) {
        int accept = st.top();
        accepted.clear();
        accepted.insert(accept);
        result += QString::number(accept);
    }
    result += "};\n";
    return result;
}
int NFA::priority(char c) {
    switch (c) {
    case '*':
        return 3;
    case '+':
        return 3;
    case '?':
        return 3;
    case '.':
        return 2;
    case '|':
        return 1;
    default:
        return 0;// 括号和普通字符
    }
}

//处理输入的正则表达式
// 校验逻辑：在开始转换前检查输入文本/所选文件中是否存在
// 以“_”开头的正则定义（形如“_ID100 = ...”）。若未检测到此类
// 正则定义，则给出提示并终止解析与构造流程。
void NFA::processed(Dialog *dialog)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cout << "file open error";
        return;
    }
    QTextStream in(&file);
    init();

    // 先读入所有行，进行“是否存在以_开头的正则定义”的校验
    QStringList lines;
    while (!in.atEnd()) {
        lines << in.readLine();
    }
    file.close();

    auto hasUnderscoreRegex = [&]() -> bool {
        for (const QString &line : lines) {
            const QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) continue;
            int eq = trimmed.indexOf('=');
            if (eq <= 0) continue; // 无“=”或“=”在首位
            if (trimmed.startsWith('_')) {
                // 右侧非空即认为是正则定义
                const QString rhs = trimmed.mid(eq + 1).trimmed();
                if (!rhs.isEmpty()) return true;
            }
        }
        return false;
    }();

    if (!hasUnderscoreRegex) {
        QMessageBox::warning(dialog, QObject::tr("提示"),
                             QObject::tr("未检测到以“_”开头的正则表达式，或输入中没有任何正则定义；\n请在输入或文件中添加以“_”开头的正则表达式后再进行转换。"));
        return;
    }

    // 重构：一次性读取所有正则，构造统一NFA并输出
    QString result1 = ""; // 单条正则的NFA结构（对比用）
    QString result2 = ""; // 合并后DFA结构
    QString result3 = ""; // 合并后最小化DFA结构

    // 统一构建流程的中间数据
    clear();
    DFA d;
    d.clear();
    vector<pair<int,int>> regex_pairs; // 记录每条正则的(start, accept)

    // 按行处理（两遍读取已在校验中完成，这里直接用缓存的 lines）
    for (const QString &qline : lines)
    {
        QByteArray ba = qline.toUtf8();
        buffer = ba.data();

        int equalPos = buffer.find("=");
        if (equalPos == -1)
        {
            cout << "输入格式错误";
            return;
        }

        if (buffer[0] == '_')
        {
            // 提取正则表达式
            string regexp = buffer.substr(equalPos + 1);

            // 若为并集型词法（如 _special200S），提前解析'|'-分隔的词面，加入到字母表，避免如'*'被误当作闭包导致栈错误
            {
                string nameLeft = buffer.substr(0, equalPos);
                auto trim = [&](const string& s) {
                    size_t l = 0, r = s.size();
                    while (l < r && isspace(static_cast<unsigned char>(s[l]))) ++l;
                    while (r > l && isspace(static_cast<unsigned char>(s[r-1]))) --r;
                    return s.substr(l, r - l);
                };
                string nLeft = trim(nameLeft);
                bool isUnionLex = !nLeft.empty() && nLeft.back() == 'S';
                if (isUnionLex) {
                    // 分割'|'
                    size_t pos = 0;
                    while (pos < regexp.size()) {
                        size_t bar = regexp.find('|', pos);
                        string tok = trim(regexp.substr(pos, bar == string::npos ? string::npos : bar - pos));
                        if (!tok.empty()) {
                            auto addName = [&](const string& t) {
                                if (find(names.begin(), names.end(), t) == names.end()) names.push_back(t);
                            };
                            // 针对需要转义但未转义的特殊字符进行双版本加入
                            if (tok == "*") { addName("*"); addName("\\*"); }
                            else if (tok == "+") { addName("+"); addName("\\+"); }
                            else if (tok == "|") { addName("|"); addName("\\|"); }
                            else { addName(tok); }
                        }
                        if (bar == string::npos) break; else pos = bar + 1;
                    }
                }
            }

            // 正则预处理与后缀化
            regexp = insert_concat(regexp);
            cout << "Regexp Expression: " << regexp << endl;
            string postfix = regexp_to_postfix(regexp);
            cout << "Postfix Expression: " << postfix << endl;

            // 构造该正则的NFA子图（增量追加到当前nfa）
            postfix_to_nfa(postfix);

            // 对比打印：打印该子图结构（此时栈顶即该子图的接受状态）
            result1 += print_nfa() + "\n\n";

            // 取出该子图的(start, accept)对并记录（保持状态编号唯一性与连续性）
            int accept = st.top(); st.pop();
            int start = st.top();  st.pop();
            regex_pairs.push_back({start, accept});

            // 解析编码基：如 _ID101 / _num100 / _specail200S
            string nameLeft = buffer.substr(0, equalPos);
            // 提取末尾的数字（忽略末尾字母如S）
            int codeBase = 0;
            for (int i = (int)nameLeft.size()-1; i >= 0; --i) {
                if (nameLeft[i] >= '0' && nameLeft[i] <= '9') {
                    int j = i;
                    while (j >= 0 && nameLeft[j] >= '0' && nameLeft[j] <= '9') --j;
                    codeBase = stoi(nameLeft.substr(j+1, i - j));
                    break;
                }
            }
            acceptCodeMap[accept] = codeBase;
        }
        else
        {
            // 处理命名：加入到全局字母表，支持最长匹配
            string name = buffer.substr(0, equalPos);
            names.push_back(name);
        }
    }

    // 合并所有NFA子图为统一NFA（Thompson构造）：新增统一起点S与统一终点E
    unify_all_regex(regex_pairs);

    // 基础验证：检查统一起点的ε出边数量与统一终点的入边建立情况
    {
        int E = st.top();
        int S = 0; // 次顶为起点
        {
            int tmp = st.top();
            st.pop();
            S = st.top();
            st.push(tmp);
        }
        bool okE = true;
        for (auto &pr : regex_pairs) {
            int acc = pr.second;
            bool found = false;
            for (int v : nfa[acc].a["eps"]) if (v == E) { found = true; break; }
            if (!found) { okE = false; break; }
        }
        QString check = QString("[验证] S的ε出边=%1, 期望=%2, E入边完整=%3\n")
                            .arg((int)nfa[S].a["eps"].size())
                            .arg((int)regex_pairs.size())
                            .arg(okE ? "true" : "false");
        result1 += check + "\n";
    }

    // 构建合并后的DFA
    d.nfa_to_dfa(nfa);
    QString dfaOut = d.print_dfa();
    qDebug() << "nfa -> dfa: ";
    qDebug() << dfaOut;
    dialog->setCurrentDFA(d);

    // 1) 构建并填充统一NFA的状态转换表
    {
        QTableWidget *table = dialog->ui->tableWidget_nfa;
        // 构建表头：State + ε + Σ
        QStringList headers;
        headers << "State" << "ε";
        for (auto &sym : names) headers << QString::fromUtf8(sym);
        table->clear();
        table->setColumnCount(headers.size());
        table->setRowCount((int)nfa.size());
        table->setHorizontalHeaderLabels(headers);

        // 起点检测（无入度）
        set<int> in;
        for (int i = 0; i < (int)nfa.size(); ++i) {
            // ε入度
            for (int v : nfa[i].a["eps"]) in.insert(v);
            // Σ入度
            for (auto &sym : names) {
                for (int v : nfa[i].a[sym]) in.insert(v);
            }
        }
        int startNfa = 0;
        for (int i = 0; i < (int)nfa.size(); ++i) if (!in.count(i)) { startNfa = i; break; }

        // 填充行
        for (int i = 0; i < (int)nfa.size(); ++i) {
            QString stateName = QString::number(i);
            if (i == startNfa) stateName += " (I)";
            if (accepted.count(i)) stateName += " (Z)";
            table->setItem(i, 0, new QTableWidgetItem(stateName));

            // ε列
            QString epsTargets;
            for (int k = 0; k < (int)nfa[i].a["eps"].size(); ++k) {
                if (k) epsTargets += ", ";
                epsTargets += QString::number(nfa[i].a["eps"][k]);
            }
            table->setItem(i, 1, new QTableWidgetItem(epsTargets));

            // Σ列
            int baseCol = 2;
            for (int j = 0; j < (int)names.size(); ++j) {
                QString cell;
                const auto &vec = nfa[i].a[names[j]];
                for (int k = 0; k < (int)vec.size(); ++k) {
                    if (k) cell += ", ";
                    cell += QString::number(vec[k]);
                }
                table->setItem(i, baseCol + j, new QTableWidgetItem(cell));
            }
            table->setRowHeight(i, 22);
        }
        table->resizeColumnsToContents();
    }

    // 2) 构建并填充合并后DFA的状态转换表
    {
        QTableWidget *table = dialog->ui->tableWidget_dfa;
        QStringList headers;
        headers << "State";
        for (auto &sym : names) headers << QString::fromUtf8(sym);
        table->clear();
        table->setColumnCount(headers.size());
        table->setRowCount(d.dfa_size);
        table->setHorizontalHeaderLabels(headers);

        for (int i = 0; i < d.dfa_size; ++i) {
            QString stateName = QString::number(i);
            if (i == d.start_index) stateName += " (I)";
            if (d.dfa[i].end) stateName += " (Z)";
            table->setItem(i, 0, new QTableWidgetItem(stateName));

            for (int j = 0; j < (int)names.size(); ++j) {
                QString cell;
                const auto &vec = d.dfa[i].a[names[j]];
                for (int k = 0; k < (int)vec.size(); ++k) {
                    if (k) cell += ", ";
                    cell += QString::number(vec[k]);
                }
                table->setItem(i, 1 + j, new QTableWidgetItem(cell));
            }
            table->setRowHeight(i, 22);
        }
        table->resizeColumnsToContents();
    }

    //dfa最小化
    QString minOut = d.minDFA();
    qDebug() << "minDFA: ";
    qDebug() << minOut;
    // 3) 构建并填充最小化DFA的状态转换表
    {
        QTableWidget *table = dialog->ui->tableWidget_minDfa;
        // 此时 d 内已被 minDFA 更新为最小化结果
        QStringList headers;
        headers << "State";
        for (auto &sym : names) headers << QString::fromUtf8(sym);
        table->clear();
        table->setColumnCount(headers.size());
        table->setRowCount(d.dfa_size);
        table->setHorizontalHeaderLabels(headers);

        // 起点分区：沿用 stateBlock[0] 的打印逻辑已被替换，直接用 start_index=0
        for (int i = 0; i < d.dfa_size; ++i) {
            QString stateName = QString::number(i);
            if (i == 0) stateName += " (I)";
            if (d.dfa[i].end) stateName += " (Z)";
            table->setItem(i, 0, new QTableWidgetItem(stateName));

            for (int j = 0; j < (int)names.size(); ++j) {
                QString cell;
                const auto &vec = d.dfa[i].a[names[j]];
                for (int k = 0; k < (int)vec.size(); ++k) {
                    if (k) cell += ", ";
                    cell += QString::number(vec[k]);
                }
                table->setItem(i, 1 + j, new QTableWidgetItem(cell));
            }
            table->setRowHeight(i, 22);
        }
        table->resizeColumnsToContents();
    }
}
