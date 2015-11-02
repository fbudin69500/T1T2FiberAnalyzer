#ifndef SCRIPTWRITER_H
#define SCRIPTWRITER_H

#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QString>

// class used for writing python script
// used singleton pattern
class ScriptWriter
{
public:
    static ScriptWriter *getInstance(QString tool, QString pipeline);
    void writePreliminary();
    void writeData();
    void close();


private:
    ScriptWriter(QString tool, QString pipeline);
    QString tool_script;
    QString pipeline_script;
    QLockFile* toollock;

    bool hasTool;
    static ScriptWriter *writer;
};

#endif // SCRIPTWRITER_H
