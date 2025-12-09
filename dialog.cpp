#include "dialog.h"
#include "NFA.h"
#include "DFA.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QGuiApplication>
#include <QVBoxLayout>
extern QString fileName;
extern vector<string> names;
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

//退出按钮
void Dialog::on_pushButton_2_clicked()
{
    qDebug()<<"退出"; //点的是退出
    this->close();
}

//点击重新转换
void Dialog::on_pushButton_clicked()
{
    QString strTextEdit = ui->textEdit->toPlainText();

    QFile file("../测试数据.txt");
    //打开文件，不存在则创建
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    //写入文件需要字符串为QByteArray格式
    file.write(strTextEdit.toUtf8());
    file.close();

    fileName = "../测试数据.txt";
    if(!fileName.isNull())
    {

        QFile file(fileName);
        if(!file.open(QFile::ReadOnly|QFile::Text))
        {
            QMessageBox::warning(this,tr("Error"),tr("read file error:&1").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->textEdit->setPlainText(in.readAll());
        QApplication::restoreOverrideCursor();
        file.close();

        //fileName是选择的文件名
        names.clear();
        NFA n;
        n.processed(this);

        show();
    }

}

void Dialog::setCurrentDFA(const DFA& d)
{
    currentDFA = std::make_unique<DFA>(d);
}


static QString exportTable(QTableWidget* table)
{
    QString out;
    if (!table) return out;
    // 头
    QStringList headers;
    for (int c = 0; c < table->columnCount(); ++c) {
        QTableWidgetItem *hi = table->horizontalHeaderItem(c);
        headers << (hi ? hi->text() : QString("Col%1").arg(c));
    }
    out += headers.join(", ") + "\n";
    // 行
    for (int r = 0; r < table->rowCount(); ++r) {
        QStringList cells;
        for (int c = 0; c < table->columnCount(); ++c) {
            QTableWidgetItem *it = table->item(r, c);
            cells << (it ? it->text() : "");
        }
        out += cells.join(", ") + "\n";
    }
    return out;
}

//另存按钮
void Dialog::on_pushButton_3_clicked()
{
    // 导出三张表
    QString strOut;
    strOut += "[NFA]\n" + exportTable(ui->tableWidget_nfa) + "\n";
    strOut += "[DFA]\n" + exportTable(ui->tableWidget_dfa) + "\n";
    strOut += "[MinDFA]\n" + exportTable(ui->tableWidget_minDfa) + "\n";

    QString address = QFileDialog::getSaveFileName(this, tr("保存到"), "./Data/", tr("Text Files (*.txt)"));
    if (!address.isEmpty()) {
        QFile file(address);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(strOut.toUtf8());
            file.close();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件进行保存！"));
        }
    }
}

//直接导出按钮
void Dialog::on_pushButton_4_clicked()
{
    QString strOut;
    strOut += "[NFA]\n" + exportTable(ui->tableWidget_nfa) + "\n";
    strOut += "[DFA]\n" + exportTable(ui->tableWidget_dfa) + "\n";
    strOut += "[MinDFA]\n" + exportTable(ui->tableWidget_minDfa) + "\n";

    const QString address = "../转换结果.txt";
    if (!address.isEmpty()) {
        QFile file(address);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(strOut.toUtf8());
            file.close();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件进行保存！"));
        }
        const QString str = "文件已保存到\n" + address;
        QMessageBox::about(this, tr("保存成功"), str);
    }
}

//生成代码
void Dialog::on_pushButton_genCode_clicked()
{
    if (!currentDFA || currentDFA->dfa_size == 0) {
        QMessageBox::critical(this, tr("错误"), tr("请先进行转换以生成 DFA！"));
        return;
    }
    QString src = currentDFA->generate_switch_case_source();
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("生成的匹配代码"));
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QPlainTextEdit *pe = new QPlainTextEdit(dlg);
    pe->setPlainText(src);
    pe->setReadOnly(true);
    layout->addWidget(pe);
    QPushButton *copyBtn = new QPushButton(tr("复制到剪贴板"), dlg);
    QObject::connect(copyBtn, &QPushButton::clicked, [pe]() {
        QGuiApplication::clipboard()->setText(pe->toPlainText());
    });
    layout->addWidget(copyBtn);
    dlg->resize(800, 600);
    dlg->show();
}

