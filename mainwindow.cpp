#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "csvbrowser.h"
#include "errorreporter.h"
#include <QApplication>
#include <QFileDialog>
#include <QDialog>
#include <QtDebug>
#include <QTableView>
#include <iostream>
#include <utility>
#include <functional>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QComboBox* cb1 = ui->DTIComboPath;
    QComboBox* cb2 = ui->DTIComboSID;
    QComboBox* cb3 = ui->T12ComboPath;
    QComboBox* cb4 = ui->T12ComboSID;

    QObject::connect(cb1,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb2,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb3,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb4,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));

    std::cout << tool::syscall("pwd");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_T12MapInputBtn_clicked()
{
    /* setting path in edit line*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select File"),DEFAULT_DIR);
    if(fileName == NULL) return;
    ui->T12MapInputText->setText(fileName);

    // parse headers of csv file and populate into combo boxes
    T12headers.clear();
    try{
        tool::parseCSV(fileName.toStdString(),T12headers);
        QComboBox* cb1 = ui->T12ComboPath;
        QComboBox* cb2 = ui->T12ComboSID;
        cb1->clear();
        cb2->clear();
        for(std::string i : T12headers){
            QString si = QString::fromStdString(i);
            cb1->addItem(si);
            cb2->addItem(si);
        }

    }
    catch(io::error::can_not_open_file e){
        ErrorReporter::fire("CSV file at the give npath is not found!");
        ui->T12MapInputText->clear();
        return;
    }

}

void MainWindow::on_DTIdefInputBtn_clicked()
{
    /* setting path in edit line*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select File"),DEFAULT_DIR);
    if(fileName == NULL) return;
    ui->DTIdefInputText->setText(fileName);
    // parse headers of csv file and populate into combo boxes
    DTIheaders.clear();
    try{
        tool::parseCSV(fileName.toStdString(),DTIheaders);
        QComboBox* cb1 = ui->DTIComboPath;
        QComboBox* cb2 = ui->DTIComboSID;
        cb1->clear();
        cb2->clear();
        for(std::string i : DTIheaders){
            QString si = QString::fromStdString(i);
            cb1->addItem(si);
            cb2->addItem(si);
        }

    }
    catch(io::error::can_not_open_file e){
        ErrorReporter::fire("CSV file at the give npath is not found!");
        ui->DTIdefInputText->clear();
        return;
    }

}

void MainWindow::on_DTIAtlasPathBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("Open Directory"),DEFAULT_DIR,
                                                    QFileDialog::ShowDirsOnly);
    if(dir == NULL) return;
    ui->DTIFiber_Path->setText(dir);
    std::vector<tool::TractData> filelist;
    tool::getnrrdfiles(dir.toStdString(),filelist);

    FiberTractModel *mm = new FiberTractModel(0,filelist);
    QItemSelectionModel *m =ui->CSVMatchTable->selectionModel();
    ui->Fiber_Tracts_Table->setModel(mm);
    ui->Fiber_Tracts_Table->horizontalHeader()->setStretchLastSection(true);
    if(m) delete m;
}

void MainWindow::on_T12BrowseBtn_clicked()
{
    QString dir = ui->T12MapInputText->text();
    if(dir == NULL) return;
    std::vector<std::vector<std::string>> csv_results;
    std::vector<std::string> headers;
    if(dir != NULL){
        try{
        csv_results = tool::parseCSV(dir.toStdString(),headers);
        }
        catch(io::error::can_not_open_file e){
            ErrorReporter::fire("CSV file is not found!");
            return;
        }
    }
    CSVBrowser *bd = new CSVBrowser();
    bd->show();
    bd->loadTable(csv_results,headers);

}

void MainWindow::on_DTIBrowseBtn_clicked()
{
    QString dir = ui->DTIdefInputText->text();
    std::vector<std::vector<std::string>> csv_results;
    std::vector<std::string> headers;
    if(dir != NULL){
        try{
        csv_results = tool::parseCSV(dir.toStdString(),headers);
        }
        catch(io::error::can_not_open_file e){
            ErrorReporter::fire("CSV file is not found!");
            return;
        }
    }
    CSVBrowser *bd = new CSVBrowser();
    bd->show();
    bd->loadTable(csv_results,headers);
}

void MainWindow::checkHeaderSelection(){
    QString str1 = ui->T12ComboPath->currentText();
    QString str2 = ui->T12ComboSID->currentText();
    QString str3 = ui->DTIComboPath->currentText();
    QString str4 = ui->DTIComboSID->currentText();
    if((!str1.isEmpty()) && (!str2.isEmpty()) && (!str3.isEmpty()) && (!str4.isEmpty()))
        ui->MatchResultBtn->setEnabled(true);
}

void MainWindow::on_MatchResultBtn_clicked()
{
    T12TractData.clear();
    DTITractData.clear();
    std::string str1 = ui->T12ComboPath->currentText().toStdString();
    str1 = tool::trim(str1);
    std::string str2 = ui->T12ComboSID->currentText().toStdString();
    str2 = tool::trim(str2);
    std::string str3 = ui->DTIComboPath->currentText().toStdString();
    str3 = tool::trim(str3);
    std::string str4 = ui->DTIComboSID->currentText().toStdString();
    str4 = tool::trim(str4);

    if(str1 == str2 || str3 == str4){
        ErrorReporter::fire("Header names should not be the same!");
        return;
    }

    QString T12_csv = ui->T12MapInputText->text();
    QString DTI_csv = ui->DTIdefInputText->text();

    tool::parseMapContent(T12_csv.toStdString(),T12TractData,str1,str2);
    tool::parseMapContent(DTI_csv.toStdString(),DTITractData,str3,str4);

    AtlasModel *mm = new AtlasModel(0,T12TractData,DTITractData);
    QItemSelectionModel *m = ui->CSVMatchTable->selectionModel();
    ui->CSVMatchTable->setModel(mm);
    ui->CSVMatchTable->horizontalHeader()->setStretchLastSection(true);
    if(m!=NULL) delete m;


}
