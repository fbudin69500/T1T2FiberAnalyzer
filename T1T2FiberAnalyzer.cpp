#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QDialog>
#include <QtDebug>
#include <QTableView>
#include <QTabWidget>
#include <iostream>
#include <utility>
#include <functional>
#include "include/T1T2FiberAnalyzer.h"

T1T2FiberAnalyzer::T1T2FiberAnalyzer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitializeState();
}

T1T2FiberAnalyzer::~T1T2FiberAnalyzer()
{
    SaveGuiValue();
    delete m_gui, s_gui, l_gui;
    delete DEFAULT_PATH;
    delete ui;
}


void T1T2FiberAnalyzer::InitializeState(){
    ui->tabWidget->setCurrentIndex(0);
    ui->T12MapInputText->setReadOnly(true);
    ui->DTIdefInputText->setReadOnly(true);

    m_gui = new Model_T1T2FiberAnalyzer();
    s_gui = new Save_T1T2FiberAnalyzer();
    l_gui = new Load_T1T2FiberAnalyzer();

    QComboBox* cb1 = ui->DTIComboPath;
    QComboBox* cb2 = ui->DTIComboSID;
    QComboBox* cb3 = ui->T12ComboPath;
    QComboBox* cb4 = ui->T12ComboSID;

    // set/check header selection
    QObject::connect(ui->T12MapInputText,SIGNAL(textChanged(QString)),this,SLOT(T12extractHeaders()));
    QObject::connect(ui->DTIdefInputText,SIGNAL(textChanged(QString)),this,SLOT(DTIextractHeaders()));
    QObject::connect(cb1,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb2,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb3,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb4,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));


    // default path of file dialog
    if(tool::checkDirExist(DEFAULT_DIR))
        DEFAULT_PATH = new QString(DEFAULT_DIR);
    else
        DEFAULT_PATH = new QString("./");

    // load value from QtGUI xml if data was previously stored
    QFileInfo checkXML(QTGUI_XML_NAME);
    if(checkXML.exists() && checkXML.isFile()){
        l_gui->load(*m_gui,QTGUI_XML_NAME);
        SyncToUI();
    }

}

bool T1T2FiberAnalyzer::checkPyVersion(std::string path){
    std::string cmd = path+" "+PYVERSION_SCRIPT_PATH;
    std::string result = tool::syscall(cmd.c_str());
    // removing newline character
    tool::checkNewLine(result);

    if (result == "true"){
        return true;
    }else
        return false;
}

void T1T2FiberAnalyzer::SyncToModel(){
    // concerns about racing condition?
    m_gui->setT12MapInputText(ui->T12MapInputText->text());
    m_gui->setT12ComboPath(ui->T12ComboPath->currentText());
    m_gui->setT12ComboSID(ui->T12ComboSID->currentText());
    m_gui->setDTIdefInputText(ui->DTIdefInputText->text());
    m_gui->setDTIComboPath(ui->DTIComboPath->currentText());
    m_gui->setDTIComboSID(ui->DTIComboSID->currentText());
    m_gui->setDTIFiber_Path(ui->DTIFiber_Path->text());
}

void T1T2FiberAnalyzer::SyncToUI(){
    ui->T12MapInputText->setText(m_gui->getT12MapInputText());
    ui->DTIdefInputText->setText(m_gui->getDTIdefInputText());

    int tid = ui->T12ComboPath->findText(m_gui->getT12ComboPath());
    if(tid >= 0){
        ui->T12ComboPath->setCurrentIndex(tid);
    }
    int tsid = ui->T12ComboSID->findText(m_gui->getT12ComboSID());
    if(tsid >= 0){
        ui->T12ComboSID->setCurrentIndex(tsid);
    }
    int did = ui->DTIComboPath->findText(m_gui->getDTIComboPath());
    if(did >= 0){
        ui->DTIComboPath->setCurrentIndex(did);
    }
    int dsid = ui->DTIComboSID->findText(m_gui->getDTIComboSID());
    if(dsid >= 0){
        ui->DTIComboSID->setCurrentIndex(dsid);
    }
}

void T1T2FiberAnalyzer::checkHeaderSelection(){
    QString str1 = ui->T12ComboPath->currentText();
    QString str2 = ui->T12ComboSID->currentText();
    QString str3 = ui->DTIComboPath->currentText();
    QString str4 = ui->DTIComboSID->currentText();
    if((!str1.isEmpty()) && (!str2.isEmpty()) && (!str3.isEmpty()) && (!str4.isEmpty())){
        ui->MatchResultBtn->setEnabled(true);
        ui->MatchTableSelectAll->setEnabled(true);
        ui->MatchTableDeselectAll->setEnabled(true);
    }
}

void T1T2FiberAnalyzer::T12extractHeaders(){
    QString fileName = ui->T12MapInputText->text();
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

void T1T2FiberAnalyzer::DTIextractHeaders(){
    QString fileName = ui->DTIdefInputText->text();
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

void T1T2FiberAnalyzer::SaveGuiValue(){
    SyncToModel();
    //QString m_DialogDir;
    /*QString filename = QFileDialog::getSaveFileName( this , "Save Changed Value to Configuration File?" , m_DialogDir , "XML files (*.xml)" );
    if( filename != "" )
    {
        QFileInfo fi( filename ) ;
        m_DialogDir = fi.dir().absolutePath() ;
        s_gui->save(*m_gui,filename.toStdString());
    }*/

    QMessageBox::StandardButton rtn = QMessageBox::question(this, "","Save Changed Value to Configuration File?");
    if(rtn == QMessageBox::Yes || rtn == QMessageBox::NoButton){
        s_gui->save(*m_gui,QTGUI_XML_NAME);
    }
}

void T1T2FiberAnalyzer::initializePyPath(){
    char* pypath = std::getenv("TFA_PYTHON");

    if(!pypath){
        const char* pypath = tool::syscall("which python").c_str();
        ui->pyPath->setText(QString(pypath));
    }
    else if(tool::checkExecutable(pypath))
        ui->pyPath->setText(QString(pypath));
    else
        ErrorReporter::fire("Provided python path is not executable!");

    std::string vc = ui->pyPath->text().toStdString();
    tool::checkNewLine(vc);
    if(!checkPyVersion(vc)){
        ErrorReporter::fire("Given executable is unsupported, or python version is below minimum requirement (2.5.0)!");
        ui->pyPath->clear();
    }
}



void T1T2FiberAnalyzer::on_T12MapInputBtn_clicked()
{
    /* setting path in edit line*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select File"),*DEFAULT_PATH);
    if(fileName == NULL) return;
    ui->T12MapInputText->setText(fileName);
}

void T1T2FiberAnalyzer::on_DTIdefInputBtn_clicked()
{
    /* setting path in edit line*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select File"),*DEFAULT_PATH);
    if(fileName == NULL) return;
    ui->DTIdefInputText->setText(fileName);
}

void T1T2FiberAnalyzer::on_DTIAtlasPathBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("Open Directory"),*DEFAULT_PATH,
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

    ui->FiberTableSelectAll->setEnabled(true);
    ui->FiberTableDeselectAll->setEnabled(true);

}

void T1T2FiberAnalyzer::on_T12BrowseBtn_clicked()
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

void T1T2FiberAnalyzer::on_DTIBrowseBtn_clicked()
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

void T1T2FiberAnalyzer::on_MatchResultBtn_clicked()
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

    /*
    if(str1 == str2 || str3 == str4){
        ErrorReporter::fire("Header names should not be the same!");
        return;
    }
    */

    QString T12_csv = ui->T12MapInputText->text();
    QString DTI_csv = ui->DTIdefInputText->text();

    try{
        tool::parseMapContent(T12_csv.toStdString(),T12TractData,str1,str2);
        tool::parseMapContent(DTI_csv.toStdString(),DTITractData,str3,str4);
        AtlasModel *mm = new AtlasModel(0,T12TractData,DTITractData);
        QItemSelectionModel *m = ui->CSVMatchTable->selectionModel();
        ui->CSVMatchTable->setModel(mm);
        ui->CSVMatchTable->horizontalHeader()->setStretchLastSection(true);
        if(m!=NULL) delete m;
    }catch(io::error::can_not_open_file e){
        ErrorReporter::fire("Given csv paths can not be open.");
        return;
    }catch(io::error::too_few_columns e){
        ErrorReporter::fire("Invalid csv files are given.");
        return;
    }catch(io::error::too_many_columns e){
        ErrorReporter::fire("Invalid csv files are given.");
        return;
    }catch(io::error::missing_column_in_header e){
        ErrorReporter::fire("Specified headers are not found in given csv path.");
        return;
    }


}

void T1T2FiberAnalyzer::on_pyPathBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Select Python Path"), *DEFAULT_PATH);
    if(filename == NULL) return;
    std::string path = filename.toStdString();
    tool::checkNewLine(path);
    if (!tool::checkExecutable(path))
        ErrorReporter::fire("Provided python path is not executable!");
    else if(!checkPyVersion(path)){
        ErrorReporter::fire("Given executable is unsupported, or python version is below minimum requirement (2.5.0)!");
    }else
        ui->pyPath->setText(filename);

}

void T1T2FiberAnalyzer::on_MatchTableSelectAll_clicked()
{
    AtlasModel* model = (AtlasModel*) ui->CSVMatchTable->model();
    model->resetModel(Qt::Checked);
}

void T1T2FiberAnalyzer::on_MatchTableDeselectAll_clicked()
{
    AtlasModel* model = (AtlasModel*) ui->CSVMatchTable->model();
    model->resetModel(Qt::Unchecked);
}

void T1T2FiberAnalyzer::on_FiberTableSelectAll_clicked()
{
    FiberTractModel* model = (FiberTractModel*) ui->Fiber_Tracts_Table->model();
    model->resetModel(Qt::Checked);
}

void T1T2FiberAnalyzer::on_FiberTableDeselectAll_clicked()
{
    FiberTractModel* model = (FiberTractModel*) ui->Fiber_Tracts_Table->model();
    model->resetModel(Qt::Unchecked);
}
