    #ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <memory>
class DFA;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    Ui::Dialog *ui;
    void setCurrentDFA(const DFA& d);
private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_genCode_clicked();
private:
    std::unique_ptr<DFA> currentDFA;
};

#endif // DIALOG_H
