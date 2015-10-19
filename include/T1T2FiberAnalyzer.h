#ifndef T1T2FIBERANALYZER_H
#define T1T2FIBERANALYZER_H
#define DEFAULT_DIR "/home/haiweich/Dev/repo/T1T2FiberAnalyzer/test_data/"
#define QTGUI_XML_NAME "T1T2GUI.xml"
#define PYVERSION_SCRIPT_PATH "./version.py"

#include <QMainWindow>

#include "include/csvbrowser.h"
#include "include/errorreporter.h"
#include "include/Load_T1T2FiberAnalyzer.h"
#include "include/Save_T1T2FiberAnalyzer.h"
#include "include/Model_T1T2FiberAnalyzer.h"
#include "fibertractmodel.h"
#include "atlasmodel.h"
#include "Resources/csv.h"
#include "ui_mainwindow.h"

namespace Ui {
class T1T2FiberAnalyzer;
}

class T1T2FiberAnalyzer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit T1T2FiberAnalyzer(QWidget *parent = 0);
    ~T1T2FiberAnalyzer();

    void initializePyPath();
    bool checkPyVersion(std::string path);
    
private slots:
    void checkHeaderSelection();

    void T12extractHeaders();

    void DTIextractHeaders();

    void on_T12MapInputBtn_clicked();

    void on_DTIdefInputBtn_clicked();

    void on_DTIAtlasPathBtn_clicked();

    void on_T12BrowseBtn_clicked();

    void on_DTIBrowseBtn_clicked();

    void on_MatchResultBtn_clicked();

    void on_pyPathBtn_clicked();

    void on_MatchTableSelectAll_clicked();

    void on_MatchTableDeselectAll_clicked();

    void on_FiberTableSelectAll_clicked();

    void on_FiberTableDeselectAll_clicked();

signals:
    void headerSelected();

private:
    Ui::MainWindow *ui;
    Model_T1T2FiberAnalyzer* m_gui;
    Save_T1T2FiberAnalyzer* s_gui;
    Load_T1T2FiberAnalyzer* l_gui;
    std::map<std::string,tool::TractData> T12TractData;
    std::map<std::string,tool::TractData> DTITractData;
    std::vector<std::string> T12headers;
    std::vector<std::string> DTIheaders;
    const QString* DEFAULT_PATH;

    void InitializeState();
    void SyncToModel();
    void SyncToUI();
    void SaveGuiValue();



};

#endif // MAINWINDOW_H
