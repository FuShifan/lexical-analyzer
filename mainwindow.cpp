#include "mainwindow.h"
#include "ui_dialog.h"
#include "ui_mainwindow.h"
#include "NFA.h"
#include "DFA.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDebug>

extern QString fileName;
extern vector<string> names;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "点击打开文件";

    fileName = QFileDialog::getOpenFileName(this,tr("打开文件"),"./Data/",tr("Text File(*.txt)"));
    if(!fileName.isNull())
    {
        dialog = new Dialog(this); //新建一个窗口对象，this指定了新窗口的父对象是mainwindow，在销毁mainwindow时也会销毁该窗口dialog->setModal(false);
        dialog->setModal(false);//modal属性决定了将弹出的dialog窗口为非模态

        QFile file(fileName);
        if(!file.open(QFile::ReadOnly|QFile::Text))
        {
            QMessageBox::warning(this,tr("Error"),tr("read file error:&1").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        dialog->ui->textEdit->setPlainText(in.readAll());
        QApplication::restoreOverrideCursor();
        file.close();

        //fileName是选择的文件名
        names.clear();
        NFA n;
        n.processed(dialog);

        dialog->show();
    }
    else{
        qDebug()<<"取消"; //点的是取消
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    qDebug() << "点击开始转换";
    QString strTextEdit = "" + ui->textEdit->toPlainText();

    QFile file("../测试数据.txt");
    //打开文件，不存在则创建
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    //写入文件需要字符串为QByteArray格式
    file.write(strTextEdit.toUtf8());
    file.close();

    fileName = "../测试数据.txt";
    if(!fileName.isNull())
    {
        dialog = new Dialog(this); //新建一个窗口对象，this指定了新窗口的父对象是mainwindow，在销毁mainwindow时也会销毁该窗口dialog->setModal(false);
        dialog->setModal(false);//modal属性决定了将弹出的dialog窗口为非模态

        QFile file(fileName);
        if(!file.open(QFile::ReadOnly|QFile::Text))
        {
            QMessageBox::warning(this,tr("Error"),tr("read file error:&1").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        dialog->ui->textEdit->setPlainText(in.readAll());
        QApplication::restoreOverrideCursor();
        file.close();

        //fileName是选择的文件名
        names.clear();
        NFA n;
        n.processed(dialog);

        dialog->show();
    }

}

void MainWindow::on_pushButton_3_clicked()
{
    qDebug()<<"退出"; //点的是退出
    this->close();
}

