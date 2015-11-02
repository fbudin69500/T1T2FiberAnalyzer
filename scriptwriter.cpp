#include "scriptwriter.h"

ScriptWriter* ScriptWriter::writer = NULL;

ScriptWriter* ScriptWriter::getInstance(QString tool, QString pipeline){
    if(!writer)
        writer = new ScriptWriter(tool, pipeline);
    return writer;
}

// to-do: enforce pathname to end with .py
ScriptWriter::ScriptWriter(QString tool,QString pipeline): toollock(NULL)
{
    tool_script = tool;
    pipeline_script = pipeline;
    hasTool = false;
}


void ScriptWriter::writePreliminary(){
    if(toollock && toollock->isLocked()){
        // to-do: throw exception
        return;
    }

    QFileInfo info(tool_script);
    if(info.exists()){
        // to-do: throw exception
    }else{
        toollock = new QLockFile(tool_script);
        toollock->setStaleLockTime(3000);

        if(!toollock->tryLock()){
            // to-do: throw exception
            return;
        }

        QFile file(tool_script);
        if(file.open(QIODevice::WriteOnly)){
            // obtaining lock of file
            file.write("xxx");
            file.write("yyy");
            file.close();


        }else{
            // to-do: open failed. Should throw an error

        }

    }
}


void ScriptWriter::writeData(){

}

void ScriptWriter::close(){
    if(toollock && toollock->isLocked()){
        toollock->unlock();
    }
    QFile file(tool_script);
    if(file.exists())
        file.remove();
}
