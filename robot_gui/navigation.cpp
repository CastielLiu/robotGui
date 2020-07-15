#include "navigation.h"

Navigation::Navigation(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<std::vector<goalInfo_t>>("goalsInfoType");
}

bool Navigation::loadGoalPointsInfo(const QString &file_name)
{
    QFile goals_file(file_name);
    if(!goals_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open goals file: " << file_name << " failed!";
        return false;
    }

    QXmlStreamReader xmlReader(&goals_file);
    while(!xmlReader.atEnd())
    {
        //此处若不使用静态变量，将导致name为空，原因暂时未知
        static goalInfo_t goalInfo;
        if(xmlReader.isStartElement())
        {
            if(xmlReader.name() == "Goal")
            {
                QXmlStreamAttributes goalAttrs = xmlReader.attributes();
                if(goalAttrs.hasAttribute("id"))
                    goalInfo.id = goalAttrs.value("id").toInt();

                if(goalAttrs.hasAttribute("name"))
                    goalInfo.name = goalAttrs.value("name").toString().toStdString();
            }
            else if(xmlReader.name() == "Orientation")
            {
                QXmlStreamAttributes orienAttrs = xmlReader.attributes();
                if(orienAttrs.hasAttribute("x"))
                    goalInfo.orientation.x = orienAttrs.value("x").toDouble();
                if(orienAttrs.hasAttribute("y"))
                    goalInfo.orientation.y = orienAttrs.value("y").toDouble();
                if(orienAttrs.hasAttribute("z"))
                    goalInfo.orientation.z = orienAttrs.value("z").toDouble();
                if(orienAttrs.hasAttribute("w"))
                    goalInfo.orientation.w = orienAttrs.value("w").toDouble();
            }
            else if(xmlReader.name() == "Position")
            {
                QXmlStreamAttributes positionAttrs = xmlReader.attributes();
                if(positionAttrs.hasAttribute("x"))
                    goalInfo.position.x = positionAttrs.value("x").toDouble();
                if(positionAttrs.hasAttribute("y"))
                    goalInfo.position.y = positionAttrs.value("y").toDouble();
                if(positionAttrs.hasAttribute("z"))
                    goalInfo.position.z = positionAttrs.value("z").toDouble();
            }

        }
        else if(xmlReader.isEndElement() )
        {
            if(xmlReader.name() == "Goal")
            {
                //goalInfo.print();
                mGoalsInfo.push_back(goalInfo);
            }
        }

        xmlReader.readNext();
    }

    goals_file.close();
    if(mGoalsInfo.size())
        emit this->updateGoals(mGoalsInfo);

    return true;
}
