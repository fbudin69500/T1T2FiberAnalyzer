#include <iostream>
#include <utility>
#include <functional>
#include <typeinfo>
#include "include/T1T2FiberAnalyzer.h"

T1T2FiberAnalyzer::T1T2FiberAnalyzer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    para_File(QTGUI_XML_NAME)
{
    ui->setupUi(this);
    isSync = false;
    InitializeState();
}


T1T2FiberAnalyzer::~T1T2FiberAnalyzer()
{
    writer->close();
    delete m_gui;
    delete s_gui;
    delete l_gui;
    delete atlas;
    delete tracts;
    delete writer;
    delete DEFAULT_PATH;
    delete ui;
    // deallocate memory used by TableView model?

}

void T1T2FiberAnalyzer::initializePyPath(){
    char* pypath = std::getenv("TFA_PYTHON");

    // write to version.py
    // tool::writePyVersion()
    writer = ScriptWriter::getInstance("version2.py","version3.py");
    writer->writePreliminary();

    if(!pypath){
        char* path = std::getenv("PATH");
        std::vector<std::string> checkpath;
        tool::tokenize(path,":",checkpath);
        if(checkpath.empty())
            ErrorReporter::fire("Failed to locate a python compiler in $PATH! Please configure it manually.");
        else{
            for(std::vector<std::string>::iterator p = checkpath.begin(); p != checkpath.end();p++){
                QDirIterator dirit(QString::fromStdString(*p));
                while(dirit.hasNext()){
                    QString fn = dirit.next();         
                    if(tool::checkExecutable(fn.toStdString()) && fn.toStdString().find("python") != std::string::npos){
                        //qDebug() << fn;
                        if(checkPyVersion(fn.toStdString())){
                            ui->conf_pypath->setText(fn);
                            return;
                        }
                    }
                }
            }

            // failed to find python compiler
            ErrorReporter::fire("Failed to locate a python compiler in $PATH! Please configure it manually.");
            return;
        }

    }
    else if(tool::checkExecutable(pypath)){
        // to-do: codes need cleanup here. Python version should be checked first before populating it to gui
        ui->conf_pypath->setText(QString(pypath));
    }
    else{
        ErrorReporter::fire("Given path in $TFA_PYTHON is not executable!");
        return;
    }

    std::string vc = ui->conf_pypath->text().toStdString();
    tool::checkNewLine(vc);
    if(!checkPyVersion(vc)){
        ErrorReporter::fire("Given executable is unsupported, or python version is below minimum requirement (2.5.0)!");
        ui->conf_pypath->clear();
    }
}


// private methods

void T1T2FiberAnalyzer::InitializeState(){
    SetEventTriggers();

    ui->tabWidget->setCurrentIndex(0);
    ui->para_T12MapInputText->setReadOnly(true);
    ui->para_DTIdefInputText->setReadOnly(true);

    m_gui = new para_Model_T1T2FiberAnalyzer();
    s_gui = new para_Save_T1T2FiberAnalyzer();
    l_gui = new para_Load_T1T2FiberAnalyzer();
    atlas = NULL;
    tracts = NULL;

    // default path of file dialog
    if(tool::checkDirExist(DEFAULT_DIR))
        DEFAULT_PATH = new QString(DEFAULT_DIR);
    else
        DEFAULT_PATH = new QString("./");

    // load value from QtGUI xml if data was previously stored
    QFileInfo checkXML(QTGUI_XML_NAME);
    if(checkXML.exists() && checkXML.isFile()){
        isSync = true;
        l_gui->load(*m_gui,QTGUI_XML_NAME);
        SyncToUI();
    }


}

void T1T2FiberAnalyzer::SetEventTriggers(){
    QComboBox* cb1 = ui->para_DTIComboPath;
    QComboBox* cb2 = ui->para_DTIComboSID;
    QComboBox* cb3 = ui->para_T12ComboPath;
    QComboBox* cb4 = ui->para_T12ComboSID;

    // set/check header selection
    QObject::connect(ui->para_T12MapInputText,SIGNAL(textChanged(QString)),this,SLOT(T12extractHeaders()));
    QObject::connect(ui->para_DTIdefInputText,SIGNAL(textChanged(QString)),this,SLOT(DTIextractHeaders()));
    QObject::connect(cb1,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb2,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb3,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));
    QObject::connect(cb4,SIGNAL(currentIndexChanged(int)),this,SLOT(checkHeaderSelection()));

    // menu actions
    QObject::connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(close()));
    QObject::connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(savePara()));
    QObject::connect(ui->actionLoad,SIGNAL(triggered()),this,SLOT(loadPara()));
}

bool T1T2FiberAnalyzer::checkPyVersion(std::string path){
    std::string cmd = path+" "+PYVERSION_SCRIPT_PATH;
    std::string result = tool::syscall(cmd.c_str());
    // removing newline character
    tool::checkNewLine(result);

    if(result == "true")
        return true;
    else
        return false;
}

void T1T2FiberAnalyzer::SyncToModel(){
    // concerns about racing condition?
    m_gui->setpara_T12MapInputText(ui->para_T12MapInputText->text());
    m_gui->setpara_T12ComboPath(ui->para_T12ComboPath->currentText());
    m_gui->setpara_T12ComboSID(ui->para_T12ComboSID->currentText());
    m_gui->setpara_DTIdefInputText(ui->para_DTIdefInputText->text());
    m_gui->setpara_DTIComboPath(ui->para_DTIComboPath->currentText());
    m_gui->setpara_DTIComboSID(ui->para_DTIComboSID->currentText());
    m_gui->setpara_DTIFiber_Path(ui->para_DTIFiber_Path->text());

    if(ui->para_CSVMatchTable->model()){
        m_gui->setpara_CSVMatchTable(SyncFromAtlasTableView());
    }
}

void T1T2FiberAnalyzer::SyncToUI(){
    ui->para_T12MapInputText->setText(m_gui->getpara_T12MapInputText());
    ui->para_DTIdefInputText->setText(m_gui->getpara_DTIdefInputText());

    int tid = ui->para_T12ComboPath->findText(m_gui->getpara_T12ComboPath());
    if(tid >= 0){
        ui->para_T12ComboPath->setCurrentIndex(tid);
    }
    int tsid = ui->para_T12ComboSID->findText(m_gui->getpara_T12ComboSID());
    if(tsid >= 0){
        ui->para_T12ComboSID->setCurrentIndex(tsid);
    }
    int did = ui->para_DTIComboPath->findText(m_gui->getpara_DTIComboPath());
    if(did >= 0){
        ui->para_DTIComboPath->setCurrentIndex(did);
    }
    int dsid = ui->para_DTIComboSID->findText(m_gui->getpara_DTIComboSID());
    if(dsid >= 0){
        ui->para_DTIComboSID->setCurrentIndex(dsid);
    }

    /* synchronization of table model only occurs after match button is clicked
    */


}

void T1T2FiberAnalyzer::SyncToAtlasTableView(){
    std::vector<std::vector<QString> > m = m_gui->getpara_CSVMatchTable();
    for(std::vector<std::vector<QString> >::iterator it1 = m.begin(); it1 != m.end(); ++it1){
        // integrity check
        if (it1->size() != 2)
            continue;
        for(std::vector<QString>::iterator it2 = (*it1).begin(); it2 != (*it1).end(); ++it2){
            if(it2->at(0) == QString("false")){
                unsigned int found = atlas->findData(it2->at(1));
                if(found >= 0)
                    atlas->resetModel(Qt::Unchecked,found);
            }
        }
    }
}

// issue: match subjectID, or match everything?
std::vector<std::vector<QString> > T1T2FiberAnalyzer::SyncFromAtlasTableView(){
    std::vector<std::vector<QString> > data;
    for(unsigned int i =0; i < atlas->getDataSize(); i++){
        std::vector<QString> row;
        if(atlas->getCheckState(i))
            row.push_back("true");
        else
            row.push_back("false");
        row.push_back(atlas->getData(i).subjectID);
        data.push_back(row);
    }

    return data;
}

void T1T2FiberAnalyzer::T12extractHeaders(){
    QString fileName = ui->para_T12MapInputText->text();
    // parse headers of csv file and populate into combo boxes
    T12headers.clear();
    try{
        tool::parseCSV(fileName.toStdString(),T12headers);
        QComboBox* cb1 = ui->para_T12ComboPath;
        QComboBox* cb2 = ui->para_T12ComboSID;
        cb1->clear();
        cb2->clear();
        for(std::vector<std::string>::iterator i = T12headers.begin(); i != T12headers.end();i++){
            QString si = QString::fromStdString(*i);
            cb1->addItem(si);
            cb2->addItem(si);
        }

    }
    catch(csvparser::ReadError e){
            ErrorReporter::fire(string(e.what()));
            return;
        }
}

void T1T2FiberAnalyzer::DTIextractHeaders(){
    QString fileName = ui->para_DTIdefInputText->text();
    // parse headers of csv file and populate into combo boxes
    DTIheaders.clear();
    try{
        tool::parseCSV(fileName.toStdString(),DTIheaders);
        QComboBox* cb1 = ui->para_DTIComboPath;
        QComboBox* cb2 = ui->para_DTIComboSID;
        cb1->clear();
        cb2->clear();
        for(std::vector<std::string>::iterator i = DTIheaders.begin(); i != DTIheaders.end() ; i++ ){
            QString si = QString::fromStdString(*i);
            cb1->addItem(si);
            cb2->addItem(si);
        }

    }
    catch(csvparser::ReadError e){
            ErrorReporter::fire(string(e.what()));
            return;
        }
}

QMessageBox::StandardButton T1T2FiberAnalyzer::SaveGuiValue(QString filename){
    SyncToModel();
    QMessageBox::StandardButton rtn = QMessageBox::question(this, "","Save Changed Value to Configuration File "+ filename  +"?",
                                                            QMessageBox::Yes|QMessageBox::No| QMessageBox::Cancel);
    if(rtn == QMessageBox::Yes || rtn == QMessageBox::NoButton){
        s_gui->save(*m_gui,filename.toStdString());
    }

    return rtn;
}

// protected methods

void T1T2FiberAnalyzer::contextMenuEvent(QContextMenuEvent *event){
    ui->menuConfiguration->exec(event->globalPos());
}

void T1T2FiberAnalyzer::closeEvent(QCloseEvent *event){
    QMessageBox::StandardButton rtn = SaveGuiValue(para_File);

    if(rtn == QMessageBox::Cancel || rtn == QMessageBox::Escape){
        event->ignore();
    }else
        event->accept();
}

// slots

void T1T2FiberAnalyzer::savePara(){
    // to-do: add a cancel button

    SyncToModel();
    QString m_DialogDir = para_File;
    QString filename = QFileDialog::getSaveFileName( this , "Save Changed Value to Configuration File" , m_DialogDir , "XML files (*.xml)" );
    if( filename != "" )
    {
        QFileInfo fi( filename ) ;
        m_DialogDir = fi.dir().absolutePath() ;
        s_gui->save(*m_gui,filename.toStdString());
        para_File = fi.fileName();
    }

}

void T1T2FiberAnalyzer::loadPara(){
    // to-do: check for corner cases

    QString m_DialogDir = para_File;
    QString filename = QFileDialog::getOpenFileName(this,"Load Configuration File", m_DialogDir, "XML files (*.xml)");
    if(filename != ""){
        isSync = true;
        l_gui->load(*m_gui,filename.toStdString());
        SyncToUI();
    }
}

void T1T2FiberAnalyzer::checkHeaderSelection(){
    QString str1 = ui->para_T12ComboPath->currentText();
    QString str2 = ui->para_T12ComboSID->currentText();
    QString str3 = ui->para_DTIComboPath->currentText();
    QString str4 = ui->para_DTIComboSID->currentText();
    if((!str1.isEmpty()) && (!str2.isEmpty()) && (!str3.isEmpty()) && (!str4.isEmpty())){
        ui->MatchResultBtn->setEnabled(true);
        ui->MatchTableSelectAll->setEnabled(true);
        ui->MatchTableDeselectAll->setEnabled(true);
    }
}

void T1T2FiberAnalyzer::on_T12MapInputBtn_clicked()
{
    /* setting path in edit line*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select File"),*DEFAULT_PATH);
    if(fileName == NULL) return;
    ui->para_T12MapInputText->setText(fileName);
}

void T1T2FiberAnalyzer::on_DTIdefInputBtn_clicked()
{
    /* setting path in edit line*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select File"),*DEFAULT_PATH);
    if(fileName == NULL) return;
    ui->para_DTIdefInputText->setText(fileName);
}

void T1T2FiberAnalyzer::on_DTIAtlasPathBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("Open Directory"),*DEFAULT_PATH,
                                                    QFileDialog::ShowDirsOnly);
    if(dir == NULL) return;
    ui->para_DTIFiber_Path->setText(dir);
    std::vector<tool::TractData> filelist;
    tool::getnrrdfiles(dir.toStdString(),filelist);

    tracts = new FiberTractModel(0,filelist);
    QItemSelectionModel *m =ui->para_CSVMatchTable->selectionModel();
    ui->para_Fiber_Tracts_Table->setModel(tracts);
    ui->para_Fiber_Tracts_Table->horizontalHeader()->setStretchLastSection(true);
    if(m) delete m;


    ui->FiberTableSelectAll->setEnabled(true);
    ui->FiberTableDeselectAll->setEnabled(true);

}

void T1T2FiberAnalyzer::on_T12BrowseBtn_clicked()
{
    QString dir = ui->para_T12MapInputText->text();
    if(dir == NULL) return;
    std::vector<std::vector<std::string> > csv_results;
    std::vector<std::string> headers;
    if(dir != NULL){
        try{
            csv_results = tool::parseCSV(dir.toStdString(),headers);
        }
        catch(csvparser::ReadError e){
                ErrorReporter::fire(string(e.what()));
                return;
            }
    }
    CSVBrowser *bd = new CSVBrowser();
    bd->show();
    bd->loadTable(csv_results,headers);

}

void T1T2FiberAnalyzer::on_DTIBrowseBtn_clicked()
{
    QString dir = ui->para_DTIdefInputText->text();
    std::vector<std::vector<std::string> > csv_results;
    std::vector<std::string> headers;
    if(dir != NULL){
        try{
            csv_results = tool::parseCSV(dir.toStdString(),headers);
        }
        catch(csvparser::ReadError e){
                ErrorReporter::fire(string(e.what()));
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
    std::string str1 = ui->para_T12ComboPath->currentText().toStdString();
    str1 = tool::trim(str1);
    std::string str2 = ui->para_T12ComboSID->currentText().toStdString();
    str2 = tool::trim(str2);
    std::string str3 = ui->para_DTIComboPath->currentText().toStdString();
    str3 = tool::trim(str3);
    std::string str4 = ui->para_DTIComboSID->currentText().toStdString();
    str4 = tool::trim(str4);

    /*
    if(str1 == str2 || str3 == str4){
        ErrorReporter::fire("Header names should not be the same!");
        return;
    }
    */

    QString T12_csv = ui->para_T12MapInputText->text();
    QString DTI_csv = ui->para_DTIdefInputText->text();

    try{
        tool::parseMapContent(T12_csv,T12TractData,str1,str2);
        tool::parseMapContent(DTI_csv,DTITractData,str3,str4);
        atlas = new AtlasModel(0,T12TractData,DTITractData);
        QItemSelectionModel *m = ui->para_CSVMatchTable->selectionModel();

        // synchronize states if loader exists
        if(isSync)
            SyncToAtlasTableView();

        ui->para_CSVMatchTable->setModel(atlas);
        ui->para_CSVMatchTable->horizontalHeader()->setStretchLastSection(true);
        if(m!=NULL) delete m;

    }catch(csvparser::ReadError e){
        ErrorReporter::fire(string(e.what()));
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
        ui->conf_pypath->setText(filename);

}

void T1T2FiberAnalyzer::on_MatchTableSelectAll_clicked()
{
    atlas->resetModel(Qt::Checked);
}

void T1T2FiberAnalyzer::on_MatchTableDeselectAll_clicked()
{
    atlas->resetModel(Qt::Unchecked);
}

void T1T2FiberAnalyzer::on_FiberTableSelectAll_clicked()
{
    tracts->resetModel(Qt::Checked);
}

void T1T2FiberAnalyzer::on_FiberTableDeselectAll_clicked()
{
    tracts->resetModel(Qt::Unchecked);
}
